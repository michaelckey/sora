// debug.h

#ifndef DEBUG_H
#define DEBUG_H

//~ enums

enum debug_batch_type {
    debug_batch_type_none,
    debug_batch_type_line,
    debug_batch_type_mesh_solid,
    debug_batch_type_mesh_wireframe,
};

//~ structs

struct debug_vertex_t {
    vec3_t position;
    color_t color;
};

struct debug_batch_t {
    debug_batch_t* next;
    debug_batch_t* prev;
    
    debug_batch_type type;
    
    debug_vertex_t* vertices;
    u32 vertex_count;
    
    i32* indices;
    u32 index_count;
};

struct debug_state_t {
    
    arena_t* arena;
    
    gfx_handle_t vertex_shader;
    gfx_handle_t pixel_shader;
    
    gfx_handle_t vertex_buffer;
    gfx_handle_t index_buffer;
    gfx_handle_t constant_buffer;
    
    debug_batch_t* batch_first;
    debug_batch_t* batch_last;
    debug_batch_t* batch_free;
    
    gfx_handle_t camera;
    
};

//~ globals

global debug_state_t debug_state;

//~ functions 

function void debug_init();
function void debug_release();
function void debug_render();

function void debug_set_camera(gfx_handle_t camera);

function void debug_draw_line(vec3_t p0, vec3_t p1, color_t col);
function void debug_draw_line(vec3_t p0, vec3_t p1, color_t c0, color_t c1);
function void debug_draw_quad(vec3_t p0, vec3_t p1, vec3_t p2, vec3_t p3, color_t col);
function void debug_draw_sphere(vec3_t center, f32 radius, color_t col);
function void debug_draw_mesh(mesh_t* mesh, color_t col, b8 wireframe = false);

// internal
function debug_batch_t* _debug_batch_find(debug_batch_type type, u32 vertex_count, u32 index_count = 0);
inlnfunc void _debug_batch_push_vertex(debug_batch_t* batch, debug_vertex_t vertex);
inlnfunc void _debug_batch_push_indices(debug_batch_t* batch, i32 i0, i32 i1, i32 i2);

#endif // DEBUG_H