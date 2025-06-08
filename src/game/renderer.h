// renderer.h

#ifndef RENDERER_H
#define RENDERER_H

//- typedefs 

struct render_pass_t;
typedef void render_pass_init_func(render_pass_t*);
typedef void render_pass_execute_func(render_pass_t*);

//- structs 

struct render_pass_t {
    
    // global list
    render_pass_t* global_next;
    render_pass_t* global_prev;
    
    
    
    
    str_t label;
    render_pass_init_func* init_func;
    render_pass_execute_func* execute_func;
    
};

struct renderer_t {
    
    arena_t* arena;
    
    gfx_handle_t context;
    
    camera_t* camera;
    
    gfx_handle_t vertex_buffer;
    gfx_handle_t index_buffer;
    gfx_handle_t transform_constant_buffer;
    gfx_handle_t camera_constant_buffer;
    
};

//- functions 

function renderer_t* renderer_create(gfx_handle_t graphics_device);
function void renderer_release(renderer_t* renderer);
function void renderer_set_camera(renderer_t* renderer, camera_t* camera);

function void renderer_draw_mesh(renderer_t* renderer, mesh_t* mesh, mat4_t transform, gfx_handle_t vertex_shader, gfx_handle_t pixel_shader);

#endif // RENDERER_H