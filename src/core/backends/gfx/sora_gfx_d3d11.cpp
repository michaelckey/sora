// sora_gfx_d3d11.cpp

#ifndef SORA_GFX_D3D11_CPP
#define SORA_GFX_D3D11_CPP

//~ libs

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

//~ implementation

//- state functions 

function void
gfx_init() {
    
    gfx_d3d11_state.arena = arena_create(gigabytes(1));
    
    HRESULT hr = 0;
    UINT device_flags = 0;
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
    
#if BUILD_DEBUG
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif 
    
    // create device and device context
    hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, device_flags, feature_levels, array_count(feature_levels), D3D11_SDK_VERSION, &gfx_d3d11_state.device, 0, &gfx_d3d11_state.device_context);
    
    // get dxgi device, adaptor, and factory
    hr = gfx_d3d11_state.device->QueryInterface(__uuidof(IDXGIDevice1), (void**)(&gfx_d3d11_state.dxgi_device));
    hr = gfx_d3d11_state.dxgi_device->GetAdapter(&gfx_d3d11_state.dxgi_adapter);
    hr = gfx_d3d11_state.dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)(&gfx_d3d11_state.dxgi_factory));
    
}

function void
gfx_release() {
    
    // release d3d11 devices
    if (gfx_d3d11_state.dxgi_factory != nullptr) { gfx_d3d11_state.dxgi_factory->Release(); }
	if (gfx_d3d11_state.dxgi_adapter != nullptr) { gfx_d3d11_state.dxgi_adapter->Release(); }
	if (gfx_d3d11_state.dxgi_device != nullptr) { gfx_d3d11_state.dxgi_device->Release(); }
	if (gfx_d3d11_state.device_context != nullptr) { gfx_d3d11_state.device_context->Release(); }
	if (gfx_d3d11_state.device != nullptr) { gfx_d3d11_state.device->Release(); }
    
    // release arena
    arena_release(gfx_d3d11_state.arena);
    
}

//- command functions 





//- context functions 

function gfx_context_t
gfx_context_create(os_window_t window_handle, gfx_context_flags flags) {
    
    gfx_d3d11_context_t* context = gfx_d3d11_state.context_free;
    if (context != nullptr) {
        stack_pop(gfx_d3d11_state.context_free);
    } else {
        context = (gfx_d3d11_context_t*)arena_alloc(gfx_d3d11_state.arena, sizeof(gfx_d3d11_context_t));
    }
    memset(context, 0, sizeof(gfx_d3d11_context_t));
    dll_push_back(gfx_d3d11_state.context_first, gfx_d3d11_state.context_last, context);
    
    context->window = window_handle;
    context->flags = flags;
    context->size = os_window_get_size(window_handle);
    
    // create swapchain
    HRESULT hr = 0;
    DXGI_SWAP_CHAIN_DESC1 swapchain_desc = { 0 };
    swapchain_desc.Width = context->size.x;
    swapchain_desc.Height = context->size.y;
    swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchain_desc.Stereo = FALSE;
    swapchain_desc.SampleDesc.Count = 1;
    swapchain_desc.SampleDesc.Quality = 0;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.BufferCount = 2;
    swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    //swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
    swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchain_desc.Flags = 0;
    
    // get win32 window
    os_w32_window_t* w32_window = os_w32_window_from_window_handle(window_handle);
    
    hr = gfx_d3d11_state.dxgi_factory->CreateSwapChainForHwnd(gfx_d3d11_state.device, w32_window->handle, &swapchain_desc, 0, 0, &context->swapchain);
    hr = context->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&context->framebuffer));
    hr = gfx_d3d11_state.device->CreateRenderTargetView(context->framebuffer, 0, &context->framebuffer_rtv);
    
    // create depthbuffer if needed
    if (flags & gfx_context_flag_depthbuffer) {
        
        D3D11_TEXTURE2D_DESC depthbuffer_desc = { 0 };
        depthbuffer_desc.Width = context->size.x;
        depthbuffer_desc.Height = context->size.y;
        depthbuffer_desc.MipLevels = 1;
        depthbuffer_desc.ArraySize = 1;
        depthbuffer_desc.Format = DXGI_FORMAT_D32_FLOAT;
        depthbuffer_desc.SampleDesc.Count = 1;
        depthbuffer_desc.SampleDesc.Quality = 0;
        depthbuffer_desc.Usage = D3D11_USAGE_DEFAULT;
        depthbuffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthbuffer_desc.CPUAccessFlags = 0;
        depthbuffer_desc.MiscFlags = 0;
        
        hr = gfx_d3d11_state.device->CreateTexture2D(&depthbuffer_desc, nullptr, &context->depthbuffer);
        
        D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = { 0 };
        dsv_desc.Format = depthbuffer_desc.Format;
        dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsv_desc.Texture2D.MipSlice = 0;
        
        hr = gfx_d3d11_state.device->CreateDepthStencilView(context->depthbuffer, &dsv_desc, &context->depthbuffer_dsv);
        
    }
    
    gfx_context_t context_handle = { (u64)context };
    return context_handle;
}

function void
gfx_context_release(gfx_context_t context_handle) {
    
    gfx_d3d11_context_t* context = (gfx_d3d11_context_t*)(context_handle.id);
    
    // release resources
    if (context->framebuffer_rtv != nullptr) { context->framebuffer_rtv->Release(); }
    if (context->framebuffer != nullptr) { context->framebuffer->Release(); }
    if (context->swapchain != nullptr) { context->swapchain->Release(); }
    
    if (context->depthbuffer_dsv != nullptr) { context->depthbuffer_dsv->Release(); }
    if (context->depthbuffer != nullptr) { context->depthbuffer->Release(); }
    
    
    // remove from list
    dll_remove(gfx_d3d11_state.context_first, gfx_d3d11_state.context_last, context);
    stack_push(gfx_d3d11_state.context_free, context);
    
}

function void
gfx_context_set(gfx_context_t context_handle) {
    gfx_d3d11_context_t* context = (gfx_d3d11_context_t*)(context_handle.id);
    if (context != nullptr) {
        gfx_d3d11_state.device_context->OMSetRenderTargets(1, &context->framebuffer_rtv, nullptr);
    }
}

function void
gfx_context_clear(gfx_context_t context_handle, color_t clear_color, f32 depth_clear) {
    gfx_d3d11_context_t* context = (gfx_d3d11_context_t*)(context_handle.id);
    if (context != nullptr) {
        FLOAT clear_color_array[] = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
        gfx_d3d11_state.device_context->ClearRenderTargetView(context->framebuffer_rtv, clear_color_array);
    }
}

function void
gfx_context_present(gfx_context_t context_handle, b8 vsync) {
    gfx_d3d11_context_t* context = (gfx_d3d11_context_t*)(context_handle.id);
    if (context != nullptr) {
        if (!os_window_is_minimized(context->window)) {
            context->swapchain->Present(vsync, 0);
        } else {
            os_sleep(16); // TODO: maybe do something else.
        }
    }
}

function uvec2_t
gfx_context_get_size(gfx_context_t context_handle) {
    gfx_d3d11_context_t* context = (gfx_d3d11_context_t*)(context_handle.id);
    uvec2_t result = uvec2(0, 0);
    if (context != nullptr) {
        result = context->size;
    }
    return result;
}

function void
gfx_context_resize(gfx_context_t context_handle, uvec2_t size) {
    gfx_d3d11_context_t* context = (gfx_d3d11_context_t*)(context_handle.id);
    if (context != nullptr && 
        !uvec2_equals(context->size, size) && 
        size.x != 0 && size.y != 0) {
        
        // flush correct state
        gfx_d3d11_state.device_context->Flush();
        gfx_d3d11_state.device_context->OMSetRenderTargets(0, 0, 0);
        HRESULT hr = 0;
        
        // release buffers
        if (context->framebuffer_rtv != nullptr) { context->framebuffer_rtv->Release();context->framebuffer_rtv = nullptr; }
        if (context->framebuffer != nullptr) { context->framebuffer->Release(); context->framebuffer = nullptr; }
        
        // resize buffers
        hr = context->swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        hr = context->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&context->framebuffer));
        hr = gfx_d3d11_state.device->CreateRenderTargetView(context->framebuffer, 0, &context->framebuffer_rtv);
        
        // set new size
        context->size = size;
        
    }
}



//- buffer functions 

function gfx_buffer_t 
gfx_buffer_create_ex(gfx_buffer_desc_t desc) {
    
}

function gfx_buffer_t 
gfx_buffer_create(gfx_buffer_type type, u32 size, const void* initial_data = nullptr) {
    
}

function void                gfx_buffer_release(gfx_buffer_t buffer_handle);
function void                gfx_buffer_update(gfx_buffer_t buffer_handle, u32 offset, u32 size, const void* data);

function void                gfx_buffer_set_vertex(gfx_buffer_t buffer_handle, u32 slot = 0, u32 stride = 0);
function void                gfx_buffer_set_index(gfx_buffer_t buffer_handle);
function void                gfx_buffer_set_constant(gfx_buffer_t buffer_handle, u32 slot = 0);
function void                gfx_buffer_set_structured(gfx_buffer_t buffer_handle, u32 slot = 0);

function gfx_buffer_type     gfx_buffer_get_type(gfx_buffer_t buffer_handle);
function gfx_usage           gfx_buffer_get_usage(gfx_buffer_t buffer_handle);
function u32                 gfx_buffer_get_size(gfx_buffer_t buffer_handle);
function u32                 gfx_buffer_get_stride(gfx_buffer_t buffer_handle);




//~ d3d11 specific functions


//- resource 

function gfx_d3d11_resource_t* 
ffx_d3d11_resource_alloc(gfx_d3d11_resource_type type) {
    
    gfx_d3d11_resource_t* resource = gfx_d3d11_state.resource_free;
    if (resource != nullptr) {
        stack_pop(gfx_d3d11_state.resource_free);
    } else {
        resource = (gfx_d3d11_resource_t*)arena_alloc(gfx_d3d11_state.arena, sizeof(gfx_d3d11_resource_t));
    }
    memset(resource, 0, sizeof(gfx_d3d11_resource_t));
    dll_push_back(gfx_d3d11_state.resource_first, gfx_d3d11_state.resource_last, resource);
    
    resource->type = type;
    
    return resource;
}

function void 
gfx_d3d11_resource_release(gfx_d3d11_resource_t* resource) {
    dll_remove(gfx_d3d11_state.resource_first, gfx_d3d11_state.resource_last, resource);
    stack_push(gfx_d3d11_state.resource_free, resource);
}






#endif // SORA_GFX_D3D11_CPP