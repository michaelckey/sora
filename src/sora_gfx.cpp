// sora_gfx.cpp

#ifndef SORA_GFX_CPP
#define SORA_GFX_CPP

// implementation

// handle
function b8 
gfx_handle_equals(gfx_handle_t a, gfx_handle_t b) {
	return (a.data[0] == b.data[0]);
}

// pipeline

function gfx_pipeline_t 
gfx_pipeline_create() {
	gfx_pipeline_t result = { 0 };
	result.fill_mode = gfx_fill_solid;
	result.cull_mode = gfx_cull_mode_back;
	result.topology = gfx_topology_tris;
	result.filter_mode = gfx_filter_linear;
	result.wrap_mode = gfx_wrap_clamp;
	result.depth_mode = gfx_depth;
	result.viewport = rect(0.0f, 0.0f, 0.0f, 0.0f);
	result.scissor = rect(0.0f, 0.0f, 0.0f, 0.0f);
	return result;
}

function b8
gfx_pipeline_equals(gfx_pipeline_t a, gfx_pipeline_t b) {
	b8 result = (
                 a.fill_mode == b.fill_mode &&
                 a.cull_mode == b.cull_mode &&
                 a.topology == b.topology &&
                 a.filter_mode == b.filter_mode &&
                 a.wrap_mode == b.wrap_mode &&
                 a.depth_mode == b.depth_mode &&
                 rect_equals(a.viewport, b.viewport) &&
                 rect_equals(a.scissor, b.scissor)
                 );
    
	return result;
}

// helper functions
function b8
gfx_texture_format_is_depth(gfx_texture_format format) {
	b8 result = false;
	switch (format) {
		case gfx_texture_format_d24s8:
		case gfx_texture_format_d32: {
			result = true;
			break;
		}
	}
	return result;
}

// per backend includes

#ifdef GFX_BACKEND_D3D11
#    include "backends/gfx/sora_gfx_d3d11.cpp"
#elif GFX_BACKEND_D3D12
#    include "backends/gfx/sora_gfx_d3d12.cpp"
#elif GFX_BACKEND_OPENGL
#    include "backends/gfx/sora_gfx_opengl.cpp"
#elif GFX_BACKEND_METAL
#    include "backends/gfx/sora_gfx_metal.cpp"
#elif GFX_BACKEND_VULKAN
#    include "backends/gfx/sora_gfx_vulkan.cpp"
#endif

#endif // SORA_GFX_CPP