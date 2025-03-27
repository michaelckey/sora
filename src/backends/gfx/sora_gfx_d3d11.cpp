// sora_gfx_d3d11.cpp

#ifndef SORA_GFX_D3D11_CPP
#define SORA_GFX_D3D11_CPP

// include libs
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// implementation

// state
function void 
gfx_init() {
    
	// create arenas
	gfx_state.resource_arena = arena_create(megabytes(64));
	gfx_state.renderer_arena = arena_create(megabytes(64));
	
	// init resource list
	gfx_state.resource_first = nullptr;
	gfx_state.resource_last = nullptr;
	gfx_state.resource_free = nullptr;
	
	// init renderer list
	gfx_state.renderer_first = nullptr;
	gfx_state.renderer_last = nullptr;
	gfx_state.renderer_free = nullptr;
	gfx_state.renderer_active = nullptr;
    
	// create device
	HRESULT hr = 0;
	u32 device_flags = 0;
	D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
    
#ifdef BUILD_DEBUG
	device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // BUILD DEBUG
    
	hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, device_flags, feature_levels, array_count(feature_levels), D3D11_SDK_VERSION, &gfx_state.device, 0, &gfx_state.device_context);
	gfx_assert(hr, "failed to create device.");
    
	// get dxgi device, adaptor, and factory
	hr = gfx_state.device->QueryInterface(__uuidof(IDXGIDevice1), (void**)(&gfx_state.dxgi_device));
	gfx_assert(hr, "failed to get dxgi device.");
	hr = gfx_state.dxgi_device->GetAdapter(&gfx_state.dxgi_adapter);
	gfx_assert(hr, "failed to get dxgi adaptor.");
	hr = gfx_state.dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)(&gfx_state.dxgi_factory));
	gfx_assert(hr, "failed to get dxgi factory.");
	
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
		hr = gfx_state.device->CreateSamplerState(&sampler_desc, &gfx_state.nearest_wrap_sampler);
		gfx_assert(hr, "faield to create nearest wrap sampler.");
        
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = gfx_state.device->CreateSamplerState(&sampler_desc, &gfx_state.linear_wrap_sampler);
		gfx_assert(hr, "faield to create linear wrap sampler.");
        
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = gfx_state.device->CreateSamplerState(&sampler_desc, &gfx_state.linear_clamp_sampler);
		gfx_assert(hr, "faield to create linear clamp sampler.");
        
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = gfx_state.device->CreateSamplerState(&sampler_desc, &gfx_state.nearest_clamp_sampler);
		gfx_assert(hr, "faield to create nearest clamp sampler.");
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
		hr = gfx_state.device->CreateDepthStencilState(&depth_stencil_desc, &gfx_state.depth_stencil_state);
		gfx_assert(hr, "failed to create depth stencil state.");
        
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
		hr = gfx_state.device->CreateDepthStencilState(&depth_stencil_desc, &gfx_state.no_depth_stencil_state);
		gfx_assert(hr, "failed to create non depth stencil state.");
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
		hr = gfx_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_state.solid_cull_none_rasterizer);
		gfx_assert(hr, "failed to create solid cull none rasterizer state.");
		
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
		hr = gfx_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_state.solid_cull_front_rasterizer);
		gfx_assert(hr, "failed to create solid cull front rasterizer state.");
        
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
		hr = gfx_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_state.solid_cull_back_rasterizer);
		gfx_assert(hr, "failed to create solid cull back rasterizer state.");
        
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
		hr = gfx_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_state.wireframe_cull_none_rasterizer);
		gfx_assert(hr, "failed to create wireframe cull none rasterizer state.");
        
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
		hr = gfx_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_state.wireframe_cull_front_rasterizer);
		gfx_assert(hr, "failed to create wireframe cull front rasterizer state.");
        
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
		hr = gfx_state.device->CreateRasterizerState(&rasterizer_desc, &gfx_state.wireframe_cull_back_rasterizer);
		gfx_assert(hr, "failed to create wireframe cull back rasterizer state.");
	}
    
	// blend state
	{
		D3D11_BLEND_DESC blend_state_desc = { 0 };
		//blend_state_desc.AlphaToCoverageEnable = TRUE;
		//blend_state_desc.IndependentBlendEnable = FALSE;
		blend_state_desc.RenderTarget[0].BlendEnable = TRUE;
		blend_state_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_state_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend_state_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_state_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_state_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blend_state_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend_state_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = gfx_state.device->CreateBlendState(&blend_state_desc, &gfx_state.blend_state);
		gfx_assert(hr, "failed to create blend state.");
	}
    
}

function void
gfx_release() {
    
	// release assets
	if (gfx_state.linear_wrap_sampler != nullptr) { gfx_state.linear_wrap_sampler->Release(); }
	if (gfx_state.linear_clamp_sampler != nullptr) { gfx_state.linear_clamp_sampler->Release(); }
	if (gfx_state.nearest_wrap_sampler != nullptr) { gfx_state.nearest_wrap_sampler->Release(); }
	if (gfx_state.nearest_clamp_sampler != nullptr) { gfx_state.nearest_clamp_sampler->Release(); }
	if (gfx_state.depth_stencil_state != nullptr) { gfx_state.depth_stencil_state->Release(); }
	if (gfx_state.no_depth_stencil_state != nullptr) { gfx_state.no_depth_stencil_state->Release(); }
	if (gfx_state.solid_cull_none_rasterizer != nullptr) { gfx_state.solid_cull_none_rasterizer->Release(); }
	if (gfx_state.solid_cull_front_rasterizer != nullptr) { gfx_state.solid_cull_front_rasterizer->Release(); }
	if (gfx_state.solid_cull_back_rasterizer != nullptr) { gfx_state.solid_cull_back_rasterizer->Release(); }
	if (gfx_state.wireframe_cull_none_rasterizer != nullptr) { gfx_state.wireframe_cull_none_rasterizer->Release(); }
	if (gfx_state.wireframe_cull_front_rasterizer != nullptr) { gfx_state.wireframe_cull_front_rasterizer->Release(); }
	if (gfx_state.wireframe_cull_back_rasterizer != nullptr) { gfx_state.wireframe_cull_back_rasterizer->Release(); }
	if (gfx_state.blend_state != nullptr) { gfx_state.blend_state->Release(); }
    
	// release d3d11 devices
	if (gfx_state.dxgi_factory != nullptr) { gfx_state.dxgi_factory->Release(); }
	if (gfx_state.dxgi_adapter != nullptr) { gfx_state.dxgi_adapter->Release(); }
	if (gfx_state.dxgi_device != nullptr) { gfx_state.dxgi_device->Release(); }
	if (gfx_state.device_context != nullptr) { gfx_state.device_context->Release(); }
	if (gfx_state.device != nullptr) { gfx_state.device->Release(); }
    
	// release arenas
	arena_release(gfx_state.renderer_arena);
	arena_release(gfx_state.resource_arena);
    
}

function void
gfx_update() {
	
    // TODO: maybe we don't need this, each renderer should be responsible for updating itself.
    
	// update renderers
	for (gfx_d3d11_renderer_t* renderer = gfx_state.renderer_first; renderer != 0; renderer = renderer->next) {
        
		gfx_handle_t renderer_handle = gfx_d3d11_handle_from_renderer(renderer);
        
		uvec2_t window_size = os_window_get_size(renderer->window);
		if (!uvec2_equals(renderer->resolution, window_size)) {
			gfx_renderer_resize(renderer_handle, window_size);
		}
        
	}
    
	//// hotload shaders
	//for (gfx_shader_t* shader = gfx_state.shader_first; shader != 0; shader = shader->next) {
    
	//	// if shader was created from file
	//	if (shader->desc.filepath.size != 0) {
    
	//		// check if file has been updated
	//		os_file_info_t info = os_file_get_info(shader->desc.filepath);
    
	//		// recompile 
	//		if (shader->last_modified != info.last_modified) {
    
	//			// get new source
	//			str_t src = os_file_read_all(gfx_state.scratch_arena, shader->desc.filepath);
    
	//			// try to compile
	//			gfx_shader_compile(shader, src);
    
	//			// set new last updated
	//			shader->last_modified = info.last_modified;
	//		}
	//	}
	//}
	
}

function void
gfx_draw(u32 vertex_count, u32 start_index) {
	gfx_state.device_context->Draw(vertex_count, start_index);
}

function void
gfx_draw_indexed(u32 index_count, u32 start_index, u32 offset) {
	gfx_state.device_context->DrawIndexed(index_count, start_index, offset);
}

function void
gfx_draw_instanced(u32 vertex_count, u32 instance_count, u32 start_vertex_index, u32 start_instance_index) {
	gfx_state.device_context->DrawInstanced(vertex_count, instance_count, start_vertex_index, start_instance_index);
}

function void 
gfx_dispatch(u32 thread_group_x, u32 thread_group_y, u32 thread_group_z) {
	gfx_state.device_context->Dispatch(thread_group_x, thread_group_y, thread_group_z);
}

function void 
gfx_set_sampler(gfx_filter_mode filter_mode, gfx_wrap_mode wrap_mode, u32 slot) {
    
	// choose samplers
	ID3D11SamplerState* sampler = nullptr;
    
	if (filter_mode == gfx_filter_linear && wrap_mode == gfx_wrap_repeat) {
		sampler = gfx_state.linear_wrap_sampler;
	} else if (filter_mode == gfx_filter_linear && wrap_mode == gfx_wrap_clamp) {
		sampler = gfx_state.linear_clamp_sampler;
	} else if (filter_mode == gfx_filter_nearest && wrap_mode == gfx_wrap_repeat) {
		sampler = gfx_state.nearest_wrap_sampler;
	} else if (filter_mode == gfx_filter_nearest && wrap_mode == gfx_wrap_clamp) {
		sampler = gfx_state.nearest_clamp_sampler;
	}
	
	// bind sampler
	gfx_state.device_context->PSSetSamplers(slot, 1, &sampler);
}

function void 
gfx_set_topology(gfx_topology_type topology_type) {
	D3D11_PRIMITIVE_TOPOLOGY topology = gfx_d3d11_prim_top_from_top_type(topology_type);
	gfx_state.device_context->IASetPrimitiveTopology(topology);
}

function void 
gfx_set_rasterizer(gfx_fill_mode fill_mode, gfx_cull_mode cull_mode) {
    
	ID3D11RasterizerState* rasterizer = nullptr;
    
	// get rasterizer state
	if (fill_mode == gfx_fill_solid) {
		if (cull_mode == gfx_cull_mode_none) {
			rasterizer = gfx_state.solid_cull_none_rasterizer;
		} else if (cull_mode == gfx_cull_mode_front) {
			rasterizer = gfx_state.solid_cull_front_rasterizer;
		} else if (cull_mode == gfx_cull_mode_back) {
			rasterizer = gfx_state.solid_cull_back_rasterizer;
		}
	} else if (fill_mode == gfx_fill_wireframe) {
		if (cull_mode == gfx_cull_mode_none) {
			rasterizer = gfx_state.wireframe_cull_none_rasterizer;
		} else if (cull_mode == gfx_cull_mode_front) {
			rasterizer = gfx_state.wireframe_cull_front_rasterizer;
		} else if (cull_mode == gfx_cull_mode_back) {
			rasterizer = gfx_state.wireframe_cull_back_rasterizer;
		}
	}
    
	// bind rasterizer state
	gfx_state.device_context->RSSetState(rasterizer);
}

function void 
gfx_set_viewport(rect_t viewport) {
	D3D11_VIEWPORT d3d11_viewport = { viewport.x0, viewport.y0, viewport.x1, viewport.y1, 0.0f, 1.0f };
	gfx_state.device_context->RSSetViewports(1, &d3d11_viewport);
}

function void 
gfx_set_scissor(rect_t scissor) {
	D3D11_RECT d3d11_rect = {
		(i32)scissor.x0, (i32)scissor.y0,
		(i32)scissor.x1 , (i32)scissor.y1
	};
	gfx_state.device_context->RSSetScissorRects(1, &d3d11_rect);
}

function void 
gfx_set_depth_mode(gfx_depth_mode depth_mode) {
	ID3D11DepthStencilState* state = nullptr;
	switch (depth_mode) {
		case gfx_depth: { state = gfx_state.depth_stencil_state; break; }
		case gfx_depth_none: { state = gfx_state.no_depth_stencil_state; break; }
	}
	gfx_state.device_context->OMSetDepthStencilState(state, 1);
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
    
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(buffer.data[0]);
    
	switch (resource->buffer_desc.type) {
		case gfx_buffer_type_vertex: {
			u32 offset = 0;
			gfx_state.device_context->IASetVertexBuffers(slot, 1, &resource->buffer.id, &stride, &offset);
			break;
		}
		case gfx_buffer_type_index:	{
			gfx_state.device_context->IASetIndexBuffer(resource->buffer.id, DXGI_FORMAT_R32_UINT, 0);
			break;
		}
		case gfx_buffer_type_constant: {
			gfx_state.device_context->VSSetConstantBuffers(slot, 1, &resource->buffer.id);
			gfx_state.device_context->PSSetConstantBuffers(slot, 1, &resource->buffer.id);
			gfx_state.device_context->CSSetConstantBuffers(slot, 1, &resource->buffer.id);
			break;
		}
	}
    
}

function void
gfx_set_texture(gfx_handle_t texture, u32 slot, gfx_texture_usage texture_usage) {
    
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
	if (!gfx_handle_equals(texture, {0})) {
		switch (texture_usage) {
            
			case gfx_texture_usage_ps: {
				gfx_state.device_context->PSSetShaderResources(slot, 1, &resource->texture.srv);
				break;
			}
            
			case gfx_texture_usage_cs: {
				gfx_state.device_context->CSSetShaderResources(slot, 1, &resource->texture.srv);
				gfx_state.device_context->CSSetUnorderedAccessViews(slot, 1, &resource->texture.uav, nullptr);
				break;
			}
		}
        
	} else {
		ID3D11UnorderedAccessView* null_uav = nullptr;
		ID3D11ShaderResourceView* null_srv = nullptr;
		gfx_state.device_context->PSSetShaderResources(slot, 1, &null_srv);
		gfx_state.device_context->CSSetShaderResources(slot, 1, &null_srv);
		gfx_state.device_context->CSSetUnorderedAccessViews(slot, 1, &null_uav, nullptr);
	}
}

function void
gfx_set_texture_array(gfx_handle_t* textures, u32 texture_count, u32 slot, gfx_texture_usage texture_usage) {
    
	temp_t scratch = scratch_begin();
	
	if (textures != nullptr) {
        
		// make list of srvs
		ID3D11ShaderResourceView** srvs = (ID3D11ShaderResourceView**)arena_alloc(scratch.arena, sizeof(ID3D11ShaderResourceView*) * texture_count);
		for (i32 i = 0; i < texture_count; i++) {
			gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(textures[i].data[0]);
			srvs[i] = resource->texture.srv;
		}
		
		// bind srvs
		switch (texture_usage) {
			case gfx_texture_usage_ps: {
				gfx_state.device_context->PSSetShaderResources(slot, texture_count, srvs);
				break;
			}
			case gfx_texture_usage_cs: {
				gfx_state.device_context->CSSetShaderResources(slot, texture_count, srvs);
                
				// TODO: we might want the uavs
				//gfx_state.device_context->CSSetUnorderedAccessViews(slot, 1, &texture->uav, nullptr);
				break;
			}
		}
        
	} else {
		// make null list of srvs
		ID3D11ShaderResourceView** null_srvs = (ID3D11ShaderResourceView**)arena_calloc(scratch.arena, sizeof(ID3D11ShaderResourceView*) * texture_count);
		
		// bind null srvs
		gfx_state.device_context->PSSetShaderResources(slot, texture_count, null_srvs);
		gfx_state.device_context->CSSetShaderResources(slot, texture_count, null_srvs);
	}
    
	scratch_end(scratch);
    
}

function void 
gfx_set_texture_3d(gfx_handle_t texture, u32 slot, gfx_texture_usage usage) {
    
	//if (texture != nullptr) {
	//	switch (usage) {
	//		case gfx_texture_usage_ps: {
	//			gfx_state.device_context->PSSetShaderResources(slot, 1, &texture->srv);
	//			break;
	//		}
	//								  
	//		case gfx_texture_usage_cs: {
	//			gfx_state.device_context->CSSetShaderResources(slot, 1, &texture->srv);
	//			gfx_state.device_context->CSSetUnorderedAccessViews(slot, 1, &texture->uav, nullptr);
	//			break;
	//		}
	//	}
    
	//} else {
	//	ID3D11UnorderedAccessView* null_uav = nullptr;
	//	ID3D11ShaderResourceView* null_srv = nullptr;
	//	gfx_state.device_context->PSSetShaderResources(slot, 1, &null_srv);
	//	gfx_state.device_context->CSSetShaderResources(slot, 1, &null_srv);
	//	gfx_state.device_context->CSSetUnorderedAccessViews(slot, 1, &null_uav, nullptr);
	//}
    
}

function void
gfx_set_shader(gfx_handle_t shader) {
	if (!gfx_handle_equals(shader, {0})) {
		gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
		gfx_state.device_context->VSSetShader(resource->shader.vertex_shader, 0, 0);
		gfx_state.device_context->PSSetShader(resource->shader.pixel_shader, 0, 0);
		gfx_state.device_context->IASetInputLayout(resource->shader.input_layout);
	} else {
		gfx_state.device_context->VSSetShader(nullptr, 0, 0);
		gfx_state.device_context->PSSetShader(nullptr, 0, 0);
		gfx_state.device_context->IASetInputLayout(nullptr);
	}
}

function void 
gfx_set_compute_shader(gfx_handle_t shader) {
    
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
    
	if (!gfx_handle_equals(shader, { 0 })) {
		gfx_state.device_context->CSSetShader(resource->compute_shader.compute_shader, 0, 0);
	} else {
		gfx_state.device_context->CSSetShader(nullptr, 0, 0);
	}
}

function void
gfx_set_render_target(gfx_handle_t render_target) {
    
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
    
	if (gfx_handle_equals(render_target, { 0 })) {
        
        if (gfx_state.renderer_active != nullptr) {
            gfx_state.device_context->OMSetRenderTargets(1, &gfx_state.renderer_active->framebuffer_rtv, nullptr);
        } else {
            gfx_state.device_context->OMSetRenderTargets(0, nullptr, nullptr);
        }
        
	} else {
		if (resource->render_target.rtv != nullptr) {
			gfx_state.device_context->OMSetRenderTargets(1, &resource->render_target.rtv, resource->render_target.dsv);
		}
	}
}


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


// renderer

function gfx_handle_t
gfx_renderer_create(os_handle_t window, color_t clear_color) {
    
	// get from resource pool or create one
	gfx_d3d11_renderer_t* renderer = gfx_state.renderer_free;
	if (renderer != nullptr) {
		stack_pop(gfx_state.renderer_free);
	} else {
		renderer = (gfx_d3d11_renderer_t*)arena_alloc(gfx_state.renderer_arena, sizeof(gfx_d3d11_renderer_t));
	}
	memset(renderer, 0, sizeof(gfx_d3d11_renderer_t));
	dll_push_back(gfx_state.renderer_first, gfx_state.renderer_last, renderer);
    
	// fill
	renderer->window = window;
	renderer->clear_color = clear_color;
	renderer->resolution = os_window_get_size(window);
    
	// create swapchain
	HRESULT hr = 0;
	DXGI_SWAP_CHAIN_DESC1 swapchain_desc = { 0 };
	swapchain_desc.Width = renderer->resolution.x;
	swapchain_desc.Height = renderer->resolution.y;
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
	hr = gfx_state.dxgi_factory->CreateSwapChainForHwnd(gfx_state.device, w32_window->handle, &swapchain_desc, 0, 0, &renderer->swapchain);
    if (FAILED(hr)) { goto renderer_create_cleanup; }
    
	// get framebuffer from swapchain
	hr = renderer->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&renderer->framebuffer));
    if (FAILED(hr)) { goto renderer_create_cleanup; }
    
	// create render target view
	hr = gfx_state.device->CreateRenderTargetView(renderer->framebuffer, 0, &renderer->framebuffer_rtv);
    if (FAILED(hr)) { goto renderer_create_cleanup; }
    
    renderer_create_cleanup:
    gfx_handle_t handle = { 0 };
    if (!FAILED(hr)) { handle = gfx_d3d11_handle_from_renderer(renderer); }
    
	return handle;
}

function void 
gfx_renderer_release(gfx_handle_t renderer) {
    
	// get renderer
	gfx_d3d11_renderer_t* d3d11_renderer = gfx_d3d11_renderer_from_handle(renderer);
    
	// release d3d11
	if (d3d11_renderer->framebuffer_rtv != nullptr) { d3d11_renderer->framebuffer_rtv->Release(); }
	if (d3d11_renderer->framebuffer != nullptr) { d3d11_renderer->framebuffer->Release(); }
	if (d3d11_renderer->swapchain != nullptr) { d3d11_renderer->swapchain->Release(); }
    
	// push to free stack
	dll_remove(gfx_state.renderer_first, gfx_state.renderer_last, d3d11_renderer);
	stack_push(gfx_state.renderer_free, d3d11_renderer);
    
}

function void
gfx_renderer_resize(gfx_handle_t renderer, uvec2_t resolution) {
    
	// get renderer
	gfx_d3d11_renderer_t* d3d11_renderer = gfx_d3d11_renderer_from_handle(renderer);
    
	// skip is invalid resolution
	if (resolution.x == 0 || resolution.y == 0) {
		return;
	}
    
	gfx_state.device_context->OMSetRenderTargets(0, 0, 0);
	HRESULT hr = 0;
    
	// release buffers
	if (d3d11_renderer->framebuffer_rtv != nullptr) { d3d11_renderer->framebuffer_rtv->Release(); d3d11_renderer->framebuffer_rtv = nullptr; }
	if (d3d11_renderer->framebuffer != nullptr) { d3d11_renderer->framebuffer->Release(); d3d11_renderer->framebuffer = nullptr; }
    
	// resize framebuffer
	hr = d3d11_renderer->swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    //if (FAILED(hr)) { goto renderer_resize_error; }
	hr = d3d11_renderer->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&d3d11_renderer->framebuffer));
    //if (FAILED(hr)) { goto renderer_resize_error; }
	hr = gfx_state.device->CreateRenderTargetView(d3d11_renderer->framebuffer, 0, &d3d11_renderer->framebuffer_rtv);
    //if (FAILED(hr)) { goto renderer_resize_error; }
	
	// set new resolution
	d3d11_renderer->resolution = resolution;
    
}

function void
gfx_renderer_begin(gfx_handle_t renderer) {
    prof_scope("renderer_begin") {
        
        // get renderer
        gfx_d3d11_renderer_t* d3d11_renderer = gfx_d3d11_renderer_from_handle(renderer);
        
        // set render target
        gfx_state.device_context->OMSetBlendState(gfx_state.blend_state, nullptr, 0xffffffff);
        gfx_state.device_context->OMSetRenderTargets(1, &d3d11_renderer->framebuffer_rtv, nullptr);
        
        // clear render target
        FLOAT clear_color_array[] = { d3d11_renderer->clear_color.r, d3d11_renderer->clear_color.g, d3d11_renderer->clear_color.b, d3d11_renderer->clear_color.a };
        gfx_state.device_context->ClearRenderTargetView(d3d11_renderer->framebuffer_rtv, clear_color_array);
        
        // set active renderer
        gfx_state.renderer_active = d3d11_renderer;
    }
}

function void
gfx_renderer_end(gfx_handle_t renderer) {
    prof_scope("renderer_end") {
        
        // get renderer
        gfx_d3d11_renderer_t* d3d11_renderer = gfx_d3d11_renderer_from_handle(renderer);
        
        if (!os_window_is_minimized(d3d11_renderer->window)) {
            d3d11_renderer->swapchain->Present(1, 0);
        } else {
            os_sleep(16);
        }
        gfx_state.device_context->ClearState();
        gfx_state.renderer_active = nullptr;
    }
}

function void
gfx_renderer_blit(gfx_handle_t renderer, gfx_handle_t texture) {
    
	// get renderer
	gfx_d3d11_renderer_t* d3d11_renderer = gfx_d3d11_renderer_from_handle(renderer);
	gfx_d3d11_resource_t* texture_resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
	if (texture_resource->texture_desc.format == gfx_texture_format_rgba8 &&
		texture_resource->texture_desc.size.x == d3d11_renderer->resolution.x &&
		texture_resource->texture_desc.size.y == d3d11_renderer->resolution.y) {
        
		// resolve if higher sample count
		if (texture_resource->texture_desc.sample_count > 1) {
			gfx_state.device_context->ResolveSubresource(d3d11_renderer->framebuffer, 0, texture_resource->texture.id, 0, gfx_d3d11_dxgi_format_from_texture_format(gfx_texture_format_rgba8));
		} else {
			gfx_state.device_context->CopyResource(d3d11_renderer->framebuffer, texture_resource->texture.id);
		}
	}
    
}

function uvec2_t 
gfx_renderer_get_size(gfx_handle_t renderer) {
	gfx_d3d11_renderer_t* d3d11_renderer = gfx_d3d11_renderer_from_handle(renderer);
	return d3d11_renderer->resolution;
}

// buffer
function gfx_handle_t
gfx_buffer_create_ex(gfx_buffer_desc_t desc, void* data) {
    
	// get from resource pool or create one
	gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_buffer);
    
	// fill description
	resource->buffer_desc = desc;
	
	// create buffer
	HRESULT hr = 0;
	D3D11_BUFFER_DESC buffer_desc = { 0 };
	buffer_desc.ByteWidth = desc.size;
	buffer_desc.Usage = gfx_d3d11_d3d11_usage_from_gfx_usage(desc.usage);
	buffer_desc.BindFlags = gfx_d3d11_bind_flags_from_buffer_type(desc.type);
	buffer_desc.CPUAccessFlags = gfx_d3d11_access_flags_from_gfx_usage(desc.usage);
	buffer_desc.MiscFlags = 0;
	
	// initial data
	D3D11_SUBRESOURCE_DATA buffer_data = { 0 };
	if (data != nullptr) {
		buffer_data.pSysMem = data;
	}
    
	hr = gfx_state.device->CreateBuffer(&buffer_desc, data ? &buffer_data : nullptr, &resource->buffer.id);
	gfx_assert(hr, "failed to create buffer.");
    
	gfx_handle_t handle = { (u64)resource };
    
	return handle;
}

function gfx_handle_t
gfx_buffer_create(gfx_buffer_type type, u32 size, void* data){
	return gfx_buffer_create_ex({ type, size, gfx_usage_stream }, data);
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
    
	D3D11_MAPPED_SUBRESOURCE mapped_subresource = { 0 };
	HRESULT hr = gfx_state.device_context->Map(resource->buffer.id, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
	gfx_assert(hr, "failed to map buffer.");
	u8* ptr = (u8*)mapped_subresource.pData;
	memcpy(ptr, data, size);
	gfx_state.device_context->Unmap(resource->buffer.id, 0);
}


// texture
function gfx_handle_t
gfx_texture_create_ex(gfx_texture_desc_t desc, void* data) {
    
	gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_texture);
    HRESULT hr = 0;
	
	// fill description
	resource->texture_desc = desc;
    
    // create texture
	gfx_d3d11_texture_create_resources(resource, data);
    
    gfx_handle_t handle = { 0 };
    if (!FAILED(hr)) { 
        log_infof("successfullt created texture: %.*s", resource->texture_desc.name.size, resource->texture_desc.name.data);
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
	desc.type = gfx_texture_type_2d;
	desc.sample_count = 1;
	desc.usage = gfx_usage_dynamic;
	desc.render_target = false;
    
	// create and return texture
	gfx_handle_t handle = gfx_texture_create_ex(desc, data);
    
	return handle;
}

function gfx_handle_t
gfx_texture_load(str_t filepath) {
	
	// load file
	i32 width = 0;
	i32 height = 0;
	i32 bpp = 0;
    
	stbi_set_flip_vertically_on_load(1);
	unsigned char* buffer = stbi_load((char*)filepath.data, &width, &height, &bpp, 4);
    
    //fill description
    gfx_texture_desc_t desc = { 0 };
	desc.name = str_get_file_name(filepath);
	desc.size = uvec2(width, height);
	desc.format = gfx_texture_format_rgba8;
	desc.type = gfx_texture_type_2d;
	desc.sample_count = 1;
	desc.usage = gfx_usage_dynamic;
	desc.render_target = false;
    
    //create and return texture
    gfx_handle_t handle = gfx_texture_create_ex(desc, buffer);
    
	return handle;
}

function void
gfx_texture_release(gfx_handle_t texture) {
    if (!gfx_handle_equals(texture, { 0 })) {
        
        // get resource
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
        
        // release d3d11
        if (resource->texture.id != nullptr) { resource->texture.id->Release(); resource->texture.id = nullptr;}
        if (resource->texture.srv != nullptr) { resource->texture.srv->Release(); resource->texture.srv = nullptr; }
        if (resource->texture.uav != nullptr) { resource->texture.uav->Release(); resource->texture.uav = nullptr; }
        
        gfx_d3d11_resource_release(resource);
    }
}

function uvec2_t
gfx_texture_get_size(gfx_handle_t texture) {
    uvec2_t result = { 0 };
    
    if (!gfx_handle_equals(texture, { 0 })) {
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
        result = resource->texture_desc.size;
    }
    
    return result;
}

function void 
gfx_texture_resize(gfx_handle_t texture, uvec2_t size) {
    if (size.x > 0 && size.y > 0) {
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
        resource->texture_desc.size = size;
        gfx_d3d11_texture_create_resources(resource, nullptr);
    }
}

function void 
gfx_texture_fill(gfx_handle_t texture, void* data) {
    
	// get resource
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
	D3D11_BOX dst_box = { 0, 0, 0, resource->texture_desc.size.x, resource->texture_desc.size.y, 1 };
	u32 src_row_pitch = resource->texture_desc.size.x * gfx_d3d11_byte_size_from_texture_format(resource->texture_desc.format);
	gfx_state.device_context->UpdateSubresource(resource->texture.id, 0, &dst_box, data, src_row_pitch, 0);
}

function void 
gfx_texture_fill_region(gfx_handle_t texture, rect_t region, void* data) {
    
	// get resource
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
	D3D11_BOX dst_box = {
        (UINT)region.x0, (UINT)region.y0, 0,
        (UINT)region.x1, (UINT)region.y1, 1,
	};
    
	if (dst_box.right > resource->texture_desc.size.x || dst_box.bottom > resource->texture_desc.size.y) {
		log_errorf("gfx_texture_fill_region: incorrect region size!");
        return;
	}
    
	u32 bytes = gfx_d3d11_byte_size_from_texture_format(resource->texture_desc.format);
	u32 src_row_pitch = (region.x1 - region.x0) * bytes;
	gfx_state.device_context->UpdateSubresource(resource->texture.id, 0, &dst_box, data, src_row_pitch, 0);
}

function void 
gfx_texture_blit(gfx_handle_t texture_dst, gfx_handle_t texture_src) {
    
	// get resources
	gfx_d3d11_resource_t* resource_dst = (gfx_d3d11_resource_t*)(texture_dst.data[0]);
	gfx_d3d11_resource_t* resource_src = (gfx_d3d11_resource_t*)(texture_src.data[0]);
    
	if (resource_dst->texture_desc.format == resource_src->texture_desc.format &&
		resource_dst->texture_desc.size.x == resource_src->texture_desc.size.x &&
		resource_dst->texture_desc.size.y == resource_src->texture_desc.size.y &&
		resource_dst->texture_desc.type ==   resource_src->texture_desc.type) {
        
		// resolve if higher sample count
		if (resource_src->texture_desc.sample_count > 1) {
			gfx_state.device_context->ResolveSubresource(resource_dst->texture.id, 0, resource_src->texture.id, 0, gfx_d3d11_dxgi_format_from_texture_format(resource_dst->texture_desc.format));
		} else {
			gfx_state.device_context->CopyResource(resource_dst->texture.id, resource_src->texture.id);
		}
	}
}

function void
gfx_d3d11_texture_create_resources(gfx_d3d11_resource_t* resource, void* data) {
    
    // release old resources if needs
    if (resource->texture.id != nullptr) { resource->texture.id->Release(); resource->texture.id = nullptr;}
    if (resource->texture.srv != nullptr) { resource->texture.srv->Release(); resource->texture.srv = nullptr; }
    if (resource->texture.uav != nullptr) { resource->texture.uav->Release(); resource->texture.uav = nullptr; }
    
    // create texture
    
    HRESULT hr = 0;
    b8 has_uav = true;
    
    // determine bind flags
    D3D11_BIND_FLAG bind_flags = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE);
    
    if (resource->texture_desc.sample_count == 1) {
        bind_flags = (D3D11_BIND_FLAG)(bind_flags | D3D11_BIND_UNORDERED_ACCESS);
    }
    
    if (resource->texture_desc.render_target) {
        if (gfx_texture_format_is_depth(resource->texture_desc.format)) {
            bind_flags = (D3D11_BIND_FLAG)(bind_flags | D3D11_BIND_DEPTH_STENCIL);
            bind_flags = (D3D11_BIND_FLAG)(bind_flags & ~D3D11_BIND_UNORDERED_ACCESS);
            has_uav = false;
        } else {
            bind_flags = (D3D11_BIND_FLAG)(bind_flags | D3D11_BIND_RENDER_TARGET);
        }
    }
    
    // fill out description
    D3D11_TEXTURE2D_DESC texture_desc = { 0 };
    texture_desc.Width = resource->texture_desc.size.x;
    texture_desc.Height = resource->texture_desc.size.y;
    texture_desc.ArraySize = 1;
    texture_desc.Format = gfx_d3d11_dxgi_format_from_texture_format(resource->texture_desc.format);
    texture_desc.SampleDesc.Count = resource->texture_desc.sample_count;
    texture_desc.SampleDesc.Quality = (resource->texture_desc.sample_count > 1) ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
    texture_desc.Usage = gfx_d3d11_d3d11_usage_from_gfx_usage(resource->texture_desc.usage);
    texture_desc.BindFlags = bind_flags;
    texture_desc.CPUAccessFlags = gfx_d3d11_access_flags_from_gfx_usage(resource->texture_desc.usage);
    texture_desc.MipLevels = 1;
    texture_desc.MiscFlags = 0;
    
    // initial data
    D3D11_SUBRESOURCE_DATA texture_data = { 0 };
    if (data != nullptr) {
        texture_data.pSysMem = data;
        texture_data.SysMemPitch = resource->texture_desc.size.x * gfx_d3d11_byte_size_from_texture_format(resource->texture_desc.format);
    }
    
    // create texture2d
    hr = gfx_state.device->CreateTexture2D(&texture_desc, data ? &texture_data : nullptr, &resource->texture.id);
    if (FAILED(hr)) {
        os_graphical_message(true, str("[gfx] texture error"), str("failed to create texture!"));
        os_abort(1);
    }
    
    
    // create srv
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = { 0 };
    srv_desc.Format = gfx_d3d11_srv_format_from_texture_format(resource->texture_desc.format);
    if (resource->texture_desc.sample_count > 1) {
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    } else {
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1; // TODO: support mips.
    }
    hr = gfx_state.device->CreateShaderResourceView(resource->texture.id, &srv_desc, &resource->texture.srv);
    if (FAILED(hr)) {
        os_graphical_message(true, str("[gfx] texture error"), str("failed to create srv for texture!"));
        os_abort(1);
    }
    
    // create uav
    
    if (resource->texture_desc.sample_count == 1 && has_uav) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = { 0 };
        uav_desc.Format = texture_desc.Format;
        uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = 0;
        
        hr = gfx_state.device->CreateUnorderedAccessView(resource->texture.id, &uav_desc, &resource->texture.uav);
        if (FAILED(hr)) {
            os_graphical_message(true, str("[gfx] texture error"), str("failed to create uav for texture!"));
            os_abort(1);
        }
    }
    
    if (FAILED(hr)) {
        log_errorf("gfx_texture_create_resources: texture error occured!");
    }
    
}

function void
gfx_texture_write(gfx_handle_t texture, str_t filepath) {
    
    temp_t scratch = scratch_begin();
    
    gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(texture.data[0]);
    
    // get texture description
    D3D11_TEXTURE2D_DESC desc;
    resource->texture.id->GetDesc(&desc);
    
    // create staging resource description
    D3D11_TEXTURE2D_DESC staging_desc = desc;
    staging_desc.Usage = D3D11_USAGE_STAGING;
    staging_desc.BindFlags = 0;
    staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    staging_desc.MiscFlags = 0;
    
    // create staging texture
    ID3D11Texture2D* staging_texture = nullptr;
    HRESULT hr = gfx_state.device->CreateTexture2D(&staging_desc, nullptr, &staging_texture);
    
    gfx_state.device_context->CopyResource(staging_texture, resource->texture.id);
    
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    hr = gfx_state.device_context->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped_resource);
    
    u32 byte_size = gfx_d3d11_byte_size_from_texture_format(resource->texture_desc.format);
    
    u8* texture_data = (u8*)arena_alloc(scratch.arena, desc.Width * desc.Height * byte_size);
    
    u8* dst = texture_data;
    u8* src = (u8*)mapped_resource.pData;
    
    // copy data
    for (u32 row = 0; row < desc.Height; row++) {
        memcpy(dst, src, desc.Width * byte_size );
        dst += desc.Width * byte_size;
        src += mapped_resource.RowPitch;
    }
    
    // Unmap resource
    gfx_state.device_context->Unmap(staging_texture, 0);
    
    // output to file
    stbi_write_png((char*)filepath.data, desc.Width, desc.Height, 4, texture_data, desc.Width * byte_size);
    
    // Cleanup
    staging_texture->Release();
    
    scratch_end(scratch);
    
}


// 3d texture

//function gfx_texture_3d_t* 
//gfx_texture_3d_create_ex(gfx_texture_3d_desc_t desc, void* data) {
//
//	// get from resource pool or create one.
//	gfx_texture_3d_t* texture = gfx_state.texture_3d_free;
//	if (texture != nullptr) {
//		stack_pop(gfx_state.texture_3d_free);
//	} else {
//		texture = (gfx_texture_3d_t*)arena_alloc(gfx_state.resource_arena, sizeof(gfx_texture_3d_t));
//	}
//	memset(texture, 0, sizeof(gfx_texture_3d_t));
//	dll_push_back(gfx_state.texture_3d_first, gfx_state.texture_3d_last, texture);
//
//	// fill description
//	texture->desc = desc;
//	
//	HRESULT hr = 0;
//
//	// determine bind flags
//	D3D11_BIND_FLAG bind_flags = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
//
//	// fill out description
//	D3D11_TEXTURE3D_DESC texture_desc = { 0 };
//	texture_desc.Width = desc.size.x;
//	texture_desc.Height = desc.size.y;
//	texture_desc.Depth = desc.size.z;
//	texture_desc.MipLevels = 1;
//	texture_desc.Format = gfx_d3d11_dxgi_format_from_texture_format(desc.format);
//	texture_desc.Usage = gfx_d3d11_d3d11_usage_from_gfx_usage(desc.usage);
//	texture_desc.BindFlags = bind_flags;
//	texture_desc.CPUAccessFlags = gfx_d3d11_access_flags_from_gfx_usage(desc.usage);
//	texture_desc.MiscFlags = 0;
//
//	// initial data
//	D3D11_SUBRESOURCE_DATA texture_data = { 0 };
//	if (data != nullptr) {
//		texture_data.pSysMem = data;
//		u32 bytes = gfx_d3d11_byte_size_from_texture_format(desc.format);
//		texture_data.SysMemPitch = texture->desc.size.x * bytes;
//		texture_data.SysMemSlicePitch = texture->desc.size.x * texture->desc.size.y * bytes;
//	}
//
//	// create texture3d
//	hr = gfx_state.device->CreateTexture3D(&texture_desc, data ? &texture_data : nullptr, &texture->id);
//	gfx_assert(hr, "failed to create 3d texture: '%.*s'.", desc.name.size, desc.name.data);
//
//	// create srv
//	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = { 0 };
//	srv_desc.Format = gfx_d3d11_srv_format_from_texture_format(desc.format);
//	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
//	srv_desc.Texture3D.MostDetailedMip = 0;
//	srv_desc.Texture3D.MipLevels = -1;
//	hr = gfx_state.device->CreateShaderResourceView(texture->id, &srv_desc, &texture->srv);
//	gfx_assert(hr, "failed to create shader resource view for 3d texture: '%.*s'.", desc.name.size, desc.name.data);
//
//	// create uav
//	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc = { 0 };
//	uav_desc.Format = texture_desc.Format;
//	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
//	uav_desc.Texture3D.MipSlice = 0;
//	uav_desc.Texture3D.FirstWSlice = 0;
//	uav_desc.Texture3D.WSize = -1;
//
//	hr = gfx_state.device->CreateUnorderedAccessView(texture->id, &uav_desc, &texture->uav);
//	gfx_assert(hr, "failed to create unorderd access view for 3d texture: '%.*s'.", desc.name.size, desc.name.data);
//	
//
//	return texture;
//}
//
//function gfx_texture_3d_t*
//gfx_texture_3d_create(str_t name, uvec3_t size, gfx_texture_format format, void* data) {
//	
//	gfx_texture_3d_desc_t desc = { 0 };
//	desc.name = name;
//	desc.size = size;
//	desc.format = format;
//	desc.usage = gfx_usage_dynamic;
//
//	gfx_texture_3d_t* texture = gfx_texture_3d_create_ex(desc, data);
//
//	return texture;
//}
//
//function void
//gfx_texture_3d_release(gfx_texture_3d_t* texture) {
//
//	// release d3d11
//	if (texture->id != nullptr) { texture->id->Release(); texture->id = nullptr; }
//	if (texture->srv != nullptr) { texture->srv->Release(); texture->srv = nullptr; }
//	if (texture->uav != nullptr) { texture->uav->Release(); texture->uav = nullptr; }
//
//	// push to free stack
//	dll_remove(gfx_state.texture_3d_first, gfx_state.texture_3d_last, texture);
//	stack_push(gfx_state.texture_3d_free, texture);
//
//	texture = nullptr;
//}
//
//




// shaders

function gfx_handle_t
gfx_shader_create_ex(str_t src, gfx_shader_desc_t desc) {
    
	// get from resource pool or create
	gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_shader);
    
	// fill description
	resource->shader_desc= desc;
    
	// create handle
	gfx_handle_t handle = { (u64)resource };
    
	// compile shader
	gfx_shader_compile(handle, src);
	
	return handle;
}

function gfx_handle_t
gfx_shader_create(str_t src, str_t name, gfx_shader_attribute_t* attribute_list, u32 attribute_count) {
    
	// fill description
	gfx_shader_desc_t desc = { 0 };
	desc.name = name;
	desc.filepath = str("");
	desc.attributes = attribute_list;
	desc.attribute_count = attribute_count;
    
	// create and return shader
	gfx_handle_t shader = gfx_shader_create_ex(src, desc);
    
	return shader;
}

function gfx_handle_t
gfx_shader_load(str_t filepath, gfx_shader_attribute_t* attribute_list, u32 attribute_count) {
    
	temp_t scratch = scratch_begin();
    
	// load src from file
	str_t src = os_file_read_all(scratch.arena, filepath);
    
	// fill description
	gfx_shader_desc_t desc = { 0 };
	desc.name = str_get_file_name(filepath);
	desc.filepath = filepath;
	desc.attributes = attribute_list;
	desc.attribute_count = attribute_count;
    
	// create and return shader
	gfx_handle_t shader = gfx_shader_create_ex(src, desc);
    
	scratch_end(scratch);
    
	return shader;
}

function void
gfx_shader_release(gfx_handle_t shader) {
    if (!gfx_handle_equals(shader, { 0 })) {
        
        // get resource
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
        
        // release d3d11
        if (resource->shader.vertex_shader != nullptr) { resource->shader.vertex_shader->Release(); resource->shader.vertex_shader = nullptr; }
        if (resource->shader.pixel_shader != nullptr) { resource->shader.pixel_shader->Release(); resource->shader.pixel_shader = nullptr; }
        if (resource->shader.input_layout != nullptr) { resource->shader.input_layout->Release(); resource->shader.input_layout = nullptr; }
        
        // release resource
        gfx_d3d11_resource_release(resource);
    }
}

function void
gfx_shader_compile(gfx_handle_t shader, str_t src) {
    
	temp_t scratch = scratch_begin();
    
	// get resource
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
    
	HRESULT hr;
	b8 success = false;
	ID3DBlob* vs_blob = nullptr;
	ID3DBlob* ps_blob = nullptr;
	ID3DBlob* vs_error_blob = nullptr;
	ID3DBlob* ps_error_blob = nullptr;
	ID3D11VertexShader* vertex_shader = nullptr;
	ID3D11PixelShader* pixel_shader = nullptr;
    
	u32 compile_flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
    
#if defined(BUILD_DEBUG)
	compile_flags |= D3DCOMPILE_DEBUG;
#endif 
    
	if (src.size == 0) {
		goto shader_build_cleanup;
	}
    
	// compile vertex shader
	hr = D3DCompile(src.data, src.size, (char*)resource->shader_desc.name.data, 0, 0, "vs_main", "vs_5_0", compile_flags, 0, &vs_blob, &vs_error_blob);
	if (vs_error_blob) {
		cstr error_msg = (cstr)vs_error_blob->GetBufferPointer();
		log_error("gfx_shader_compile: failed to compile vertex shader: \n\n%s\n", error_msg);
        goto shader_build_cleanup;
	}
	hr = gfx_state.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &vertex_shader);
    
	// compile pixel shader
	hr = D3DCompile(src.data, src.size, (char*)resource->shader_desc.name.data, 0, 0, "ps_main", "ps_5_0", compile_flags, 0, &ps_blob, &ps_error_blob);
	if (ps_error_blob) {
		cstr error_msg = (cstr)ps_error_blob->GetBufferPointer();
		log_error("gfx_shader_compile: failed to compile pixel shader: \n\n%s\n", error_msg);
		goto shader_build_cleanup;
	}
	hr = gfx_state.device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &pixel_shader);
    
	// create input layout if needed
	if (resource->shader.input_layout == nullptr) {
        
		gfx_shader_attribute_t* attributes = resource->shader_desc.attributes;
        
		// allocate input element array
		D3D11_INPUT_ELEMENT_DESC* input_element_desc = (D3D11_INPUT_ELEMENT_DESC*)arena_alloc(scratch.arena, sizeof(D3D11_INPUT_ELEMENT_DESC) * resource->shader_desc.attribute_count);
        
		for (i32 i = 0; i < resource->shader_desc.attribute_count; i++) {
			input_element_desc[i] = {
				(char*)attributes[i].name,
				attributes[i].slot,
				gfx_d3d11_dxgi_format_from_vertex_format(attributes[i].format),
				0, D3D11_APPEND_ALIGNED_ELEMENT,
				gfx_d3d11_input_class_from_vertex_class(attributes[i].classification),
				(attributes[i].classification == gfx_vertex_class_per_vertex) ? (u32)0 : (u32)1
			};
		}
        
		hr = gfx_state.device->CreateInputLayout(input_element_desc, resource->shader_desc.attribute_count, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &resource->shader.input_layout);
		if (FAILED(hr)) {
            log_errorf("gfx_shader_compile: faild to create shader input layout!");
			goto shader_build_cleanup;
		}
	}
	
	if (hr == S_OK) {
        
		// release old shaders if needed
		if (resource->shader.vertex_shader != nullptr) { resource->shader.vertex_shader->Release(); }
		if (resource->shader.pixel_shader != nullptr) { resource->shader.pixel_shader->Release(); }
        
		// set new shaders
		resource->shader.vertex_shader = vertex_shader;
		resource->shader.pixel_shader = pixel_shader;
        
		success = true;
        
        log_infof("successfully created shader: '%.*s'",  resource->shader_desc.name.size, resource->shader_desc.name.data);
	}
    
    shader_build_cleanup:
    
	if (vs_blob != nullptr) { vs_blob->Release(); }
	if (ps_blob != nullptr) { ps_blob->Release(); }
	if (vs_error_blob != nullptr) { vs_error_blob->Release(); }
	if (ps_error_blob != nullptr) { ps_error_blob->Release(); }
    
	if (!success) {
		if (vertex_shader != nullptr) { vertex_shader->Release(); }
		if (pixel_shader != nullptr) { pixel_shader->Release(); }
		if (resource->shader.input_layout != nullptr) { resource->shader.input_layout->Release(); }
	}
    
	scratch_end(scratch);
    
}


// compute shader

function gfx_handle_t
gfx_compute_shader_create_ex(str_t src, gfx_compute_shader_desc_t desc) {
    
	// get from resource pool or create
	gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_compute_shader);
    
	// fill description
	resource->compute_shader_desc = desc;
    
	// create handle
	gfx_handle_t handle = { (u64)resource };
    
	// compile shader
	gfx_compute_shader_compile(handle, src);
    
	return handle;
}

function gfx_handle_t
gfx_compute_shader_create(str_t src, str_t name) {
    
	// fill description
	gfx_compute_shader_desc_t desc = { 0 };
	desc.name = name;
	desc.filepath = str("");
    
	// create and return shader
	gfx_handle_t shader = gfx_compute_shader_create_ex(src, desc);
    
	return shader;
}


function gfx_handle_t
gfx_compute_shader_load(str_t filepath) {
    
	temp_t scratch = scratch_begin();
    
	// load src from file
	str_t src = os_file_read_all(scratch.arena, filepath);
    
	// get name
	str_t name = str_get_file_name(filepath);
    
	// fill description
	gfx_compute_shader_desc_t desc = { 0 };
	desc.name = name;
	desc.filepath = filepath;
    
	// create and return shader
	gfx_handle_t shader = gfx_compute_shader_create_ex(src, desc);
    
	scratch_end(scratch);
    
	return shader;
}

function void
gfx_compute_shader_release(gfx_handle_t shader) {
    if (!gfx_handle_equals(shader, { 0 })) {
        
        gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
        
        // release d3d11
        if (resource->compute_shader.compute_shader != nullptr) { resource->compute_shader.compute_shader->Release(); resource->compute_shader.compute_shader = nullptr; }
        
        // release resource
        gfx_d3d11_resource_release(resource);
        
    }
}

function void
gfx_compute_shader_compile(gfx_handle_t shader, str_t src) {
    
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(shader.data[0]);
    
	HRESULT hr;
	b8 success = false;
	ID3DBlob* cs_blob = nullptr;
	ID3DBlob* cs_error_blob = nullptr;
	ID3D11ComputeShader* compute_shader = nullptr;
    
	u32 compile_flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
    
#if defined(BUILD_DEBUG)
	compile_flags |= D3DCOMPILE_DEBUG;
#endif 
    
	if (src.size == 0) {
		goto shader_build_cleanup;
	}
    
	// compile vertex shader
	hr = D3DCompile(src.data, src.size, (char*)resource->compute_shader_desc.name.data, 0, 0, "cs_main", "cs_5_0", compile_flags, 0, &cs_blob, &cs_error_blob);
    
	if (cs_error_blob) {
		cstr error_msg = (cstr)cs_error_blob->GetBufferPointer();
		log_error("gfx_compute_shader_compile: failed to compile compute shader: \n\n%s\n", error_msg);
        goto shader_build_cleanup;
	}
	hr = gfx_state.device->CreateComputeShader(cs_blob->GetBufferPointer(), cs_blob->GetBufferSize(), nullptr, &compute_shader);
    
	if (hr == S_OK) {
        
		// release old shader if needed
		if (resource->compute_shader.compute_shader != nullptr) { resource->compute_shader.compute_shader->Release(); }
        
		// set new shaders
		resource->compute_shader.compute_shader = compute_shader;
        
		success = true;
        
        log_infof("succesffully create compute shader: %.*s", resource->compute_shader_desc.name.size, resource->compute_shader_desc.name.data);
	}
    
    shader_build_cleanup:
    
	if (cs_blob != nullptr) { cs_blob->Release(); }
	if (cs_error_blob != nullptr) { cs_error_blob->Release(); }
    
	if (!success) {
		if (compute_shader != nullptr) { compute_shader->Release(); }
	}
    
}




// render target

function gfx_handle_t 
gfx_render_target_create_ex(gfx_render_target_desc_t desc) {
    
	// get a resource and fill desc
	gfx_d3d11_resource_t* resource = gfx_d3d11_resource_create(gfx_resource_type_render_target);
	resource->render_target_desc = desc;
	
	// create handle
	gfx_handle_t handle = { (u64)resource };
    
	// internal allocate resources
	gfx_render_target_create_resources(handle);
    
	return handle;
    
}

function gfx_handle_t 
gfx_render_target_create(gfx_texture_format format, uvec2_t size, gfx_render_target_flags flags) {
    
	gfx_render_target_desc_t desc = { 0 };
	desc.format = format;
	desc.size = size;
	desc.sample_count = 1;
	desc.flags = flags;
    
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
		resource->render_target_desc.size = size;
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
		gfx_state.device_context->ClearRenderTargetView(resource->render_target.rtv, clear_color_array);
	}
    
	// clear depthbuffer
	if (resource->render_target.dsv != nullptr) {
		gfx_state.device_context->ClearDepthStencilView(resource->render_target.dsv, D3D11_CLEAR_DEPTH, depth, 0);
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
    
	// create new texture
	gfx_texture_desc_t color_texture_desc = { 0 };
	color_texture_desc.name = str("");
	color_texture_desc.size = resource->render_target_desc.size;
	color_texture_desc.format = resource->render_target_desc.format;
	color_texture_desc.type = gfx_texture_type_2d;
	color_texture_desc.sample_count = resource->render_target_desc.sample_count;
	color_texture_desc.usage = gfx_usage_dynamic;
	color_texture_desc.render_target = true;
    
	resource->render_target.color_texture = gfx_texture_create_ex(color_texture_desc);
	
	// create rtv
	HRESULT hr = 0;
    
	// release old rtv if needed
	if (resource->render_target.rtv != nullptr) {
		resource->render_target.rtv->Release();
	}
    
	// create new rtv
	D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = { 0 };
	rtv_desc.Format = gfx_d3d11_dxgi_format_from_texture_format(resource->render_target_desc.format);
	if (resource->render_target_desc.sample_count > 1) {
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	} else {
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Texture2D.MipSlice = 0;
	}
    
	// get texture resource
	gfx_d3d11_resource_t* color_texture_resource = (gfx_d3d11_resource_t*)(resource->render_target.color_texture.data[0]);
	hr = gfx_state.device->CreateRenderTargetView(color_texture_resource->texture.id, &rtv_desc, &resource->render_target.rtv);
	gfx_check_error(hr, "failed to create the color rtv for render target.");
    
	// if has depth target
	if (resource->render_target_desc.flags & gfx_render_target_flag_depth) {
        
		// release current if exist
		if (!gfx_handle_equals(resource->render_target.depth_texture, {0})) {
			gfx_texture_release(resource->render_target.depth_texture);
		}
        
		// create new texture
		gfx_texture_desc_t depth_texture_desc = { 0 };
		depth_texture_desc.name = str("");
		depth_texture_desc.size = resource->render_target_desc.size;
		depth_texture_desc.format = gfx_texture_format_d32;
		depth_texture_desc.type = gfx_texture_type_2d;
		depth_texture_desc.sample_count = resource->render_target_desc.sample_count;
		depth_texture_desc.usage = gfx_usage_dynamic;
		depth_texture_desc.render_target = true;
        
		resource->render_target.depth_texture = gfx_texture_create_ex(depth_texture_desc);
        
		// release old dsv if needed
		if (resource->render_target.dsv != nullptr) {
			resource->render_target.dsv->Release();
		}
        
		// create new dsv
		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc = { 0 };
		dsv_desc.Format = gfx_d3d11_dsv_format_from_texture_format(gfx_texture_format_d32);
		if (resource->render_target_desc.sample_count > 1) {
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		} else {
			dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsv_desc.Texture2D.MipSlice = 0;
		}
        
		gfx_d3d11_resource_t* depth_texture_resource = (gfx_d3d11_resource_t*)(resource->render_target.depth_texture.data[0]);
		hr = gfx_state.device->CreateDepthStencilView(depth_texture_resource->texture.id, &dsv_desc, &resource->render_target.dsv);
		gfx_check_error(hr, "failed to create the depth dsv for render target.");
        
	} 
    
}

function uvec2_t 
gfx_render_target_get_size(gfx_handle_t render_target) {
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
	return resource->render_target_desc.size;
}

function gfx_handle_t
gfx_render_target_get_texture(gfx_handle_t render_target) {
	gfx_d3d11_resource_t* resource = (gfx_d3d11_resource_t*)(render_target.data[0]);
	return resource->render_target.color_texture;
}

// d3d11 specific function

function gfx_d3d11_renderer_t* 
gfx_d3d11_renderer_from_handle(gfx_handle_t renderer) {
	gfx_d3d11_renderer_t* d3d11_renderer = (gfx_d3d11_renderer_t*)(renderer.data[0]);
	return d3d11_renderer;
}

function gfx_handle_t 
gfx_d3d11_handle_from_renderer(gfx_d3d11_renderer_t* renderer) {
	gfx_handle_t handle = { (u64)renderer };
	return handle;
}

// resource functions

function gfx_d3d11_resource_t* 
gfx_d3d11_resource_create(gfx_resource_type type) {
    
	// grab from free list of allocate one
	gfx_d3d11_resource_t* resource = gfx_state.resource_free;
	if (resource != nullptr) {
		stack_pop(gfx_state.resource_free);
	} else {
		resource = (gfx_d3d11_resource_t*)arena_alloc(gfx_state.resource_arena, sizeof(gfx_d3d11_resource_t));
	}
	memset(resource, 0, sizeof(gfx_d3d11_resource_t));
	dll_push_back(gfx_state.resource_first, gfx_state.resource_last, resource);
    
	// set type
	resource->type = type;
    
	return resource;
}

function void 
gfx_d3d11_resource_release(gfx_d3d11_resource_t* resource) {
    dll_remove(gfx_state.resource_first, gfx_state.resource_last, resource);
    stack_push(gfx_state.resource_free, resource);
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

function DXGI_FORMAT 
gfx_d3d11_dxgi_format_from_uniform_type(gfx_uniform_type type) {
	DXGI_FORMAT result = DXGI_FORMAT_UNKNOWN;
	switch (type) {
		case gfx_uniform_type_float: { result = DXGI_FORMAT_R32_FLOAT; break; }
		case gfx_uniform_type_float2: { result = DXGI_FORMAT_R32G32_FLOAT; break; }
		case gfx_uniform_type_float3: { result = DXGI_FORMAT_R32G32B32_FLOAT; break; }
		case gfx_uniform_type_float4: { result = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
		case gfx_uniform_type_int: { result = DXGI_FORMAT_R32_SINT; break; }
		case gfx_uniform_type_int2: { result = DXGI_FORMAT_R32G32_SINT; break; }
		case gfx_uniform_type_int3: { result = DXGI_FORMAT_R32G32B32_SINT; break; }
		case gfx_uniform_type_int4: { result = DXGI_FORMAT_R32G32B32A32_SINT; break; }
		case gfx_uniform_type_uint: { result = DXGI_FORMAT_R32_UINT; break; }
		case gfx_uniform_type_uint2: { result = DXGI_FORMAT_R32G32_UINT; break; }
		case gfx_uniform_type_uint3: { result = DXGI_FORMAT_R32G32B32_UINT; break; }
		case gfx_uniform_type_uint4: { result = DXGI_FORMAT_R32G32B32A32_UINT; break; }
	}
	return result;
}

function D3D11_INPUT_CLASSIFICATION 
gfx_d3d11_input_class_from_vertex_class(gfx_vertex_class vertex_class) {
	D3D11_INPUT_CLASSIFICATION shader_classification = (D3D11_INPUT_CLASSIFICATION)0;
	switch (vertex_class) {
		case gfx_vertex_class_per_vertex: { shader_classification = D3D11_INPUT_PER_VERTEX_DATA; break; }
		case gfx_vertex_class_per_instance: { shader_classification = D3D11_INPUT_PER_INSTANCE_DATA; break; }
	}
	return shader_classification;
}


#endif // SORA_GFX_D3D11_CPP