// sora_gfx_d3d11.cpp

#ifndef SORA_GFX_D3D11_CPP
#define SORA_GFX_D3D11_CPP

//- include libs

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

//- implementation

//- state functions

function void 
gfx_init() {
    
	// create arenas
	gfx_d3d11_state.arena = arena_create(megabytes(256));
	
	// init resource list
	gfx_d3d11_state.resource_first = nullptr;
	gfx_d3d11_state.resource_last = nullptr;
	gfx_d3d11_state.resource_free = nullptr;
	
	// init context list
	gfx_d3d11_state.context_first = nullptr;
	gfx_d3d11_state.context_last = nullptr;
	gfx_d3d11_state.context_free = nullptr;
	gfx_d3d11_state.context_active = nullptr;
    
	// create device
	HRESULT hr = 0;
	u32 device_flags = 0;
	D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
    
#ifdef BUILD_DEBUG
	device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // BUILD DEBUG
    
	hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, device_flags, feature_levels, array_count(feature_levels), D3D11_SDK_VERSION, &gfx_d3d11_state.device, 0, &gfx_d3d11_state.device_context);
    gfx_check(hr, "failed to create d3d11 device.");
    
	// get dxgi device, adaptor, and factory
	hr = gfx_d3d11_state.device->QueryInterface(__uuidof(IDXGIDevice1), (void**)(&gfx_d3d11_state.dxgi_device));
	gfx_check(hr, "failed to get dxgi device.");
	hr = gfx_d3d11_state.dxgi_device->GetAdapter(&gfx_d3d11_state.dxgi_adapter);
	gfx_check(hr, "failed to get dxgi adaptor.");
	hr = gfx_d3d11_state.dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)(&gfx_d3d11_state.dxgi_factory));
	gfx_check(hr, "failed to get dxgi factory.");
	
	// create pipeline assets
    
	// samplers
	{
		D3D11_SAMPLER_DESC sampler_desc = { 0 };
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = gfx_d3d11_state.device->CreateSamplerState(&sampler_desc, &gfx_d3d11_state.nearest_wrap_sampler);
		gfx_check(hr, "faield to create nearest wrap sampler.");
        
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = gfx_d3d11_state.device->CreateSamplerState(&sampler_desc, &gfx_d3d11_state.linear_wrap_sampler);
		gfx_check(hr, "faield to create linear wrap sampler.");
        
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = gfx_d3d11_state.device->CreateSamplerState(&sampler_desc, &gfx_d3d11_state.linear_clamp_sampler);
		gfx_check(hr, "faield to create linear clamp sampler.");
        
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = gfx_d3d11_state.device->CreateSamplerState(&sampler_desc, &gfx_d3d11_state.nearest_clamp_sampler);
		gfx_check(hr, "faield to create nearest clamp sampler.");
	}
    
	// depth stencil states
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = { 0 };
		depth_stencil_desc.DepthEnable = true;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
		depth_stencil_desc.StencilEnable = false;
		depth_stencil_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		depth_stencil_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		hr = gfx_d3d11_state.device->CreateDepthStencilState(&depth_stencil_desc, &gfx_d3d11_state.depth_stencil_state);
		gfx_check(hr, "failed to create depth stencil state.");
        
		depth_stencil_desc.DepthEnable = false;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
		depth_stencil_desc.StencilEnable = false;
		depth_stencil_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		depth_stencil_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		hr = gfx_d3d11_state.device->CreateDepthStencilState(&depth_stencil_desc, &gfx_d3d11_state.no_depth_stencil_state);
		gfx_check(hr, "failed to create non depth stencil state.");
	}
    
	// rasterizers 
	{
		D3D11_RASTERIZER_DESC rasterizer_desc = { 0 };
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = true;
		rasterizer_desc.MultisampleEnable = true;
		rasterizer_desc.AntialiasedLineEnable = true;
		hr = gfx_d3d11_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_d3d11_state.solid_cull_none_rasterizer);
		gfx_check(hr, "failed to create solid cull none rasterizer state.");
		
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_FRONT;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = true;
		rasterizer_desc.MultisampleEnable = true;
		rasterizer_desc.AntialiasedLineEnable = true;
		hr = gfx_d3d11_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_d3d11_state.solid_cull_front_rasterizer);
		gfx_check(hr, "failed to create solid cull front rasterizer state.");
        
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = true;
		rasterizer_desc.MultisampleEnable = true;
		rasterizer_desc.AntialiasedLineEnable = true;
		hr = gfx_d3d11_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_d3d11_state.solid_cull_back_rasterizer);
		gfx_check(hr, "failed to create solid cull back rasterizer state.");
        
		rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = true;
		rasterizer_desc.MultisampleEnable = true;
		rasterizer_desc.AntialiasedLineEnable = true;
		hr = gfx_d3d11_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_d3d11_state.wireframe_cull_none_rasterizer);
		gfx_check(hr, "failed to create wireframe cull none rasterizer state.");
        
		rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
		rasterizer_desc.CullMode = D3D11_CULL_FRONT;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = true;
		rasterizer_desc.MultisampleEnable = true;
		rasterizer_desc.AntialiasedLineEnable = true;
		hr = gfx_d3d11_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_d3d11_state.wireframe_cull_front_rasterizer);
		gfx_check(hr, "failed to create wireframe cull front rasterizer state.");
        
		rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
		rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.DepthBias = 0;
		rasterizer_desc.DepthBiasClamp = 0.0f;
		rasterizer_desc.SlopeScaledDepthBias = 0.0f;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.ScissorEnable = true;
		rasterizer_desc.MultisampleEnable = true;
		rasterizer_desc.AntialiasedLineEnable = true;
		hr = gfx_d3d11_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_d3d11_state.wireframe_cull_back_rasterizer);
		gfx_check(hr, "failed to create wireframe cull back rasterizer state.");
	}
    
	// blend state
	{
		D3D11_BLEND_DESC blend_state_desc = { 0 };
		blend_state_desc.AlphaToCoverageEnable = TRUE;
		blend_state_desc.IndependentBlendEnable = FALSE;
		blend_state_desc.RenderTarget[0].BlendEnable = FALSE;
		blend_state_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_state_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend_state_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_state_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_state_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blend_state_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = gfx_d3d11_state.device->CreateBlendState(&blend_state_desc, &gfx_d3d11_state.blend_state);
		gfx_check(hr, "failed to create blend state.");
	}
    
}

function void
gfx_release() {
    
	// release assets
	if (gfx_d3d11_state.linear_wrap_sampler != nullptr) { gfx_d3d11_state.linear_wrap_sampler->Release(); }
	if (gfx_d3d11_state.linear_clamp_sampler != nullptr) { gfx_d3d11_state.linear_clamp_sampler->Release(); }
	if (gfx_d3d11_state.nearest_wrap_sampler != nullptr) { gfx_d3d11_state.nearest_wrap_sampler->Release(); }
	if (gfx_d3d11_state.nearest_clamp_sampler != nullptr) { gfx_d3d11_state.nearest_clamp_sampler->Release(); }
	if (gfx_d3d11_state.depth_stencil_state != nullptr) { gfx_d3d11_state.depth_stencil_state->Release(); }
	if (gfx_d3d11_state.no_depth_stencil_state != nullptr) { gfx_d3d11_state.no_depth_stencil_state->Release(); }
	if (gfx_d3d11_state.solid_cull_none_rasterizer != nullptr) { gfx_d3d11_state.solid_cull_none_rasterizer->Release(); }
	if (gfx_d3d11_state.solid_cull_front_rasterizer != nullptr) { gfx_d3d11_state.solid_cull_front_rasterizer->Release(); }
	if (gfx_d3d11_state.solid_cull_back_rasterizer != nullptr) { gfx_d3d11_state.solid_cull_back_rasterizer->Release(); }
	if (gfx_d3d11_state.wireframe_cull_none_rasterizer != nullptr) { gfx_d3d11_state.wireframe_cull_none_rasterizer->Release(); }
	if (gfx_d3d11_state.wireframe_cull_front_rasterizer != nullptr) { gfx_d3d11_state.wireframe_cull_front_rasterizer->Release(); }
	if (gfx_d3d11_state.wireframe_cull_back_rasterizer != nullptr) { gfx_d3d11_state.wireframe_cull_back_rasterizer->Release(); }
	if (gfx_d3d11_state.blend_state != nullptr) { gfx_d3d11_state.blend_state->Release(); }
    
	// release d3d11 devices
	if (gfx_d3d11_state.dxgi_factory != nullptr) { gfx_d3d11_state.dxgi_factory->Release(); }
	if (gfx_d3d11_state.dxgi_adapter != nullptr) { gfx_d3d11_state.dxgi_adapter->Release(); }
	if (gfx_d3d11_state.dxgi_device != nullptr) { gfx_d3d11_state.dxgi_device->Release(); }
	if (gfx_d3d11_state.device_context != nullptr) { gfx_d3d11_state.device_context->Release(); }
	if (gfx_d3d11_state.device != nullptr) { gfx_d3d11_state.device->Release(); }
    
	// release arenas
	arena_release(gfx_d3d11_state.arena);
    
}

//~ low level functions

//- render state functions

function void
gfx_draw(u32 vertex_count, u32 start_index) {
	gfx_d3d11_state.device_context->Draw(vertex_count, start_index);
}

function void
gfx_draw_indexed(u32 index_count, u32 start_index, u32 offset) {
	gfx_d3d11_state.device_context->DrawIndexed(index_count, start_index, offset);
}

function void
gfx_draw_instanced(u32 vertex_count, u32 instance_count, u32 start_vertex_index, u32 start_instance_index) {
	gfx_d3d11_state.device_context->DrawInstanced(vertex_count, instance_count, start_vertex_index, start_instance_index);
}

function void 
gfx_dispatch(u32 thread_group_x, u32 thread_group_y, u32 thread_group_z) {
	gfx_d3d11_state.device_context->Dispatch(thread_group_x, thread_group_y, thread_group_z);
}

function void
gfx_set_context(gfx_handle_t context) {
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    gfx_d3d11_state.device_context->OMSetRenderTargets(1, &d3d11_context->framebuffer_rtv, nullptr);
    // TODO: maybe allow custom blend states.
    gfx_d3d11_state.device_context->OMSetBlendState(gfx_d3d11_state.blend_state, nullptr, 0xffffffff);
}

function void 
gfx_set_sampler(gfx_filter_mode filter_mode, gfx_wrap_mode wrap_mode, u32 slot) {
    
	// choose samplers
	ID3D11SamplerState* sampler = nullptr;
    
	if (filter_mode == gfx_filter_linear && wrap_mode == gfx_wrap_repeat) {
		sampler = gfx_d3d11_state.linear_wrap_sampler;
	} else if (filter_mode == gfx_filter_linear && wrap_mode == gfx_wrap_clamp) {
		sampler = gfx_d3d11_state.linear_clamp_sampler;
	} else if (filter_mode == gfx_filter_nearest && wrap_mode == gfx_wrap_repeat) {
		sampler = gfx_d3d11_state.nearest_wrap_sampler;
	} else if (filter_mode == gfx_filter_nearest && wrap_mode == gfx_wrap_clamp) {
		sampler = gfx_d3d11_state.nearest_clamp_sampler;
	}
	
	// bind sampler
	gfx_d3d11_state.device_context->PSSetSamplers(slot, 1, &sampler);
}

function void 
gfx_set_topology(gfx_topology_type topology_type) {
	D3D11_PRIMITIVE_TOPOLOGY topology = gfx_d3d11_prim_top_from_top_type(topology_type);
	gfx_d3d11_state.device_context->IASetPrimitiveTopology(topology);
}

function void 
gfx_set_rasterizer(gfx_fill_mode fill_mode, gfx_cull_mode cull_mode) {
    
	ID3D11RasterizerState* rasterizer = nullptr;
    
	// get rasterizer state
	if (fill_mode == gfx_fill_solid) {
		if (cull_mode == gfx_cull_none) {
			rasterizer = gfx_d3d11_state.solid_cull_none_rasterizer;
		} else if (cull_mode == gfx_cull_front) {
			rasterizer = gfx_d3d11_state.solid_cull_front_rasterizer;
		} else if (cull_mode == gfx_cull_back) {
			rasterizer = gfx_d3d11_state.solid_cull_back_rasterizer;
		}
	} else if (fill_mode == gfx_fill_wireframe) {
		if (cull_mode == gfx_cull_none) {
			rasterizer = gfx_d3d11_state.wireframe_cull_none_rasterizer;
		} else if (cull_mode == gfx_cull_front) {
			rasterizer = gfx_d3d11_state.wireframe_cull_front_rasterizer;
		} else if (cull_mode == gfx_cull_back) {
			rasterizer = gfx_d3d11_state.wireframe_cull_back_rasterizer;
		}
	}
    
	// bind rasterizer state
	gfx_d3d11_state.device_context->RSSetState(rasterizer);
}

function void 
gfx_set_viewport(rect_t viewport) {
	D3D11_VIEWPORT d3d11_viewport = { viewport.x0, viewport.y0, viewport.x1, viewport.y1, 0.0f, 1.0f };
	gfx_d3d11_state.device_context->RSSetViewports(1, &d3d11_viewport);
}

function void 
gfx_set_scissor(rect_t scissor) {
	D3D11_RECT d3d11_rect = {
		(i32)scissor.x0, (i32)scissor.y0,
		(i32)scissor.x1 , (i32)scissor.y1
	};
	gfx_d3d11_state.device_context->RSSetScissorRects(1, &d3d11_rect);
}

function void 
gfx_set_depth_mode(gfx_depth_mode depth_mode) {
	ID3D11DepthStencilState* state = nullptr;
	switch (depth_mode) {
		case gfx_depth: { state = gfx_d3d11_state.depth_stencil_state; break; }
		case gfx_depth_none: { state = gfx_d3d11_state.no_depth_stencil_state; break; }
	}
	gfx_d3d11_state.device_context->OMSetDepthStencilState(state, 1);
}

function void 
gfx_set_pipeline(gfx_pipeline_t pipeline) {
	gfx_set_sampler(pipeline.filter_mode, pipeline.wrap_mode, 0);
	gfx_set_topology(pipeline.topology);
	gfx_set_rasterizer(pipeline.fill_mode, pipeline.cull_mode);
	gfx_set_viewport(pipeline.viewport);
	gfx_set_scissor(pipeline.scissor);
}


function void
gfx_set_buffer(gfx_handle_t buffer, u32 slot, u32 stride) {
    
    // figure out what shaders are currently bound.
    // TODO: it would probably just be better to set some flags
    // in the graphics state and manually keep track instead of checking 
    // everytime like this, but its a problem for another time.
    
    ID3D11VertexShader* vertex_shader = nullptr;
    ID3D11PixelShader* pixel_shader = nullptr;
    ID3D11ComputeShader* compute_shader = nullptr;
    gfx_d3d11_state.device_context->VSGetShader(&vertex_shader, nullptr, nullptr);
    gfx_d3d11_state.device_context->PSGetShader(&pixel_shader, nullptr, nullptr);
    gfx_d3d11_state.device_context->CSGetShader(&compute_shader, nullptr, nullptr);
    
    // if we have a buffer to bind
    if (!gfx_handle_equals(buffer, { 0 })) {
        
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(buffer.data[0]);
        
        // if we have a buffer
        switch (resource->buffer.desc.type) {
            
            case gfx_buffer_type_vertex: {
                u32 offset = 0;
                gfx_d3d11_state.device_context->IASetVertexBuffers(slot, 1, &resource->buffer.id, &stride, &offset);
                break;
            }
            
            case gfx_buffer_type_index: {
                gfx_d3d11_state.device_context->IASetIndexBuffer(resource->buffer.id, DXGI_FORMAT_R32_UINT, 0);
                break;
            }
            
            case gfx_buffer_type_constant: {
                
                if (vertex_shader != nullptr) { gfx_d3d11_state.device_context->VSSetConstantBuffers(slot, 1, &resource->buffer.id); }
                if (pixel_shader != nullptr) { gfx_d3d11_state.device_context->PSSetConstantBuffers(slot, 1, &resource->buffer.id); }
                if (compute_shader != nullptr) { gfx_d3d11_state.device_context->CSSetConstantBuffers(slot, 1, &resource->buffer.id); }
                
                break;
            }
            
            case gfx_buffer_type_structured: {
                
                if (vertex_shader != nullptr) { gfx_d3d11_state.device_context->VSSetShaderResources(slot, 1, &resource->buffer.srv); }
                if (pixel_shader != nullptr) { gfx_d3d11_state.device_context->PSSetConstantBuffers(slot, 1, &resource->buffer.id); }
                if (compute_shader != nullptr) {
                    UINT initial_counts = -1;
                    gfx_d3d11_state.device_context->CSSetUnorderedAccessViews(slot, 1, &resource->buffer.uav, &initial_counts);
                }
                
                break;
            }
        }
        
    } else {
        
        if (compute_shader != nullptr) {
            ID3D11UnorderedAccessView* null_uav = nullptr;
            gfx_d3d11_state.device_context->CSSetUnorderedAccessViews(slot, 1, &null_uav, nullptr);
        }
        
    }
    
    // release shaders
    if (vertex_shader != nullptr) { vertex_shader->Release(); }
    if (pixel_shader != nullptr) { pixel_shader->Release(); }
    if (compute_shader != nullptr) { compute_shader->Release(); }
    
    
}

function void
gfx_set_texture(gfx_handle_t texture, u32 slot) {
    
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
	if (!gfx_handle_equals(texture, {0})) {
        
        ID3D11VertexShader* vertex_shader = nullptr;
        ID3D11PixelShader* pixel_shader = nullptr;
        ID3D11ComputeShader* compute_shader = nullptr;
        gfx_d3d11_state.device_context->VSGetShader(&vertex_shader, nullptr, nullptr);
        gfx_d3d11_state.device_context->PSGetShader(&pixel_shader, nullptr, nullptr);
        gfx_d3d11_state.device_context->CSGetShader(&compute_shader, nullptr, nullptr);
        
        if (vertex_shader != nullptr) {
            gfx_d3d11_state.device_context->VSSetShaderResources(slot, 1, &resource->texture.srv);
            vertex_shader->Release();
        }
        
        if (pixel_shader != nullptr) {
            gfx_d3d11_state.device_context->PSSetShaderResources(slot, 1, &resource->texture.srv);
            pixel_shader->Release();
        }
        
        if (compute_shader != nullptr) {
            gfx_d3d11_state.device_context->CSSetShaderResources(slot, 1, &resource->texture.srv);
            gfx_d3d11_state.device_context->CSSetUnorderedAccessViews(slot, 1, &resource->texture.uav, nullptr);
            compute_shader->Release();
        }
        
	}
}


function void
gfx_set_texture_array(gfx_handle_t* textures, u32 texture_count, u32 slot) {
    
	temp_t scratch = scratch_begin();
	
	if (textures != nullptr) {
        
		// make list of srvs
		ID3D11ShaderResourceView** srvs = (ID3D11ShaderResourceView**)arena_alloc(scratch.arena, sizeof(ID3D11ShaderResourceView*) * texture_count);
		for (i32 i = 0; i < texture_count; i++) {
			gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(textures[i].data[0]);
			srvs[i] = resource->texture.srv;
		}
		
        ID3D11VertexShader* vertex_shader = nullptr;
        ID3D11PixelShader* pixel_shader = nullptr;
        ID3D11ComputeShader* compute_shader = nullptr;
        gfx_d3d11_state.device_context->VSGetShader(&vertex_shader, nullptr, nullptr);
        gfx_d3d11_state.device_context->PSGetShader(&pixel_shader, nullptr, nullptr);
        gfx_d3d11_state.device_context->CSGetShader(&compute_shader, nullptr, nullptr);
        
		// bind srvs
        
        if (vertex_shader != nullptr) {
            gfx_d3d11_state.device_context->VSSetShaderResources(slot, texture_count, srvs);
            vertex_shader->Release();
        }
        
        if (pixel_shader != nullptr) {
            gfx_d3d11_state.device_context->PSSetShaderResources(slot, texture_count, srvs);
            pixel_shader->Release();
        }
        
        if (compute_shader != nullptr) {
            gfx_d3d11_state.device_context->CSSetShaderResources(slot, texture_count, srvs);
            // TODO: we might want the uavs
            //gfx_d3d11_state.device_context->CSSetUnorderedAccessViews(slot, texture_count, &resource->texture.uav, nullptr);
            compute_shader->Release();
        }
        
	} else {
		// make null list of srvs
		ID3D11ShaderResourceView** null_srvs = (ID3D11ShaderResourceView**)arena_calloc(scratch.arena, sizeof(ID3D11ShaderResourceView*) * texture_count);
		
		// bind null srvs
		gfx_d3d11_state.device_context->PSSetShaderResources(slot, texture_count, null_srvs);
		gfx_d3d11_state.device_context->CSSetShaderResources(slot, texture_count, null_srvs);
	}
    
	scratch_end(scratch);
    
}


function void
gfx_set_shader(gfx_handle_t shader) {
	if (!gfx_handle_equals(shader, {0})) {
		gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
        
        gfx_shader_flags flags = resource->shader.desc.flags;
        
        if (flags & gfx_shader_flag_vertex) {
            gfx_d3d11_state.device_context->VSSetShader(resource->shader.vertex_shader, 0, 0);
            gfx_d3d11_state.device_context->IASetInputLayout(resource->shader.input_layout);
            
            // unset compute
            gfx_d3d11_state.device_context->CSSetShader(nullptr, 0, 0);
            
        } else if (flags & gfx_shader_flag_pixel) {
            gfx_d3d11_state.device_context->PSSetShader(resource->shader.pixel_shader, 0, 0);
            
            gfx_d3d11_state.device_context->CSSetShader(nullptr, 0, 0);
            
        } else if (flags & gfx_shader_flag_geometry) {
            gfx_d3d11_state.device_context->GSSetShader(resource->shader.geometry_shader, 0, 0);
        } else if (flags & gfx_shader_flag_hull) {
            gfx_d3d11_state.device_context->HSSetShader(resource->shader.hull_shader, 0, 0);
        } else if (flags & gfx_shader_flag_domain) {
            gfx_d3d11_state.device_context->DSSetShader(resource->shader.domain_shader, 0, 0);
        } else if (flags & gfx_shader_flag_compute) {
            gfx_d3d11_state.device_context->CSSetShader(resource->shader.compute_shader, 0, 0);
            
            // unset vertex and pixel
            gfx_d3d11_state.device_context->VSSetShader(nullptr, 0, 0);
            gfx_d3d11_state.device_context->PSSetShader(nullptr, 0, 0);
        }
        
    } else {
        
        // set all shader to null
        gfx_d3d11_state.device_context->VSSetShader(nullptr, 0, 0);
        gfx_d3d11_state.device_context->PSSetShader(nullptr, 0, 0);
        gfx_d3d11_state.device_context->CSSetShader(nullptr, 0, 0);
    }
}

function void
gfx_set_render_target(gfx_handle_t render_target) {
    
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
    
    if (gfx_handle_equals(render_target, { 0 })) {
        gfx_d3d11_state.device_context->OMSetRenderTargets(0, nullptr, nullptr);
    } else {
        if (resource->render_target.rtv != nullptr) {
            gfx_d3d11_state.device_context->OMSetRenderTargets(1, &resource->render_target.rtv, resource->render_target.dsv);
            gfx_d3d11_state.device_context->OMSetBlendState(gfx_d3d11_state.blend_state, nullptr, 0xffffffff);
        }
    }
}


// TODO: relocate this
function void 
gfx_blit(gfx_handle_t dst, gfx_handle_t src) {
    
    gfx_handle_t texture_dst = { 0 };
    gfx_handle_t texture_src = { 0 };
    
    // figure out dst resource
    gfx_d3d11_resource_t* resource_dst = (gfx_d3d11_resource_t*)(dst.data[0]);
    switch (resource_dst->type) {
        case gfx_resource_type_texture: { texture_dst = dst; break; }
        case gfx_resource_type_render_target: {
            texture_dst = resource_dst->render_target.color_texture;
            break;
        }
    }
    
    // figure out src resource
    gfx_d3d11_resource_t* resource_src = (gfx_d3d11_resource_t*)(src.data[0]);
    switch (resource_src->type) {
        case gfx_resource_type_texture: { texture_src = src; break; }
        case gfx_resource_type_render_target: {
            texture_src = resource_src->render_target.color_texture;
            break;
        }
    }
    
    gfx_texture_blit(texture_dst, texture_src);
    
}


//- context functions

function gfx_handle_t
gfx_context_create(os_handle_t window) {
    
    // get from resource pool or create one
    gfx_d3d11_context_t* context = gfx_d3d11_state.context_free;
    if (context != nullptr) {
        stack_pop(gfx_d3d11_state.context_free);
    } else {
        context = (gfx_d3d11_context_t*)arena_alloc(gfx_d3d11_state.arena, sizeof(gfx_d3d11_context_t));
    }
    memset(context, 0, sizeof(gfx_d3d11_context_t));
    dll_push_back(gfx_d3d11_state.context_first, gfx_d3d11_state.context_last, context);
    
    // fill
    context->window = window;
    context->resolution = os_window_get_size(window);
    
    // create swapchain
    HRESULT hr = 0;
    DXGI_SWAP_CHAIN_DESC1 swapchain_desc = { 0 };
    swapchain_desc.Width = context->resolution.x;
    swapchain_desc.Height = context->resolution.y;
    swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchain_desc.Stereo = FALSE;
    swapchain_desc.SampleDesc.Count = 1;
    swapchain_desc.SampleDesc.Quality = 0;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.BufferCount = 2;
    swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchain_desc.Flags = 0;
    
    os_w32_window_t* w32_window = os_w32_window_from_handle(window);
    hr = gfx_d3d11_state.dxgi_factory->CreateSwapChainForHwnd(gfx_d3d11_state.device, w32_window->handle, &swapchain_desc, 0, 0, &context->swapchain);
    gfx_check(hr, "failed to create swapchain.");
    
    // get framebuffer from swapchain
    hr = context->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&context->framebuffer));
    gfx_check(hr, "failed to get framebuffer from swapchain.");
    
    // create render target view
    hr = gfx_d3d11_state.device->CreateRenderTargetView(context->framebuffer, 0, &context->framebuffer_rtv);
    gfx_check(hr, "failed to create render target view");
    
    // create queries
    
    D3D11_QUERY_DESC query_desc = { 0 };
    query_desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
    gfx_d3d11_state.device->CreateQuery(&query_desc, &context->disjoint_query);
    
    query_desc.Query = D3D11_QUERY_TIMESTAMP;
    gfx_d3d11_state.device->CreateQuery(&query_desc, &context->start_query);
    gfx_d3d11_state.device->CreateQuery(&query_desc, &context->end_query);
    
    // create handle
    gfx_handle_t handle = { (u64)context };
    
    return handle;
}

function void 
gfx_context_release(gfx_handle_t context) {
    
    // get context
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    
    // release d3d11
    // TODO: release queries
    
    if (d3d11_context->framebuffer_rtv != nullptr) { d3d11_context->framebuffer_rtv->Release(); }
    if (d3d11_context->framebuffer != nullptr) { d3d11_context->framebuffer->Release(); }
    if (d3d11_context->swapchain != nullptr) { d3d11_context->swapchain->Release(); }
    
    // push to free stack
    dll_remove(gfx_d3d11_state.context_first, gfx_d3d11_state.context_last, d3d11_context);
    stack_push(gfx_d3d11_state.context_free, d3d11_context);
    
}

function void
gfx_context_update(gfx_handle_t context) {
    
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    
    // resize
    uvec2_t window_size = os_window_get_size(d3d11_context->window);
    if (!uvec2_equals(d3d11_context->resolution, window_size)) {
        gfx_context_resize(context, window_size);
    }
    
}

function void
gfx_context_clear(gfx_handle_t context, color_t clear_color) {
    
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    
    FLOAT clear_color_array[] = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
    gfx_d3d11_state.device_context->ClearRenderTargetView(d3d11_context->framebuffer_rtv, clear_color_array);
    
    // query
    //gfx_d3d11_state.device_context->Begin(d3d11_context->disjoint_query);
    //gfx_d3d11_state.device_context->End(d3d11_context->start_query);
    
}

function void
gfx_context_present(gfx_handle_t context) {
    
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    
    if (!os_window_is_minimized(d3d11_context->window)) {
        d3d11_context->swapchain->Present(1, 0);
    } else {
        os_sleep(16);
    }
    
    // query
    //gfx_d3d11_state.device_context->End(d3d11_context->end_query);
    //gfx_d3d11_state.device_context->End(d3d11_context->disjoint_query);
    
    // calculate frame time
    //D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint_data;
    //u64 start_time = 0;
    //u64 end_time = 0;
    
    //while (gfx_d3d11_state.device_context->GetData(d3d11_context->disjoint_query, &disjoint_data, sizeof(disjoint_data), 0) != S_OK);
    //while (gfx_d3d11_state.device_context->GetData(d3d11_context->start_query, &start_time, sizeof(u64), 0) != S_OK);
    //while (gfx_d3d11_state.device_context->GetData(d3d11_context->end_query, &end_time, sizeof(u64), 0) != S_OK);
    
    //if (!disjoint_data.Disjoint) {
    //f64 delta = (f64)(end_time - start_time) / (f64)(disjoint_data.Frequency);
    //d3d11_context->delta_time = delta;
    //}
    
    gfx_d3d11_state.device_context->ClearState();
}

function void
gfx_context_resize(gfx_handle_t context, uvec2_t resolution) {
    
    // get context
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    
    // skip is invalid resolution
    if (resolution.x == 0 || resolution.y == 0) {
        return;
    }
    
    // flush correct state
    gfx_d3d11_state.device_context->Flush();
    
    gfx_d3d11_state.device_context->OMSetRenderTargets(0, 0, 0);
    HRESULT hr = 0;
    
    // release buffers
    if (d3d11_context->framebuffer_rtv != nullptr) { d3d11_context->framebuffer_rtv->Release(); d3d11_context->framebuffer_rtv = nullptr; }
    if (d3d11_context->framebuffer != nullptr) { d3d11_context->framebuffer->Release(); d3d11_context->framebuffer = nullptr; }
    
    // TODO: handles errors here
    
    // resize framebuffer
    hr = d3d11_context->swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    //if (FAILED(hr)) { goto context_resize_error; }
    hr = d3d11_context->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&d3d11_context->framebuffer));
    //if (FAILED(hr)) { goto context_resize_error; }
    hr = gfx_d3d11_state.device->CreateRenderTargetView(d3d11_context->framebuffer, 0, &d3d11_context->framebuffer_rtv);
    //if (FAILED(hr)) { goto context_resize_error; }
    
    // set new resolution
    d3d11_context->resolution = resolution;
    
}

function void
gfx_context_blit(gfx_handle_t context, gfx_handle_t texture) {
    
    // get context
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    gfx_d3d11_resource_t* texture_resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
    if (texture_resource->texture.desc.format == gfx_texture_format_rgba8 &&
        texture_resource->texture.desc.size.x == d3d11_context->resolution.x &&
        texture_resource->texture.desc.size.y == d3d11_context->resolution.y) {
        
        // resolve if higher sample count
        if (texture_resource->texture.desc.sample_count > 1) {
            gfx_d3d11_state.device_context->ResolveSubresource(d3d11_context->framebuffer, 0, texture_resource->texture.id, 0, gfx_d3d11_dxgi_format_from_texture_format(gfx_texture_format_rgba8));
        } else {
            gfx_d3d11_state.device_context->CopyResource(d3d11_context->framebuffer, texture_resource->texture.id);
        }
    }
    
}

function uvec2_t 
gfx_context_get_size(gfx_handle_t context) {
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    return d3d11_context->resolution;
}

function f64
gfx_context_get_delta_time(gfx_handle_t context) {
    gfx_d3d11_context_t* d3d11_context = (gfx_d3d11_context_t*)(context.data[0]);
    return d3d11_context->delta_time;
}

//- buffer functions

function gfx_handle_t
gfx_buffer_create_ex(gfx_buffer_desc_t desc, void* initial_data) {
    
    // get from resource pool or create one
    gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_buffer);
    
    // fill description
    resource->buffer.desc = desc;
    
    // constant buffer aligning
    if (desc.type == gfx_buffer_type_constant) {
        desc.size = (desc.size + 15) & ~15; // align to multiple of 16
    }
    
    // create buffer
    HRESULT hr = 0;
    D3D11_BUFFER_DESC buffer_desc = { 0 };
    buffer_desc.ByteWidth = desc.size;
    buffer_desc.Usage = gfx_d3d11_d3d11_usage_from_gfx_usage(desc.usage);
    buffer_desc.BindFlags = gfx_d3d11_bind_flags_from_buffer_type(desc.type);
    buffer_desc.CPUAccessFlags = gfx_d3d11_access_flags_from_gfx_usage(desc.usage);
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    
    // fix structured buffers
    if (desc.type == gfx_buffer_type_structured) {
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.ByteWidth = desc.size * desc.stride;
        buffer_desc.CPUAccessFlags = 0;
        buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        buffer_desc.StructureByteStride = desc.stride;
    }
    
    // initial data
    D3D11_SUBRESOURCE_DATA buffer_data = { 0 };
    if (initial_data != nullptr) {
        buffer_data.pSysMem = initial_data;
    }
    
    hr = gfx_d3d11_state.device->CreateBuffer(&buffer_desc, initial_data ? &buffer_data : nullptr, &resource->buffer.id);
    
    if (FAILED(hr)) {
        printf("failed to create buffer!\n");
    }
    
    // if structured, we will need an srv and uav
    if (desc.type == gfx_buffer_type_structured) {
        
        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = { 0 };
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uav_desc.Buffer.FirstElement = 0;
        uav_desc.Buffer.NumElements = desc.size;
        
        hr = gfx_d3d11_state.device->CreateUnorderedAccessView(resource->buffer.id, &uav_desc, &resource->buffer.uav);
        
        if (FAILED(hr)) {
            printf("failed to create buffer uav!\n");
        }
        
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = { 0 };
        srv_desc.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srv_desc.Buffer.FirstElement = 0;
        srv_desc.Buffer.NumElements = desc.size;
        
        hr = gfx_d3d11_state.device->CreateShaderResourceView(resource->buffer.id, &srv_desc,  &resource->buffer.srv);
        
        if (FAILED(hr)) {
            printf("failed to create buffer srv!\n");
        }
        
    }
    
    gfx_handle_t handle = { (u64)resource };
    
    return handle;
}

function gfx_handle_t
gfx_buffer_create(gfx_buffer_type type, u32 size, void* initial_data){
    return gfx_buffer_create_ex({ type, size, gfx_usage_stream, 0 }, initial_data);
}

function void 
gfx_buffer_release(gfx_handle_t buffer) {
    
    if (!gfx_handle_equals(buffer, { 0 })) {
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(buffer.data[0]);
        
        // release d3d11 buffer
        if (resource->buffer.id != nullptr) { resource->buffer.id->Release(); resource->buffer.id = nullptr; }
        
        gfx_d3d11_resource_release(resource);
    }
    
}

function void
gfx_buffer_fill(gfx_handle_t buffer, void* data, u32 size) {
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(buffer.data[0]);
    
    // align size if constant buffer
    if (resource->buffer.desc.type == gfx_buffer_type_constant) {
        size = (size + 15) & ~15; // align to multiple of 16
    }
    
    D3D11_MAPPED_SUBRESOURCE mapped_subresource;
    HRESULT hr = gfx_d3d11_state.device_context->Map(resource->buffer.id, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped_subresource);
    gfx_check(hr, "failed to map buffer.");
    memcpy(mapped_subresource.pData, data, size);
    gfx_d3d11_state.device_context->Unmap(resource->buffer.id, 0);
}


//- texture functions

function gfx_handle_t
gfx_texture_create_ex(gfx_texture_desc_t desc, void* data) {
    
    gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_texture);
    HRESULT hr = 0;
    
    // fill description
    resource->texture.desc = desc;
    
    // create texture
    gfx_d3d11_texture_create_resources(resource, data);
    
    gfx_handle_t handle = { 0 };
    if (!FAILED(hr)) { 
        //log_infof("successfully created texture: %.*s", resource->texture_desc.name.size, resource->texture_desc.name.data);
        handle = { (u64)resource };
    }
    
    return handle;
}

function gfx_handle_t
gfx_texture_create(uvec2_t size, gfx_texture_format format, void* data) {
    
    // fill description
    gfx_texture_desc_t desc = { 0 };
    desc.name = str("");
    desc.size = size;
    desc.format = format;
    desc.flags = 0;
    desc.type = gfx_texture_type_2d;
    desc.sample_count = 1;
    desc.usage = gfx_usage_dynamic;
    
    // create and return texture
    gfx_handle_t handle = gfx_texture_create_ex(desc, data);
    
    return handle;
}

function void
gfx_texture_release(gfx_handle_t texture) {
    
    if (gfx_handle_equals(texture, { 0 })) {
        return;
    }
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
    // release d3d11
    if (resource->texture.id != nullptr) { resource->texture.id->Release(); resource->texture.id = nullptr;}
    if (resource->texture.srv != nullptr) { resource->texture.srv->Release(); resource->texture.srv = nullptr; }
    if (resource->texture.uav != nullptr) { resource->texture.uav->Release(); resource->texture.uav = nullptr; }
    
    gfx_d3d11_resource_release(resource);
    
}

function uvec2_t
gfx_texture_get_size(gfx_handle_t texture) {
    uvec2_t result = { 0 };
    
    if (!gfx_handle_equals(texture, { 0 })) {
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
        result = resource->texture.desc.size;
    }
    
    return result;
}

function void 
gfx_texture_resize(gfx_handle_t texture, uvec2_t size) {
    if (size.x > 0 && size.y > 0) {
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
        resource->texture.desc.size = size;
        gfx_d3d11_texture_create_resources(resource, nullptr);
    }
}

function void 
gfx_texture_fill(gfx_handle_t texture, void* data) {
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
    D3D11_BOX dst_box = { 0, 0, 0, resource->texture.desc.size.x, resource->texture.desc.size.y, 1 };
    u32 src_row_pitch = resource->texture.desc.size.x * gfx_d3d11_byte_size_from_texture_format(resource->texture.desc.format);
    gfx_d3d11_state.device_context->UpdateSubresource(resource->texture.id, 0, &dst_box, data, src_row_pitch, 0);
}

function void 
gfx_texture_fill_region(gfx_handle_t texture, rect_t region, void* data) {
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
    D3D11_BOX dst_box = {
        (UINT)region.x0, (UINT)region.y0, 0,
        (UINT)region.x1, (UINT)region.y1, 1,
    };
    
    if (dst_box.right > resource->texture.desc.size.x || dst_box.bottom > resource->texture.desc.size.y) {
        log_errorf("gfx_texture_fill_region: incorrect region size!");
        return;
    }
    
    u32 bytes = gfx_d3d11_byte_size_from_texture_format(resource->texture.desc.format);
    u32 src_row_pitch = (region.x1 - region.x0) * bytes;
    gfx_d3d11_state.device_context->UpdateSubresource(resource->texture.id, 0, &dst_box, data, src_row_pitch, 0);
}

function void 
gfx_texture_blit(gfx_handle_t texture_dst, gfx_handle_t texture_src) {
    
    // get resources
    gfx_d3d11_resource_t* resource_dst = (gfx_d3d11_resource_t*)(texture_dst.data[0]);
    gfx_d3d11_resource_t* resource_src = (gfx_d3d11_resource_t*)(texture_src.data[0]);
    
    if (resource_dst->texture.desc.format == resource_src->texture.desc.format &&
        resource_dst->texture.desc.size.x == resource_src->texture.desc.size.x &&
        resource_dst->texture.desc.size.y == resource_src->texture.desc.size.y &&
        resource_dst->texture.desc.type ==   resource_src->texture.desc.type) {
        
        // resolve if higher sample count
        if (resource_src->texture.desc.sample_count > 1) {
            gfx_d3d11_state.device_context->ResolveSubresource(resource_dst->texture.id, 0, resource_src->texture.id, 0, gfx_d3d11_dxgi_format_from_texture_format(resource_dst->texture.desc.format));
        } else {
            gfx_d3d11_state.device_context->CopyResource(resource_dst->texture.id, resource_src->texture.id);
        }
    }
}

function void
gfx_d3d11_texture_create_resources(gfx_d3d11_resource_t* resource, void* data) {
    
    // release old resources if needed
    if (resource->texture.id != nullptr) { resource->texture.id->Release(); resource->texture.id = nullptr;}
    if (resource->texture.srv != nullptr) { resource->texture.srv->Release(); resource->texture.srv = nullptr; }
    if (resource->texture.uav != nullptr) { resource->texture.uav->Release(); resource->texture.uav = nullptr; }
    
    // create texture
    HRESULT hr = 0;
    b8 has_uav = true;
    
    // determine bind flags
    D3D11_BIND_FLAG bind_flags = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE);
    
    if (resource->texture.desc.sample_count == 1) {
        bind_flags = (D3D11_BIND_FLAG)(bind_flags | D3D11_BIND_UNORDERED_ACCESS);
    }
    
    if (resource->texture.desc.flags & gfx_texture_flag_render_target) {
        if (gfx_texture_format_is_depth(resource->texture.desc.format)) {
            bind_flags = (D3D11_BIND_FLAG)(bind_flags | D3D11_BIND_DEPTH_STENCIL);
            bind_flags = (D3D11_BIND_FLAG)(bind_flags & ~D3D11_BIND_UNORDERED_ACCESS);
            has_uav = false;
        } else {
            bind_flags = (D3D11_BIND_FLAG)(bind_flags | D3D11_BIND_RENDER_TARGET);
        }
    }
    
    // fill out description
    D3D11_TEXTURE2D_DESC texture_desc = { 0 };
    texture_desc.Width = resource->texture.desc.size.x;
    texture_desc.Height = resource->texture.desc.size.y;
    texture_desc.ArraySize = 1;
    texture_desc.Format = gfx_d3d11_dxgi_format_from_texture_format(resource->texture.desc.format);
    texture_desc.SampleDesc.Count = resource->texture.desc.sample_count;
    texture_desc.SampleDesc.Quality = (resource->texture.desc.sample_count > 1) ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
    texture_desc.Usage = gfx_d3d11_d3d11_usage_from_gfx_usage(resource->texture.desc.usage);
    texture_desc.BindFlags = bind_flags;
    texture_desc.CPUAccessFlags = gfx_d3d11_access_flags_from_gfx_usage(resource->texture.desc.usage);
    texture_desc.MipLevels = 1;
    texture_desc.MiscFlags = 0;
    
    // initial data
    D3D11_SUBRESOURCE_DATA texture_data = { 0 };
    if (data != nullptr) {
        texture_data.pSysMem = data;
        texture_data.SysMemPitch = resource->texture.desc.size.x * gfx_d3d11_byte_size_from_texture_format(resource->texture.desc.format);
    }
    
    // create texture2d
    hr = gfx_d3d11_state.device->CreateTexture2D(&texture_desc, data ? &texture_data : nullptr, &resource->texture.id);
    if (FAILED(hr)) {
        os_graphical_message(true, str("[gfx] texture error"), str("failed to create texture!"));
        os_abort(1);
    }
    
    // create srv
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = { 0 };
    srv_desc.Format = gfx_d3d11_srv_format_from_texture_format(resource->texture.desc.format);
    if (resource->texture.desc.sample_count > 1) {
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    } else {
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1; // TODO: support mips.
    }
    hr = gfx_d3d11_state.device->CreateShaderResourceView(resource->texture.id, &srv_desc, &resource->texture.srv);
    if (FAILED(hr)) {
        os_graphical_message(true, str("[gfx] texture error"), str("failed to create srv for texture!"));
        os_abort(1);
    }
    
    // create uav
    
    if (resource->texture.desc.sample_count == 1 && has_uav) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = { 0 };
        uav_desc.Format = texture_desc.Format;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = 0;
        
        hr = gfx_d3d11_state.device->CreateUnorderedAccessView(resource->texture.id, &uav_desc, &resource->texture.uav);
        if (FAILED(hr)) {
            os_graphical_message(true, str("[gfx] texture error"), str("failed to create uav for texture!"));
            os_abort(1);
        }
    }
    
    if (FAILED(hr)) {
        log_errorf("gfx_texture_create_resources: texture error occured!");
    }
    
}

//- shader functions

function gfx_handle_t
gfx_shader_create_ex(gfx_shader_desc_t desc) {
    
    // get from resource pool or create
    gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_shader);
    
    // fill description
    resource->shader.desc = desc;
    
    // create handle
    gfx_handle_t handle = { (u64)resource };
    
    return handle;
}

function gfx_handle_t
gfx_shader_create(str_t name, gfx_shader_flags flags) {
    
    // fill description
    gfx_shader_desc_t desc = { 0 };
    desc.name = name;
    desc.flags = flags;
    
    // create and return shader
    gfx_handle_t shader = gfx_shader_create_ex( desc);
    
    return shader;
}

function void
gfx_shader_release(gfx_handle_t shader) {
    
    if (gfx_handle_equals(shader, { 0 })) {
        return;
    }
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
    
    // release shaders
    gfx_shader_flags flags = resource->shader.desc.flags;
    if (flags & gfx_shader_flag_vertex) { 
        if (resource->shader.vertex_shader != nullptr) { resource->shader.vertex_shader->Release(); resource->shader.vertex_shader = nullptr; }
        if (resource->shader.input_layout != nullptr) { resource->shader.input_layout->Release(); resource->shader.input_layout = nullptr; }
    } else if (flags & gfx_shader_flag_pixel) { 
        if (resource->shader.pixel_shader != nullptr) { resource->shader.pixel_shader->Release(); resource->shader.pixel_shader = nullptr; }
    } else if (flags & gfx_shader_flag_geometry) {
        if (resource->shader.geometry_shader != nullptr) { resource->shader.geometry_shader->Release(); resource->shader.geometry_shader = nullptr; }
    } else if (flags & gfx_shader_flag_hull) { 
        if (resource->shader.hull_shader != nullptr) { resource->shader.hull_shader->Release(); resource->shader.hull_shader = nullptr; }
    } else if (flags & gfx_shader_flag_domain) { 
        if (resource->shader.domain_shader != nullptr) { resource->shader.domain_shader->Release(); resource->shader.domain_shader = nullptr; }
    } else if (flags & gfx_shader_flag_compute) { 
        if (resource->shader.compute_shader != nullptr) { resource->shader.compute_shader->Release(); resource->shader.compute_shader = nullptr; }
    }
    
    // release shader blob
    if (resource->shader.shader_blob != nullptr) { resource->shader.shader_blob->Release(); resource->shader.shader_blob = nullptr; }
    
    // release resource
    gfx_d3d11_resource_release(resource);
}

function b8
gfx_shader_compile(gfx_handle_t shader, str_t src) {
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
    
    HRESULT hr;
    b8 success = true;
    ID3DBlob* shader_blob = nullptr;
    ID3DBlob* error_blob = nullptr;
    
    // TODO: don't leave this in.
    u32 compile_flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_SKIP_VALIDATION | D3DCOMPILE_SKIP_OPTIMIZATION;
    
#if defined(BUILD_DEBUG)
    //compile_flags |= D3DCOMPILE_DEBUG;
#endif 
    
    // comile shader
    char* shader_name = (char*)resource->shader.desc.name.data;
    
    char* shader_entry_point = (char*)gfx_d3d11_shader_entry_name_from_shader_flags(resource->shader.desc.flags).data;
    if (resource->shader.desc.entry_point.size != 0) {
        shader_entry_point = (char*)resource->shader.desc.entry_point.data;
    }
    char* shader_target = (char*)gfx_d3d11_shader_target_from_shader_flags(resource->shader.desc.flags).data;
    
    hr = D3DCompile(src.data, src.size, shader_name, 0, 0, shader_entry_point, shader_target, compile_flags, 0, &shader_blob, &error_blob);
    if (error_blob) {
        success = false;
        cstr error_msg = (cstr)error_blob->GetBufferPointer();
        log_errorf("gfx_shader_compile: failed to compile shader '%s': \n\n%s", resource->shader.desc.name.data, error_msg);
    }
    
    if (error_blob != nullptr) {
        if (shader_blob != nullptr) { shader_blob->Release(); }
        error_blob->Release();
    } else {
        gfx_shader_set_binary(shader, shader_blob->GetBufferPointer(), shader_blob->GetBufferSize());
        resource->shader.shader_blob = shader_blob;
    }
    
    return success;
    
}

function void
gfx_shader_set_binary(gfx_handle_t shader, void* binary, u32 binary_size) {
    
    HRESULT hr;
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
    
    // if we succeeded, release old shaders and replace
    
    
    gfx_shader_flags flags = resource->shader.desc.flags;
    if (flags & gfx_shader_flag_vertex) { 
        
        temp_t scratch = scratch_begin();
        
        if (resource->shader.vertex_shader != nullptr) {
            resource->shader.vertex_shader->Release(); 
            resource->shader.vertex_shader = nullptr;
        }
        
        hr = gfx_d3d11_state.device->CreateVertexShader(binary, binary_size, nullptr, &resource->shader.vertex_shader);
        
        // create input layout
        
        ID3D11ShaderReflection* reflector = nullptr;
        D3DReflect(binary, binary_size, __uuidof(ID3D11ShaderReflection), (void**)&reflector);
        D3D11_SHADER_DESC shader_desc;
        reflector->GetDesc(&shader_desc);
        
        // allocate input element array
        D3D11_INPUT_ELEMENT_DESC* input_element_desc = (D3D11_INPUT_ELEMENT_DESC*)arena_alloc(scratch.arena, sizeof(D3D11_INPUT_ELEMENT_DESC) * shader_desc.InputParameters);
        D3D11_INPUT_CLASSIFICATION classification = gfx_d3d11_input_class_from_shader_flags(resource->shader.desc.flags);
        u32 data_step_rate = (resource->shader.desc.flags & gfx_shader_flag_per_instance) ? (u32)1 : (u32)0;
        
        for (i32 i = 0; i < shader_desc.InputParameters; i++) {
            
            D3D11_SIGNATURE_PARAMETER_DESC param_desc;
            reflector->GetInputParameterDesc(i, &param_desc);
            
            input_element_desc[i].Format = gfx_d3d11_dxgi_format_from_mask_component(param_desc.Mask, param_desc.ComponentType);
            input_element_desc[i].SemanticName = param_desc.SemanticName;
            input_element_desc[i].SemanticIndex = param_desc.SemanticIndex;
            input_element_desc[i].InputSlot = 0;
            input_element_desc[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            input_element_desc[i].InputSlotClass = classification;
            input_element_desc[i].InstanceDataStepRate = data_step_rate;
            
        }
        
        // create input layout
        hr = gfx_d3d11_state.device->CreateInputLayout(input_element_desc, shader_desc.InputParameters, binary, binary_size, &resource->shader.input_layout);
        
        // release reflection
        reflector->Release();
        scratch_end(scratch);
        
        if (FAILED(hr)) {
            log_errorf("gfx_shader_compile: faild to create shader input layout for: '%s'!", resource->shader.desc.name.data);
        }
        
        
    } else if (flags & gfx_shader_flag_pixel) { 
        
        if (resource->shader.pixel_shader != nullptr) {
            resource->shader.pixel_shader->Release(); 
            resource->shader.pixel_shader = nullptr;
        }
        
        hr = gfx_d3d11_state.device->CreatePixelShader(binary, binary_size, nullptr, &resource->shader.pixel_shader);
        
    } else if (flags & gfx_shader_flag_geometry) {
        
        if (resource->shader.geometry_shader != nullptr) {
            resource->shader.geometry_shader->Release(); 
            resource->shader.geometry_shader = nullptr;
        }
        
        hr = gfx_d3d11_state.device->CreateGeometryShader(binary, binary_size, nullptr, &resource->shader.geometry_shader);
        
    } else if (flags & gfx_shader_flag_hull) { 
        
        if (resource->shader.hull_shader != nullptr) {
            resource->shader.hull_shader->Release(); 
            resource->shader.hull_shader = nullptr;
        }
        
        hr = gfx_d3d11_state.device->CreateHullShader(binary, binary_size, nullptr, &resource->shader.hull_shader);
        
    } else if (flags & gfx_shader_flag_domain) { 
        
        if (resource->shader.domain_shader != nullptr) {
            resource->shader.domain_shader->Release(); 
            resource->shader.domain_shader = nullptr;
        }
        
        hr = gfx_d3d11_state.device->CreateDomainShader(binary, binary_size, nullptr, &resource->shader.domain_shader);
        
    } else if (flags & gfx_shader_flag_compute) { 
        
        if (resource->shader.compute_shader != nullptr) {
            resource->shader.compute_shader->Release(); 
            resource->shader.compute_shader = nullptr;
        }
        
        hr = gfx_d3d11_state.device->CreateComputeShader(binary, binary_size, nullptr, &resource->shader.compute_shader);
        
    }
    
}

function void
gfx_shader_get_binary(gfx_handle_t shader, void** out_binary, u32* out_binary_size) {
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
    
    if (resource->shader.shader_blob != nullptr) {
        *out_binary = resource->shader.shader_blob->GetBufferPointer();
        *out_binary_size = resource->shader.shader_blob->GetBufferSize();
    }
    
}


//- render target functions

function gfx_handle_t 
gfx_render_target_create_ex(gfx_render_target_desc_t desc) {
    
    // get a resource and fill desc
    gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_render_target);
    resource->render_target.desc = desc;
    
    // create handle
    gfx_handle_t handle = { (u64)resource };
    
    // internal allocate resources
    gfx_render_target_create_resources(handle);
    
    return handle;
    
}

function gfx_handle_t 
gfx_render_target_create(uvec2_t size, gfx_texture_format colorbuffer_format, gfx_texture_format depthbuffer_format) {
    
    gfx_render_target_desc_t desc = { 0 };
    desc.colorbuffer_format = colorbuffer_format;
    desc.depthbuffer_format = depthbuffer_format;
    desc.size = size;
    desc.sample_count = 1;
    
    gfx_handle_t handle = gfx_render_target_create_ex(desc);
    
    return handle;
}

function void
gfx_render_target_release(gfx_handle_t render_target) {
    if (!gfx_handle_equals(render_target, { 0 })) {
        
        // get resource
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
        
        //  release d3d11 assets
        if (!gfx_handle_equals(resource->render_target.color_texture, { 0 })) { gfx_texture_release(resource->render_target.color_texture); resource->render_target.color_texture = { 0 }; }
        if (!gfx_handle_equals(resource->render_target.depth_texture, {0})) { gfx_texture_release(resource->render_target.depth_texture); resource->render_target.depth_texture = { 0 }; }
        if (resource->render_target.rtv != nullptr) { resource->render_target.rtv->Release(); resource->render_target.rtv = nullptr; }
        if (resource->render_target.dsv != nullptr) { resource->render_target.dsv->Release(); resource->render_target.dsv = nullptr; }
        
        // release resource
        gfx_d3d11_resource_release(resource);
    }
}

function void 
gfx_render_target_resize(gfx_handle_t render_target, uvec2_t size) {
    
    // resize if needed
    if (size.x > 0 && size.y > 0) {
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
        resource->render_target.desc.size = size;
        gfx_render_target_create_resources(render_target);
    }
}

function void
gfx_render_target_clear(gfx_handle_t render_target, color_t clear_color, f32 depth) {
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
    
    // clear colorbuffer
    if (resource->render_target.rtv != nullptr) {
        FLOAT clear_color_array[] = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
        gfx_d3d11_state.device_context->ClearRenderTargetView(resource->render_target.rtv, clear_color_array);
    }
    
    // clear depthbuffer
    if (resource->render_target.dsv != nullptr) {
        gfx_d3d11_state.device_context->ClearDepthStencilView(resource->render_target.dsv, D3D11_CLEAR_DEPTH, depth, 0);
    }
    
}

function void 
gfx_render_target_create_resources(gfx_handle_t render_target) {
    
    // get resource
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
    
    // release current if exist
    if (!gfx_handle_equals(resource->render_target.color_texture, {0})) {
        gfx_texture_release(resource->render_target.color_texture);
    }
    
    HRESULT hr = 0;
    
    // create if we have a colorbuffer format
    if (resource->render_target.desc.colorbuffer_format != gfx_texture_format_null) {
        
        gfx_texture_desc_t color_texture_desc = { 0 };
        color_texture_desc.name = str("");
        color_texture_desc.size = resource->render_target.desc.size;
        color_texture_desc.format = resource->render_target.desc.colorbuffer_format;
        color_texture_desc.flags = gfx_texture_flag_render_target;
        color_texture_desc.type = gfx_texture_type_2d;
        color_texture_desc.sample_count = resource->render_target.desc.sample_count;
        color_texture_desc.usage = gfx_usage_dynamic;
        
        resource->render_target.color_texture = gfx_texture_create_ex(color_texture_desc);
        
        // create rtv
        // release old rtv if needed
        if (resource->render_target.rtv != nullptr) {
            resource->render_target.rtv->Release();
        }
        
        // create new rtv
        D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = { 0 };
        rtv_desc.Format = gfx_d3d11_dxgi_format_from_texture_format(resource->render_target.desc.colorbuffer_format);
        if (resource->render_target.desc.sample_count > 1) {
            rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        } else {
            rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtv_desc.Texture2D.MipSlice = 0;
        }
        
        // get texture resource
        gfx_d3d11_resource_t* color_texture_resource = (gfx_d3d11_resource_t*)(resource->render_target.color_texture.data[0]);
        hr = gfx_d3d11_state.device->CreateRenderTargetView(color_texture_resource->texture.id, &rtv_desc, &resource->render_target.rtv);
        gfx_check(hr, "failed to create the color rtv for render target.");
    }
    
    // if has depth target
    if (resource->render_target.desc.depthbuffer_format != gfx_texture_format_null) {
        
        // release current if exist
        if (!gfx_handle_equals(resource->render_target.depth_texture, {0})) {
            gfx_texture_release(resource->render_target.depth_texture);
        }
        
        // create new texture
        gfx_texture_desc_t depth_texture_desc = { 0 };
        depth_texture_desc.name = str("");
        depth_texture_desc.size = resource->render_target.desc.size;
        depth_texture_desc.format = gfx_texture_format_d32;
        depth_texture_desc.flags = gfx_texture_flag_render_target;
        depth_texture_desc.type = gfx_texture_type_2d;
        depth_texture_desc.sample_count = resource->render_target.desc.sample_count;
        depth_texture_desc.usage = gfx_usage_dynamic;
        
        resource->render_target.depth_texture = gfx_texture_create_ex(depth_texture_desc);
        
        // release old dsv if needed
        if (resource->render_target.dsv != nullptr) {
            resource->render_target.dsv->Release();
        }
        
        // create new dsv
        D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = { 0 };
        dsv_desc.Format = gfx_d3d11_dsv_format_from_texture_format(gfx_texture_format_d32);
        if (resource->render_target.desc.sample_count > 1) {
            dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        } else {
            dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsv_desc.Texture2D.MipSlice = 0;
        }
        
        gfx_d3d11_resource_t* depth_texture_resource = (gfx_d3d11_resource_t*)(resource->render_target.depth_texture.data[0]);
        hr = gfx_d3d11_state.device->CreateDepthStencilView(depth_texture_resource->texture.id, &dsv_desc, &resource->render_target.dsv);
        gfx_check(hr, "failed to create the depth dsv for render target.");
        
    } 
    
}

function uvec2_t 
gfx_render_target_get_size(gfx_handle_t render_target) {
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
    return resource->render_target.desc.size;
}

function gfx_handle_t
gfx_render_target_get_texture(gfx_handle_t render_target) {
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
    return resource->render_target.color_texture;
}

//- d3d11 specific functions

//- resource functions

function gfx_d3d11_resource_t* 
gfx_d3d11_resource_create(gfx_resource_type type) {
    
    // grab from free list of allocate one
    gfx_d3d11_resource_t* resource = gfx_d3d11_state.resource_free;
    if (resource != nullptr) {
        stack_pop(gfx_d3d11_state.resource_free);
    } else {
        resource = (gfx_d3d11_resource_t*)arena_alloc(gfx_d3d11_state.arena, sizeof(gfx_d3d11_resource_t));
    }
    memset(resource, 0, sizeof(gfx_d3d11_resource_t));
    dll_push_back(gfx_d3d11_state.resource_first, gfx_d3d11_state.resource_last, resource);
    
    // set type
    resource->type = type;
    
    return resource;
}

function void 
gfx_d3d11_resource_release(gfx_d3d11_resource_t* resource) {
    dll_remove(gfx_d3d11_state.resource_first, gfx_d3d11_state.resource_last, resource);
    stack_push(gfx_d3d11_state.resource_free, resource);
}


// enum functions
function D3D11_USAGE
gfx_d3d11_d3d11_usage_from_gfx_usage(gfx_usage usage) {
    D3D11_USAGE result_usage = D3D11_USAGE_DEFAULT;
    switch (usage) {
        case gfx_usage_static: { result_usage = D3D11_USAGE_IMMUTABLE; break; }
        case gfx_usage_dynamic: { result_usage = D3D11_USAGE_DEFAULT; break; }
        case gfx_usage_stream: { result_usage = D3D11_USAGE_DYNAMIC; break; }
    }
    return result_usage;
}

function UINT
gfx_d3d11_access_flags_from_gfx_usage(gfx_usage usage) {
    UINT access_flag = 0;
    switch (usage) {
        case gfx_usage_static: { access_flag = 0; break; }
        case gfx_usage_dynamic: { access_flag = 0; break; }
        case gfx_usage_stream: { access_flag = D3D11_CPU_ACCESS_WRITE; break; }
    }
    return access_flag;
}

function D3D11_BIND_FLAG 
gfx_d3d11_bind_flags_from_buffer_type(gfx_buffer_type type) {
    D3D11_BIND_FLAG flag = (D3D11_BIND_FLAG)0;
    switch (type) {
        case gfx_buffer_type_vertex: { flag = D3D11_BIND_VERTEX_BUFFER; break; }
        case gfx_buffer_type_index: { flag = D3D11_BIND_INDEX_BUFFER; break; }
        case gfx_buffer_type_constant: { flag = D3D11_BIND_CONSTANT_BUFFER; break; }
        case gfx_buffer_type_structured: { flag = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS); break; }
    }
    return flag;
}

function DXGI_FORMAT 
gfx_d3d11_dxgi_format_from_texture_format(gfx_texture_format format) {
    DXGI_FORMAT result = DXGI_FORMAT_R8G8B8A8_UNORM;
    switch (format) {
        case gfx_texture_format_r8: { result = DXGI_FORMAT_R8_UNORM; break; }
        case gfx_texture_format_rg8: { result = DXGI_FORMAT_R8G8_UNORM; break; }
        case gfx_texture_format_rgba8: { result = DXGI_FORMAT_R8G8B8A8_UNORM; break; }
        case gfx_texture_format_bgra8: { result = DXGI_FORMAT_B8G8R8A8_UNORM; break; }
        case gfx_texture_format_r16: { result = DXGI_FORMAT_R16_UNORM; break; }
        case gfx_texture_format_rgba16: { result = DXGI_FORMAT_R16G16B16A16_UNORM; break; }
        case gfx_texture_format_r32: { result = DXGI_FORMAT_R32_FLOAT; break; }
        case gfx_texture_format_rg32: { result = DXGI_FORMAT_R32G32_FLOAT; break; }
        case gfx_texture_format_rgba32: { result = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
        case gfx_texture_format_d24s8: { result = DXGI_FORMAT_R24G8_TYPELESS; break; }
        case gfx_texture_format_d32: { result = DXGI_FORMAT_R32_TYPELESS; break; }
    }
    return result;
}

function DXGI_FORMAT
gfx_d3d11_srv_format_from_texture_format(gfx_texture_format format) {
    DXGI_FORMAT result = DXGI_FORMAT_R8G8B8A8_UNORM;
    switch (format) {
        case gfx_texture_format_r8: { result = DXGI_FORMAT_R8_UNORM; break; }
        case gfx_texture_format_rg8: { result = DXGI_FORMAT_R8G8_UNORM; break; }
        case gfx_texture_format_rgba8: { result = DXGI_FORMAT_R8G8B8A8_UNORM; break; }
        case gfx_texture_format_bgra8: { result = DXGI_FORMAT_B8G8R8A8_UNORM; break; }
        case gfx_texture_format_r16: { result = DXGI_FORMAT_R16_UNORM; break; }
        case gfx_texture_format_rgba16: { result = DXGI_FORMAT_R16G16B16A16_UNORM; break; }
        case gfx_texture_format_r32: { result = DXGI_FORMAT_R32_FLOAT; break; }
        case gfx_texture_format_rg32: { result = DXGI_FORMAT_R32G32_FLOAT; break; }
        case gfx_texture_format_rgba32: { result = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
        case gfx_texture_format_d24s8: { result = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; break; }
        case gfx_texture_format_d32: { result = DXGI_FORMAT_R32_FLOAT; break; }
    }
    return result;
}

function DXGI_FORMAT
gfx_d3d11_dsv_format_from_texture_format(gfx_texture_format format) {
    DXGI_FORMAT result = DXGI_FORMAT_R8G8B8A8_UNORM;
    switch (format) {
        case gfx_texture_format_d24s8: { result = DXGI_FORMAT_D24_UNORM_S8_UINT; break; }
        case gfx_texture_format_d32: { result = DXGI_FORMAT_D32_FLOAT; break; }
    }
    return result;
}

function u32 
gfx_d3d11_byte_size_from_texture_format(gfx_texture_format format) {
    u32 result = 0;
    switch (format) {
        case gfx_texture_format_r8: { result = 1; break; }
        case gfx_texture_format_rg8: { result = 2; break; }
        case gfx_texture_format_rgba8: { result = 4; break; }
        case gfx_texture_format_bgra8: { result = 4; break; }
        case gfx_texture_format_r16: { result = 2; break; }
        case gfx_texture_format_rgba16: { result = 8; break; }
        case gfx_texture_format_r32: { result = 4; break; }
        case gfx_texture_format_rg32: { result = 8; break; }
        case gfx_texture_format_rgba32: { result = 16; break; }
        case gfx_texture_format_d24s8: { result = 16; break; }
        case gfx_texture_format_d32: { result = 8; break; }
    }
    return result;
}

function D3D11_PRIMITIVE_TOPOLOGY 
gfx_d3d11_prim_top_from_top_type(gfx_topology_type type) {
    D3D11_PRIMITIVE_TOPOLOGY topology = (D3D11_PRIMITIVE_TOPOLOGY)0;
    switch (type) {
        case gfx_topology_points: { topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break; }
        case gfx_topology_lines: { topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break; }
        case gfx_topology_line_strip: { topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break; }
        case gfx_topology_tris: { topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break; }
        case gfx_topology_tri_strip: { topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break; }
    }
    return topology;
}

function DXGI_FORMAT 
gfx_d3d11_dxgi_format_from_vertex_format(gfx_vertex_format format) {
    DXGI_FORMAT result = DXGI_FORMAT_UNKNOWN;
    switch (format) {
        case gfx_vertex_format_float: { result = DXGI_FORMAT_R32_FLOAT; break; }
        case gfx_vertex_format_float2: { result = DXGI_FORMAT_R32G32_FLOAT; break; }
        case gfx_vertex_format_float3: { result = DXGI_FORMAT_R32G32B32_FLOAT; break; }
        case gfx_vertex_format_float4: { result = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
        case gfx_vertex_format_int: { result = DXGI_FORMAT_R32_SINT; break; }
        case gfx_vertex_format_int2: { result = DXGI_FORMAT_R32G32_SINT; break; }
        case gfx_vertex_format_int3: { result = DXGI_FORMAT_R32G32B32_SINT; break; }
        case gfx_vertex_format_int4: { result = DXGI_FORMAT_R32G32B32A32_SINT; break; }
        case gfx_vertex_format_uint: { result = DXGI_FORMAT_R32_UINT; break; }
        case gfx_vertex_format_uint2: { result = DXGI_FORMAT_R32G32_UINT; break; }
        case gfx_vertex_format_uint3: { result = DXGI_FORMAT_R32G32B32_UINT; break; }
        case gfx_vertex_format_uint4: { result = DXGI_FORMAT_R32G32B32A32_UINT; break; }
    }
    return result;
}

function D3D11_INPUT_CLASSIFICATION 
gfx_d3d11_input_class_from_shader_flags(gfx_shader_flags flags) {
    D3D11_INPUT_CLASSIFICATION shader_classification = (D3D11_INPUT_CLASSIFICATION)0;
    
    if (flags & gfx_shader_flag_per_instance) {
        shader_classification = D3D11_INPUT_PER_INSTANCE_DATA;
    } else {
        shader_classification = D3D11_INPUT_PER_VERTEX_DATA; 
    }
    
    return shader_classification;
}

function str_t 
gfx_d3d11_shader_entry_name_from_shader_flags(gfx_shader_flags flags) {
    
    str_t result = { 0 };
    
    if (flags & gfx_shader_flag_vertex) { result = str("vs_main"); }
    else if (flags & gfx_shader_flag_pixel) { result = str("ps_main"); }
    else if (flags & gfx_shader_flag_geometry) { result = str("gs_main"); }
    else if (flags & gfx_shader_flag_hull) { result = str("hs_main"); }
    else if (flags & gfx_shader_flag_domain) { result = str("ds_main"); }
    else if (flags & gfx_shader_flag_compute) { result = str("cs_main"); }
    else { result = str("main"); }
    
    return result;
}

function str_t 
gfx_d3d11_shader_target_from_shader_flags(gfx_shader_flags flags) {
    
    str_t result = { 0 };
    
    if (flags & gfx_shader_flag_vertex) { result = str("vs_5_0"); }
    else if (flags & gfx_shader_flag_pixel) { result = str("ps_5_0"); }
    else if (flags & gfx_shader_flag_geometry) { result = str("gs_5_0"); }
    else if (flags & gfx_shader_flag_hull) { result = str("hs_5_0"); }
    else if (flags & gfx_shader_flag_domain) { result = str("ds_5_0"); }
    else if (flags & gfx_shader_flag_compute) { result = str("cs_5_0"); }
    else { result = str("main"); }
    
    return result;
}

function DXGI_FORMAT 
gfx_d3d11_dxgi_format_from_mask_component(u32 mask, D3D_REGISTER_COMPONENT_TYPE component) {
    
    DXGI_FORMAT result = DXGI_FORMAT_UNKNOWN;
    
    if (mask == 1) {
        if (component == D3D_REGISTER_COMPONENT_FLOAT32) result = DXGI_FORMAT_R32_FLOAT;
        else if (component == D3D_REGISTER_COMPONENT_UINT32) result = DXGI_FORMAT_R32_UINT;
        else if (component == D3D_REGISTER_COMPONENT_SINT32) result = DXGI_FORMAT_R32_SINT;
    } else if (mask <= 3) {
        if (component == D3D_REGISTER_COMPONENT_FLOAT32) result = DXGI_FORMAT_R32G32_FLOAT;
        else if (component == D3D_REGISTER_COMPONENT_UINT32) result = DXGI_FORMAT_R32G32_UINT;
        else if (component == D3D_REGISTER_COMPONENT_SINT32) result = DXGI_FORMAT_R32G32_SINT;
    } else if (mask <= 7) {
        if (component == D3D_REGISTER_COMPONENT_FLOAT32) result = DXGI_FORMAT_R32G32B32_FLOAT;
        else if (component == D3D_REGISTER_COMPONENT_UINT32) result = DXGI_FORMAT_R32G32B32_UINT;
        else if (component == D3D_REGISTER_COMPONENT_SINT32) result = DXGI_FORMAT_R32G32B32_SINT;
    } else if (mask <= 15) {
        if (component == D3D_REGISTER_COMPONENT_FLOAT32) result = DXGI_FORMAT_R32G32B32A32_FLOAT;
        else if (component == D3D_REGISTER_COMPONENT_UINT32) result = DXGI_FORMAT_R32G32B32A32_UINT;
        else if (component == D3D_REGISTER_COMPONENT_SINT32) result = DXGI_FORMAT_R32G32B32A32_SINT;
    }
    
    return result;
}

#endif // SORA_GFX_D3D11_CPP