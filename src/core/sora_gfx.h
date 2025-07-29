// sora_gfx.h

#ifndef SORA_GFX_H
#define SORA_GFX_H

//~ typedefs 


//~ enums

typedef u32 gfx_context_flags;
enum {
    gfx_context_flag_none = 0,
    gfx_context_flag_multisample = (1 << 2),
    gfx_context_flag_depthbuffer = (1 << 3),
};

enum gfx_usage {
	gfx_usage_null,
	gfx_usage_static,
	gfx_usage_dynamic,
	gfx_usage_stream,
};

// buffer

enum gfx_buffer_type {
	gfx_buffer_type_null,
	gfx_buffer_type_vertex,
	gfx_buffer_type_index,
	gfx_buffer_type_constant,
    gfx_buffer_type_structured,
    gfx_buffer_type_indirect,
};

// texture

enum gfx_texture_type {
	gfx_texture_type_null,
	gfx_texture_type_1d,
	gfx_texture_type_2d,
	gfx_texture_type_3d,
	gfx_texture_type_cube,
};

enum gfx_texture_format {
	gfx_texture_format_null,
	gfx_texture_format_r8,
	gfx_texture_format_rg8,
	gfx_texture_format_rgba8,
	gfx_texture_format_bgra8,
	gfx_texture_format_r16,
	gfx_texture_format_rgba16,
	gfx_texture_format_r32,
	gfx_texture_format_rg32,
	gfx_texture_format_rgba32,
	gfx_texture_format_d24s8,
	gfx_texture_format_d32,
};

typedef u32 gfx_texture_flags;
enum {
    gfx_texture_flag_none = 0,
    gfx_texture_flag_render_target = (1 << 0),
    gfx_texture_flag_mipmap = (1 << 1),
};

// shader

typedef u32 gfx_shader_flags;
enum {
    
    // shader type
    gfx_shader_flag_vertex = (1 << 0),
    gfx_shader_flag_pixel = (1 << 1),
    gfx_shader_flag_geometry = (1 << 2),
    gfx_shader_flag_hull = (1 << 3),
    gfx_shader_flag_domain = (1 << 4),
    gfx_shader_flag_compute = (1 << 5),
    
    // vertex classification
    gfx_shader_flag_per_vertex = (1 << 10),
    gfx_shader_flag_per_instance = (1 << 11),
};

enum gfx_vertex_format {
	gfx_vertex_format_null,
	gfx_vertex_format_float,
	gfx_vertex_format_float2,
	gfx_vertex_format_float3,
	gfx_vertex_format_float4,
	gfx_vertex_format_uint,
	gfx_vertex_format_uint2,
	gfx_vertex_format_uint3,
	gfx_vertex_format_uint4,
	gfx_vertex_format_int,
	gfx_vertex_format_int2,
	gfx_vertex_format_int3,
	gfx_vertex_format_int4,
};

//~ structs 

// handles
struct gfx_context_t { u64 id; };
struct gfx_buffer_t { u64 id; };
struct gfx_texture_t { u64 id; };
struct gfx_shader_t { u64 id; };
struct gfx_render_target_t { u64 id; };

// buffer
struct gfx_buffer_desc_t {
    gfx_buffer_type type;
    gfx_usage usage;
    u32 size;
    u32 stride;
    void* initial_data;
};

// texture
struct gfx_texture_desc_t {
	gfx_texture_type type;
	gfx_texture_format format;
	gfx_texture_flags flags;
	gfx_usage usage;
    uvec3_t size;
	u32 sample_count;
    u32 mip_levels;
    void* initial_data;
};

// shader
struct gfx_shader_desc_t {
	str_t name;
    str_t entry_point;
	gfx_shader_flags flags;
};

// render target
struct gfx_render_target_desc_t {
	gfx_texture_format colorbuffer_format;
	gfx_texture_format depthbuffer_format;
	uvec2_t size;
	u32 sample_count;
};

//~ functions 

//- handles (implemented once)

function b8                  gfx_context_equals(gfx_context_t a, gfx_context_t b);
function b8                  gfx_buffer_equals(gfx_buffer_t a, gfx_buffer_t b);
function b8                  gfx_texture_equals(gfx_texture_t a, gfx_texture_t b);
function b8                  gfx_shader_equals(gfx_shader_t a, gfx_shader_t b);
function b8                  gfx_render_target_equals(gfx_render_target_t a, gfx_render_target_t b);

//- helper loaders (implemented once)

function gfx_texture_t       gfx_texture_load(str_t filepath, gfx_texture_flags flags = gfx_texture_flag_none);
function gfx_shader_t        gfx_shader_load(str_t filepath, gfx_shader_flags flags);

//- state (implemented per backend)

function void                gfx_init();
function void                gfx_release();

//- commands (implemented per backend)

function void                gfx_draw(u32 vertex_count, u32 start_index = 0);
function void                gfx_draw_indexed(u32 index_count, u32 start_index = 0, u32 offset = 0);
function void                gfx_draw_instanced(u32 vertex_count, u32 instance_count, u32 start_vertex_index = 0, u32 start_instance_index = 0);

function void                gfx_dispatch(u32 thread_group_x, u32 thread_group_y, u32 thread_group_z);

//- context (implemented per backend) 

function gfx_context_t       gfx_context_create(os_window_t window, gfx_context_flags flags = gfx_context_flag_none);
function void                gfx_context_release(gfx_context_t context_handle);
function void                gfx_context_set(gfx_context_t context_handle);
function void                gfx_context_clear(gfx_context_t context_handle, color_t clear_color, f32 depth_clear = 0.0f);
function void                gfx_context_present(gfx_context_t context_handle, b8 vsync = true);

function uvec2_t             gfx_context_get_size(gfx_context_t context_handle);
function void                gfx_context_resize(gfx_context_t context_handle, uvec2_t size);

//- buffers (implemented per backend)

function gfx_buffer_t        gfx_buffer_create_ex(gfx_buffer_desc_t desc);
function gfx_buffer_t        gfx_buffer_create(gfx_buffer_type type, u32 size, const void* initial_data = nullptr);
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

//- textures (implemented per backend)

function gfx_texture_t       gfx_texture_create_ex(gfx_texture_desc_t texture_desc);
function gfx_texture_t       gfx_texture_create_1d(u32 size, gfx_texture_format format = gfx_texture_format_rgba8, void* initial_data = nullptr);
function gfx_texture_t       gfx_texture_create_2d(uvec2_t size, gfx_texture_format format = gfx_texture_format_rgba8, void* initial_data = nullptr);
function gfx_texture_t       gfx_texture_create_3d(uvec3_t size, gfx_texture_format format = gfx_texture_format_rgba8, void* initial_data = nullptr);
function void                gfx_texture_release(gfx_texture_t texture_handle);
function void                gfx_texture_update(gfx_texture_t texture_handle, u32 offset, u32 size, void* data);
function void                gfx_texture_update_region_1d(gfx_texture_t texture_handle, void* data); // TODO: 
function void                gfx_texture_update_region_2d(gfx_texture_t texture_handle, void* data); // TODO:
function void                gfx_texture_update_region_3d(gfx_texture_t texture_handle, void* data); // TODO:
function void                gfx_texture_blit(gfx_texture_t texture_handle_src, gfx_texture_t texture_handle_dst);

function void                gfx_texture_set(gfx_texture_t texture_handle, u32 slot = 0);
function void                gfx_texture_set_array(gfx_texture_t* texture_handles, u32 texture_count, u32 slot_start = 0);

function gfx_texture_type    gfx_texture_get_type(gfx_texture_t texture_handle);
function gfx_texture_format  gfx_texture_get_format(gfx_texture_t texture_handle);
function gfx_texture_flags   gfx_texture_get_flags(gfx_texture_t texture_handle);
function gfx_usage           gfx_texture_get_usage(gfx_texture_t texture_handle); 
function u32                 gfx_texture_get_size_1d(gfx_texture_t texture_handle);
function uvec2_t             gfx_texture_get_size_2d(gfx_texture_t texture_handle);
function uvec3_t             gfx_texture_get_size_3d(gfx_texture_t texture_handle);
function u32                 gfx_texture_get_sample_count(gfx_texture_t texture_handle);
function u32                 gfx_texture_get_mip_levels(gfx_texture_t texture_handle);

//- shaders (implemented per backend)

function gfx_shader_t        gfx_shader_create_ex(gfx_shader_desc_t desc);
function gfx_shader_t        gfx_shader_create(str_t name, gfx_shader_flags flags);
function void                gfx_shader_release(gfx_shader_t shader_handle);
function b8                  gfx_shader_compile(gfx_shader_t shader_handle, str_t source);
function void                gfx_shader_set_binary(gfx_shader_t shader_handle, void* binary, u32 binary_size);
function void                gfx_shader_get_binary(gfx_shader_t shader_handle, void** binary_out, u32* binary_size_out);

function void                gfx_shader_set(gfx_shader_t shader_handle);

function str_t               gfx_shader_get_name(gfx_shader_t shader_handle);
function gfx_shader_flags    gfx_shader_get_flags(gfx_shader_t shader_handle);

//- render targets (implemented per backend) 

function gfx_render_target_t gfx_render_target_create_ex(gfx_render_target_desc_t desc);
function gfx_render_target_t gfx_render_target_create(uvec2_t size, gfx_texture_format colorbuffer_format, gfx_texture_format depthbuffer_format = gfx_texture_format_null);
function void                gfx_render_target_release(gfx_render_target_t render_target_handle);
function void                gfx_render_target_resize(gfx_render_target_t render_target_handle, uvec2_t size);
function void                gfx_render_target_clear(gfx_render_target_t render_target_handle, color_t clear_color, f32 depth = 1.0f);

function void                gfx_render_target_set(gfx_render_target_t render_target_handle);

function gfx_texture_format  gfx_render_target_get_colorbuffer_format(gfx_render_target_t render_target_handle);
function gfx_texture_format  gfx_render_target_get_depthbuffer_format(gfx_render_target_t render_target_handle);
function uvec2_t             gfx_render_target_get_size(gfx_render_target_t render_target_handle);
function u32                 gfx_render_target_get_sample_count(gfx_render_target_t render_target_handle);

function gfx_texture_t       gfx_render_target_get_colorbuffer(gfx_render_target_t render_target_handle);
function gfx_texture_t       gfx_render_target_get_depthbuffer(gfx_render_target_t render_target_handle);

//~ backend includes

#if OS_BACKEND_WIN32
#    if !defined(GFX_BACKEND_D3D11) && !defined(GFX_BACKEND_D3D12) && !defined(GFX_BACKEND_OPENGL) && !defined(GFX_BACKEND_VULKAN)
#        define GFX_BACKEND_D3D11 1
#    endif
#elif OS_BACKEND_MACOS
#    if !defined(GFX_BACKEND_METAL) && !defined(GFX_BACKEND_OPENGL) && !defined(GFX_BACKEND_VULKAN)
#        define GFX_BACKEND_METAL 1
#    endif
#elif OS_BACKEND_LINUX
#    if !defined(GFX_BACKEND_OPENGL) && !defined(GFX_BACKEND_VULKAN)
#        define GFX_BACKEND_OPENGL 1
#    endif
#endif 

#if GFX_BACKEND_D3D11
#    include "backends/gfx/sora_gfx_d3d11.h"
#elif GFX_BACKEND_D3D12
#     error gfx backend not implemented.
#elif GFX_BACKEND_OPENGL
#    error gfx backend not implemented.
#elif GFX_BACKEND_METAL
#    error gfx backend not implemented.
#elif GFX_BACKEND_VULKAN
#    error gfx backend not implemented.
#else
#    error unknown gfx backend.
#endif


#endif // SORA_GFX_H