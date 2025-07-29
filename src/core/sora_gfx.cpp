// sora_gfx.cpp

#ifndef SORA_GFX_CPP
#define SORA_GFX_CPP

//~ implementation

//- handle functions 

function b8
gfx_context_equals(gfx_context_t a, gfx_context_t b) {
    return (a.id == b.id);
}

function b8
gfx_buffer_equals(gfx_buffer_t a, gfx_buffer_t b) {
    return (a.id == b.id);
}

function b8
gfx_texture_equals(gfx_texture_t a, gfx_texture_t b) {
    return (a.id == b.id);
}

function b8
gfx_shader_equals(gfx_shader_t a, gfx_shader_t b) {
    return (a.id == b.id);
}

function b8
gfx_render_target_equals(gfx_render_target_t a, gfx_render_target_t b) {
    return (a.id == b.id);
}

//- helper loader functions 

function gfx_shader_t 
gfx_shader_load(str_t filepath, gfx_shader_flags flags) {
    
    
    
}


//~ backend includes 

#if GFX_BACKEND_D3D11
#    include "backends/gfx/sora_gfx_d3d11.cpp"
#elif GFX_BACKEND_D3D12

#elif GFX_BACKEND_OPENGL

#elif GFX_BACKEND_METAL

#elif GFX_BACKEND_VULKAN

#else

#endif

#endif // SORA_GFX_CPP