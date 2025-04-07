// sora_gfx.h

#ifndef SORA_GFX_H
#define SORA_GFX_H

//- includes

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendor/stb_image_write.h"

//- enums

enum gfx_resource_type {
	gfx_resource_type_null,
	gfx_resource_type_buffer,
	gfx_resource_type_texture,
	gfx_resource_type_shader,
	gfx_resource_type_compute_shader,
	gfx_resource_type_render_target,
	gfx_resource_type_count,
};

enum gfx_usage {
	gfx_usage_null,
	gfx_usage_static,
	gfx_usage_dynamic,
	gfx_usage_stream,
};

enum gfx_buffer_type {
	gfx_buffer_type_null,
	gfx_buffer_type_vertex,
	gfx_buffer_type_index,
	gfx_buffer_type_constant,
};

enum gfx_texture_usage {
	gfx_texture_usage_null,
	gfx_texture_usage_ps,
	gfx_texture_usage_cs,
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

enum gfx_texture_type {
	gfx_texture_type_null,
	gfx_texture_type_2d,
	gfx_texture_type_3d,
};

enum gfx_filter_mode {
	gfx_filter_null,
	gfx_filter_linear,
	gfx_filter_nearest,
};

enum gfx_wrap_mode {
	gfx_wrap_null,
	gfx_wrap_repeat,
	gfx_wrap_clamp,
};

enum gfx_fill_mode {
	gfx_fill_null,
	gfx_fill_solid,
	gfx_fill_wireframe,
};

enum gfx_depth_mode {
	gfx_depth_null,
	gfx_depth,
	gfx_depth_none,
};

enum gfx_topology_type {
	gfx_topology_null,
	gfx_topology_points,
	gfx_topology_lines,
	gfx_topology_line_strip,
	gfx_topology_tris,
	gfx_topology_tri_strip,
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

enum gfx_vertex_class {
	gfx_vertex_class_null,
	gfx_vertex_class_per_vertex,
	gfx_vertex_class_per_instance,
};

enum gfx_uniform_type {
	gfx_uniform_type_null,
	gfx_uniform_type_float,
	gfx_uniform_type_float2,
	gfx_uniform_type_float3,
	gfx_uniform_type_float4,
	gfx_uniform_type_uint,
	gfx_uniform_type_uint2,
	gfx_uniform_type_uint3,
	gfx_uniform_type_uint4,
	gfx_uniform_type_int,
	gfx_uniform_type_int2,
	gfx_uniform_type_int3,
	gfx_uniform_type_int4,
	gfx_uniform_type_mat4,
};

enum gfx_cull_mode {
	gfx_cull_mode_null,
	gfx_cull_mode_none,
	gfx_cull_mode_front,
	gfx_cull_mode_back,
};

// NOTE: not sure if this will be used.
enum gfx_face_winding {
	gfx_face_winding_null,
	gfx_face_winding_ccw,
	gfx_face_winding_cw,
};

typedef u32 gfx_render_target_flags;
enum {
	gfx_render_target_flag_null = 0,
	gfx_render_target_flag_depth = (1 << 0),
};

//- structs

struct gfx_handle_t {
	u64 data[1];
};

struct gfx_buffer_desc_t {
	gfx_buffer_type type;
	u32 size;
	gfx_usage usage;
};

struct gfx_texture_desc_t {
	str_t name;
	uvec2_t size;
	gfx_texture_format format;
	gfx_texture_type type;
	u32 sample_count;
	gfx_usage usage;
	b8 render_target;
};

struct gfx_texture_3d_desc_t {
	str_t name;
	uvec3_t size;
	gfx_texture_format format;
	gfx_usage usage;
};

struct gfx_shader_attribute_t {
	char* name;
	u32 slot;
	gfx_vertex_format format;
	gfx_vertex_class classification;
};

struct gfx_shader_desc_t {
	str_t name;
	str_t filepath;
	gfx_shader_attribute_t* attributes;
	u32 attribute_count;
};

struct gfx_compute_shader_desc_t {
	str_t name;
	str_t filepath;
};

struct gfx_render_target_desc_t {
	uvec2_t size;
	u32 sample_count;
	gfx_render_target_flags flags;
	gfx_texture_format format;
};

struct gfx_pipeline_t {
	gfx_fill_mode fill_mode;
	gfx_cull_mode cull_mode;
	gfx_topology_type topology;
	gfx_filter_mode filter_mode;
	gfx_wrap_mode wrap_mode;
	gfx_depth_mode depth_mode;
	rect_t viewport;
	rect_t scissor;
};


//- functions

// state (implemented per backend)
function void gfx_init();
function void gfx_release();
function void gfx_update();

function void gfx_draw(u32 vertex_count, u32 start_index = 0);
function void gfx_draw_indexed(u32 index_count, u32 start_index = 0, u32 offset = 0);
function void gfx_draw_instanced(u32 vertex_count, u32 instance_count, u32 start_vertex_index = 0, u32 start_instnace_index = 0);

function void gfx_dispatch(u32 thread_group_x, u32 thread_group_y, u32 thread_group_z);

function void gfx_set_sampler(gfx_filter_mode filter, gfx_wrap_mode wrap, u32 slot);
function void gfx_set_topology(gfx_topology_type topology);
function void gfx_set_rasterizer(gfx_fill_mode fill, gfx_cull_mode cull);
function void gfx_set_viewport(rect_t viewport);
function void gfx_set_scissor(rect_t scissor);
function void gfx_set_depth_mode(gfx_depth_mode depth);
function void gfx_set_pipeline(gfx_pipeline_t pipeline);
function void gfx_set_buffer(gfx_handle_t buffer, u32 slot = 0, u32 stride = 0);
function void gfx_set_texture(gfx_handle_t texture, u32 slot = 0, gfx_texture_usage usage = gfx_texture_usage_ps);
function void gfx_set_texture_array(gfx_handle_t* textures, u32 texture_count, u32 slot, gfx_texture_usage usage);
function void gfx_set_texture_3d(gfx_handle_t texture, u32 slot = 0, gfx_texture_usage usage = gfx_texture_usage_ps);
function void gfx_set_shader(gfx_handle_t shader = {0});
function void gfx_set_compute_shader(gfx_handle_t compute_shader = { 0 });
function void gfx_set_render_target(gfx_handle_t render_target = { 0 });

// handle
function b8 gfx_handle_equals(gfx_handle_t a, gfx_handle_t b);

// pipeline
function gfx_pipeline_t gfx_pipeline_create();

// renderer (implemented per backend)
function gfx_handle_t gfx_renderer_create(os_handle_t window, color_t clear_color);
function void gfx_renderer_release(gfx_handle_t renderer);
function void gfx_renderer_resize(gfx_handle_t renderer, uvec2_t size);
function void gfx_renderer_begin(gfx_handle_t renderer);
function void gfx_renderer_end(gfx_handle_t renderer);
function uvec2_t gfx_renderer_get_size(gfx_handle_t renderer);

// buffer (implemented per backend)
function gfx_handle_t gfx_buffer_create_ex(gfx_buffer_desc_t desc, void* data);
function gfx_handle_t gfx_buffer_create(gfx_buffer_type type, u32 size, void* data = nullptr);
function void         gfx_buffer_release(gfx_handle_t buffer);
function void         gfx_buffer_fill(gfx_handle_t buffer, void* data, u32 size);

// texture (implemented per backend)
function gfx_handle_t gfx_texture_create_ex(gfx_texture_desc_t texture_desc, void* data = nullptr);
function gfx_handle_t gfx_texture_create(uvec2_t size, gfx_texture_format format = gfx_texture_format_rgba8, void* data = nullptr);
function gfx_handle_t gfx_texture_load(str_t filepath);
function void gfx_texture_release(gfx_handle_t texture);
function uvec2_t gfx_texture_get_size(gfx_handle_t texture);
function void gfx_texture_resize(gfx_handle_t texture, uvec2_t size);
function void gfx_texture_fill(gfx_handle_t texture, void* data);
function void gfx_texture_fill_region(gfx_handle_t texture, rect_t region, void* data);
function void gfx_texture_blit(gfx_handle_t texture_dst, gfx_handle_t texture_src);
function void gfx_texture_write(gfx_handle_t texture, str_t filepath);

function gfx_handle_t gfx_texture_3d_create_ex(gfx_texture_3d_desc_t desc, void* data = nullptr);
function gfx_handle_t gfx_texture_3d_create(str_t name, uvec3_t size, gfx_texture_format format = gfx_texture_format_rgba8, void* data = nullptr);
function void gfx_texture_3d_release(gfx_handle_t texture);

// shaders (implemented per backend)
function gfx_handle_t gfx_shader_create_ex(str_t, gfx_shader_desc_t);
function gfx_handle_t gfx_shader_create(str_t, str_t, gfx_shader_attribute_t*, u32);
function gfx_handle_t gfx_shader_load(str_t, gfx_shader_attribute_t*, u32);
function void gfx_shader_release(gfx_handle_t);
function void gfx_shader_compile(gfx_handle_t, str_t);

// compute shaders (implemented per backend)
function gfx_handle_t gfx_compute_shader_create_ex(str_t src, gfx_compute_shader_desc_t desc);
function gfx_handle_t gfx_compute_shader_create(str_t src, str_t name);
function gfx_handle_t gfx_compute_shader_load(str_t filepath);
function void gfx_compute_shader_release(gfx_handle_t shader);
function void gfx_compute_shader_compile(gfx_handle_t shader, str_t src);

// render target (implemented per backend)
function gfx_handle_t gfx_render_target_create_ex(gfx_render_target_desc_t desc);
function gfx_handle_t gfx_render_target_create(gfx_texture_format format, uvec2_t size, gfx_render_target_flags flags = 0);
function void gfx_render_target_release(gfx_handle_t render_target);
function void gfx_render_target_resize(gfx_handle_t render_target, uvec2_t size);
function void gfx_render_target_clear(gfx_handle_t render_target, color_t clear_color = color(0x000000ff), f32 clear_depth = 1.0f);
function void gfx_render_target_create_resources(gfx_handle_t render_target);
function uvec2_t gfx_render_target_get_size(gfx_handle_t render_target);

// helper functions
function b8 gfx_texture_format_is_depth(gfx_texture_format format);

//- per backend includes

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
#    include "backends/gfx/sora_gfx_d3d12.h"
#elif GFX_BACKEND_OPENGL
#    include "backends/gfx/sora_gfx_opengl.h"
#elif GFX_BACKEND_METAL
#    include "backends/gfx/sora_gfx_metal.h"
#elif GFX_BACKEND_VULKAN
#    include "backends/gfx/sora_gfx_vulkan.h"
#endif 

#endif // SORA_GFX_H 