// renderer.h

// this is a simple line and point renderer
// for my SPH fluid simulation.

#ifndef RENDERER_H
#define RENDERER_H

//- defines 

#define renderer_max_batch_size (megabytes(8))

//- structs 

struct point_instance_t {
    vec3_t position;
    u32 color;
};

struct line_vertex_t {
    vec3_t position;
    u32 color;
};

struct renderer_t {
    
    arena_t* arena;
    rect_t viewport;
    
    gfx_handle_t context;
    gfx_handle_t render_target;
    gfx_handle_t constant_buffer;
    gfx_handle_t render_target_texture;
    
    // point renderer
    gfx_handle_t point_instance_buffer;
    gfx_handle_t point_vertex_shader;
    gfx_handle_t point_pixel_shader;
    point_instance_t* point_instance_data;
    u32 point_instance_count;
    
    // line renderer
    gfx_handle_t line_vertex_buffer;
    gfx_handle_t line_vertex_shader;
    gfx_handle_t line_pixel_shader;
    line_vertex_t* line_vertex_data;
    u32 line_vertex_count;
    
};

//- globals

global renderer_t renderer;

//- functions 

function void renderer_init(gfx_handle_t context);
function void renderer_release();

function void renderer_begin(camera_t* camera);
function void renderer_end();

function void renderer_flush_points();
function void renderer_flush_lines();
function void renderer_draw_point(vec3_t position, u32 color);
function void renderer_draw_point_bulk(vec3_t* positions, u32* colors, u32 count);
function void renderer_draw_line(vec3_t p0, vec3_t p1, u32 color);

#endif // RENDERER_H