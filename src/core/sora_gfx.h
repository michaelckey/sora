// sora_gfx.h

#ifndef SORA_GFX_H
#define SORA_GFX_H

//- includes

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendor/stb_image_write.h"

#define sora_gfx_mesh_magic_number 0x48534D52
#define sora_gfx_mesh_file_version 1

//- enums

enum gfx_resource_type {
	gfx_resource_type_null,
	gfx_resource_type_buffer,
	gfx_resource_type_texture,
	gfx_resource_type_shader,
	gfx_resource_type_render_target,
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

// pipeline

enum gfx_cull_mode {
	gfx_cull_null,
	gfx_cull_none,
	gfx_cull_front,
	gfx_cull_back,
};

enum gfx_depth_mode {
	gfx_depth_null,
	gfx_depth,
	gfx_depth_none,
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

enum gfx_topology_type {
	gfx_topology_null,
	gfx_topology_points,
	gfx_topology_lines,
	gfx_topology_line_strip,
	gfx_topology_tris,
	gfx_topology_tri_strip,
};


typedef u32 gfx_camera_mode;
enum {
	gfx_camera_mode_disable_roll = (1 << 0),
	gfx_camera_mode_disable_move_in_world_plane = (1 << 1),
	gfx_camera_mode_clamp_pitch = (1 << 2),
	gfx_camera_mode_clamp_yaw = (1 << 3),
	gfx_camera_mode_clamp_roll = (1 << 4),
    
	gfx_camera_mode_free = 0,
	gfx_camera_mode_first_person = gfx_camera_mode_disable_roll | gfx_camera_mode_disable_move_in_world_plane | gfx_camera_mode_clamp_pitch,
	gfx_camera_mode_orbit = gfx_camera_mode_disable_roll | gfx_camera_mode_clamp_pitch
};


//- typedefs 

struct gfx_handle_t;
typedef void gfx_render_pass_init_func(gfx_handle_t);
typedef void gfx_render_pass_execute_func(gfx_handle_t);

//- structs

// handle
struct gfx_handle_t {
	u64 data[1];
};

// buffers
struct gfx_buffer_desc_t {
	gfx_buffer_type type;
	u32 size;
	gfx_usage usage;
};

// textures
struct gfx_texture_desc_t {
	str_t name;
	uvec2_t size;
	gfx_texture_format format;
	gfx_texture_type type;
	gfx_texture_flags flags;
	gfx_usage usage;
	u32 sample_count;
};

// shaders
struct gfx_shader_desc_t {
	str_t name;
	str_t filepath;
	gfx_shader_flags flags;
    u64 last_write_time;
};

// render target
struct gfx_render_target_desc_t {
	uvec2_t size;
	u32 sample_count;
	gfx_texture_format colorbuffer_format;
	gfx_texture_format depthbuffer_format;
};

// pipeline
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

// forward declare
struct gfx_render_port_t;
struct gfx_render_connection_t;
struct gfx_render_pass_t;
struct gfx_render_graph_t;

// ports
struct gfx_render_port_t {
    gfx_render_port_t* next;
    gfx_render_port_t* prev;
    
    str_t label;
    gfx_render_pass_t* pass;
    gfx_render_port_t* connected_port;
    gfx_handle_t resource;
};

struct gfx_render_port_list_t {
    gfx_render_port_t* first;
    gfx_render_port_t* last;
};

// connections
struct gfx_render_connection_t {
    
    // global list
    gfx_render_connection_t* next;
    gfx_render_connection_t* prev;
    
    // ports
    gfx_render_port_t* from;
    gfx_render_port_t* to;
    
};

// render pass
struct gfx_render_pass_t {
    gfx_render_graph_t* graph;
    
    // global list
    gfx_render_pass_t* next;
    gfx_render_pass_t* prev;
    
    // execute list
    gfx_render_pass_t* execute_next;
    gfx_render_pass_t* execute_prev;
    
    // funcs
    gfx_render_pass_init_func* init_func;
    gfx_render_pass_execute_func* execute_func;
    
    // ports
    gfx_render_port_list_t inputs;
    gfx_render_port_list_t outputs;
    
    // topological sort
    u32 in_degree;
};

struct gfx_render_pass_ts_queue_t {
    gfx_render_pass_ts_queue_t* next;
    gfx_render_pass_ts_queue_t* prev;
    gfx_render_pass_t* pass;
};

// render graph
struct gfx_render_graph_t {
    
    // arenas
    arena_t* arena;
    
    // handles
    gfx_handle_t renderer;
    
    // passes
    gfx_render_pass_t* pass_first;
    gfx_render_pass_t* pass_last;
    gfx_render_pass_t* pass_free;
    
    // execute list
    gfx_render_pass_t* execute_first;
    gfx_render_pass_t* execute_last;
    
    // ports
    gfx_render_port_t* port_free;
    
    // connections
    gfx_render_connection_t* connection_first;
    gfx_render_connection_t* connection_last;
    gfx_render_connection_t* connection_free;
    
};

// mesh

struct gfx_face_index_t {
    i32 position_index;
    i32 texcoord_index;
    i32 normal_index;
};

struct gfx_face_t {
    gfx_face_index_t indices[4];
    u32 index_count;
};

struct gfx_vertex_lookup_t {
    i32 position_index;
    i32 texcoord_index;
    i32 normal_index;
    i32 new_index;
};

struct gfx_vertex_t {
    vec3_t position;
    vec3_t normal;
    vec3_t tangent;
    vec2_t texcoord;
    vec4_t color;
};

struct gfx_mesh_t {
    gfx_vertex_t* vertices;
    u32 vertex_count;
    
    i32* indices;
    u32 index_count;
};

struct gfx_mesh_file_header_t {
    u32 magic;
    u32 version;
    u32 vertex_count;
    u32 index_count;
    u32 flags;
};

// camera

struct gfx_camera_constants_t {
    mat4_t view_projection;
    mat4_t view;
    mat4_t projection;
    mat4_t inv_view;
    mat4_t inv_projection;
    vec3_t camera_position;
};

struct gfx_camera_t {
    
    gfx_camera_mode mode;
    gfx_camera_constants_t constants;
    
    vec3_t target_position;
    vec3_t position;
    
    f32 target_distance;
    f32 distance;
    
    quat_t target_orientation;
    quat_t orientation;
    
    vec3_t translational_input;
    vec3_t rotational_input;
    
    f32 speed;
    f32 target_speed;
    f32 target_fov;
    f32 fov;
    f32 z_near;
    f32 z_far;
    f32 min_pitch;
    f32 max_pitch;
    f32 min_yaw;
    f32 max_yaw;
    f32 min_roll;
    f32 max_roll;
};

//~ functions 

// handle (implemented once)
function b8 gfx_handle_equals(gfx_handle_t a, gfx_handle_t b);

// pipeline (implemented once)
function gfx_pipeline_t gfx_pipeline_create();

// helper functions (implemented once)
function b8 gfx_texture_format_is_depth(gfx_texture_format format);

//~ high level functions

// state (implemented per backend)
function void gfx_init();
function void gfx_release();

// renderer (implemented per backend)
function gfx_handle_t gfx_renderer_create(os_handle_t window);
function void gfx_renderer_release(gfx_handle_t renderer);
function void gfx_renderer_set_size(gfx_handle_t renderer, uvec2_t size);
function uvec2_t gfx_renderer_get_size(gfx_handle_t renderer);
function void gfx_renderer_update(gfx_handle_t renderer);
function void gfx_renderer_resize(gfx_handle_t renderer, uvec2_t size);
function void gfx_renderer_clear(gfx_handle_t renderer, color_t clear_color);
function void gfx_renderer_present(gfx_handle_t renderer);

// resources (implemented once)

// textures
function gfx_handle_t gfx_texture_load(str_t filepath, gfx_texture_flags flags = 0);

// shaders
function gfx_handle_t gfx_shader_load(str_t filepath, gfx_shader_flags flags = 0);

// mesh
function gfx_handle_t gfx_mesh_load(arena_t* arena, str_t filepath);
function u32 gfx_mesh_get_vertex_count(gfx_handle_t mesh);
function u32 gfx_mesh_get_index_count(gfx_handle_t mesh);
function gfx_vertex_t* gfx_mesh_get_vertices(gfx_handle_t mesh);
function i32* gfx_mesh_get_indices(gfx_handle_t mesh);

// camera
function gfx_handle_t gfx_camera_create(arena_t* arena, gfx_camera_mode mode, f32 fov, f32 z_near, f32 z_far);
function void gfx_camera_free_mode_input(gfx_handle_t camera, os_handle_t window);
function void gfx_camera_update(gfx_handle_t camera, rect_t viewport, f32 dt);
function gfx_camera_constants_t* gfx_camera_get_constants(gfx_handle_t camera);
function mat4_t gfx_camera_get_view(gfx_handle_t camera);
function mat4_t gfx_camera_get_projection(gfx_handle_t camera);

// render graphs (implemented once)
function gfx_handle_t gfx_render_graph_create(gfx_handle_t renderer);
function void gfx_render_graph_release(gfx_handle_t render_graph);
function void gfx_render_graph_compile(gfx_handle_t render_graph);
function void gfx_render_graph_execute(gfx_handle_t render_graph);

// render passes (implemented once)
function gfx_handle_t gfx_render_pass_create(gfx_handle_t render_graph, gfx_render_pass_init_func init_func, gfx_render_pass_execute_func execute_func);
function void gfx_render_pass_release(gfx_handle_t render_pass);
function void gfx_render_pass_add_input(gfx_handle_t render_pass, str_t label);
function void gfx_render_pass_add_output(gfx_handle_t render_pass, str_t label);
function void gfx_render_pass_link(gfx_handle_t src, str_t output_label, gfx_handle_t dst, str_t input_label);
function void gfx_render_pass_unlink(gfx_handle_t src, str_t output_label, gfx_handle_t dst, str_t input_label);
function void gfx_render_pass_set_output(gfx_handle_t render_pass, str_t output_label, gfx_handle_t resource);
function gfx_handle_t gfx_render_pass_get_input(gfx_handle_t render_pass, str_t input_label);
function gfx_handle_t gfx_render_pass_get_output(gfx_handle_t render_pass, str_t output_label);

// internal (implemented once)

// render graph internal
function gfx_render_port_t* _gfx_render_port_alloc(gfx_render_graph_t* render_graph);
function gfx_render_port_t* _gfx_render_port_find(gfx_render_port_list_t list, str_t label);
function gfx_render_connection_t* _gfx_render_connection_alloc(gfx_render_graph_t* render_graph);
function gfx_render_connection_t* _gfx_render_connection_find(gfx_render_graph_t* render_graph, gfx_render_port_t* from, gfx_render_port_t* to);

// mesh internal
function void _gfx_mesh_load_from_obj(arena_t* arena, gfx_mesh_t* mesh, str_t filepath);
function void _gfx_mesh_load_from_binary(arena_t* arena, gfx_mesh_t* mesh, str_t filepath);
function void _gfx_mesh_write_to_obj(gfx_mesh_t* mesh, str_t filepath);
function void _gfx_mesh_write_to_binary(gfx_mesh_t* mesh, str_t filepath);

//~ low level functions

// render state (implemented per backend)
function void gfx_draw(u32 vertex_count, u32 start_index = 0);
function void gfx_draw_indexed(u32 index_count, u32 start_index = 0, u32 offset = 0);
function void gfx_draw_instanced(u32 vertex_count, u32 instance_count, u32 start_vertex_index = 0, u32 start_instance_index = 0);
function void gfx_dispatch(u32 thread_group_x, u32 thread_group_y, u32 thread_group_z);
function void gfx_set_context(gfx_handle_t context);
function void gfx_set_sampler(gfx_filter_mode filter, gfx_wrap_mode wrap, u32 slot);
function void gfx_set_topology(gfx_topology_type topology);
function void gfx_set_rasterizer(gfx_fill_mode fill, gfx_cull_mode cull);
function void gfx_set_viewport(rect_t viewport);
function void gfx_set_scissor(rect_t scissor);
function void gfx_set_depth_mode(gfx_depth_mode depth);
function void gfx_set_pipeline(gfx_pipeline_t pipeline);
function void gfx_set_buffer(gfx_handle_t buffer, u32 slot = 0, u32 stride = 0);
function void gfx_set_texture(gfx_handle_t texture, u32 slot = 0);
function void gfx_set_texture_array(gfx_handle_t* textures, u32 texture_count, u32 slot);
function void gfx_set_shader(gfx_handle_t shader = {0});
function void gfx_set_render_target(gfx_handle_t render_target = { 0 });

// buffer (implemented per backend)
function gfx_handle_t gfx_buffer_create_ex(gfx_buffer_desc_t desc, void* initial_data = nullptr);
function gfx_handle_t gfx_buffer_create(gfx_buffer_type type, u32 size, void* initial_data = nullptr);
function void         gfx_buffer_release(gfx_handle_t buffer);
function void         gfx_buffer_fill(gfx_handle_t buffer, void* data, u32 size);

// texture (implemented per backend)
function gfx_handle_t gfx_texture_create_ex(gfx_texture_desc_t texture_desc, void* data = nullptr);
function gfx_handle_t gfx_texture_create(uvec2_t size, gfx_texture_format format = gfx_texture_format_rgba8, void* data = nullptr);
function void gfx_texture_release(gfx_handle_t texture);
function uvec2_t gfx_texture_get_size(gfx_handle_t texture);
function void gfx_texture_resize(gfx_handle_t texture, uvec2_t size);
function void gfx_texture_fill(gfx_handle_t texture, void* data);
function void gfx_texture_fill_region(gfx_handle_t texture, rect_t region, void* data);
function void gfx_texture_blit(gfx_handle_t texture_dst, gfx_handle_t texture_src);

// shaders (implemented per backend)
function gfx_handle_t gfx_shader_create_ex(gfx_shader_desc_t shader_desc);
function gfx_handle_t gfx_shader_create(str_t name, gfx_shader_flags flags);
function void gfx_shader_release(gfx_handle_t shader);
function void gfx_shader_compile(gfx_handle_t shader, str_t src);
function void gfx_shader_set_binary(gfx_handle_t shader, void* binary, u32 binary_size);
function void gfx_shader_get_binary(gfx_handle_t handle, void** out_binary, u32* out_binary_size);

// render target (implemented per backend)
function gfx_handle_t gfx_render_target_create_ex(gfx_render_target_desc_t desc);
function gfx_handle_t gfx_render_target_create(uvec2_t size, gfx_texture_format colorbuffer_format, gfx_texture_format depthbuffer_format = gfx_texture_format_null);
function void gfx_render_target_release(gfx_handle_t render_target);
function void gfx_render_target_resize(gfx_handle_t render_target, uvec2_t size);
function void gfx_render_target_clear(gfx_handle_t render_target, color_t clear_color = color(0x000000ff), f32 clear_depth = 1.0f);
function void gfx_render_target_create_resources(gfx_handle_t render_target);
function uvec2_t gfx_render_target_get_size(gfx_handle_t render_target);

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