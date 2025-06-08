// renderer.cpp

#ifndef RENDERER_CPP
#define RENDERER_CPP

//- implementation 

function renderer_t*
renderer_create(gfx_handle_t graphics_context) {
    
    arena_t* arena = arena_create(gigabytes(1));
    
    renderer_t* renderer = (renderer_t*)arena_alloc(arena, sizeof(renderer_t));
    
    renderer->arena = arena;
    renderer->context = graphics_context;
    
    renderer->vertex_buffer = gfx_buffer_create(gfx_buffer_type_vertex, megabytes(256));
    renderer->index_buffer = gfx_buffer_create(gfx_buffer_type_index, megabytes(16));
    renderer->transform_constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, kilobytes(64));
    renderer->camera_constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, sizeof(camera_constants_t));
    
    return renderer;
}

function void
renderer_release(renderer_t* renderer) {
    
    // release buffers
    gfx_buffer_release(renderer->camera_constant_buffer);
    gfx_buffer_release(renderer->transform_constant_buffer);
    gfx_buffer_release(renderer->index_buffer);
    gfx_buffer_release(renderer->vertex_buffer);
    
    // release arena
    arena_release(renderer->arena);
    
}

function void 
renderer_draw_mesh(renderer_t* renderer, mesh_t* mesh, mat4_t transform, gfx_handle_t vertex_shader, gfx_handle_t pixel_shader) {
    
    uvec2_t window_size = gfx_context_get_size(renderer->context);
    rect_t viewport = rect(0.0f, 0.0f, (f32)window_size.x, (f32)window_size.y);
    
    // set pipeline
    gfx_set_viewport(viewport);
    gfx_set_scissor(viewport);
    gfx_set_rasterizer(gfx_fill_solid, gfx_cull_back);
    gfx_set_topology(gfx_topology_tris);
    gfx_set_sampler(gfx_filter_nearest, gfx_wrap_repeat, 0);
    gfx_set_depth_mode(gfx_depth);
    
    // fill buffers
    gfx_buffer_fill(renderer->vertex_buffer, mesh->vertices, sizeof(vertex_t) * mesh->vertex_count);
    gfx_buffer_fill(renderer->index_buffer, mesh->indices, sizeof(i32) * mesh->index_count);
    gfx_buffer_fill(renderer->transform_constant_buffer, &transform, sizeof(mat4_t));
    
    // set shaders
    gfx_set_shader(vertex_shader);
    gfx_set_shader(pixel_shader);
    
    // set buffers
    gfx_set_buffer(renderer->vertex_buffer, 0, sizeof(vertex_t));
    gfx_set_buffer(renderer->index_buffer, 0);
    gfx_set_buffer(renderer->camera_constant_buffer, 0);
    gfx_set_buffer(renderer->transform_constant_buffer, 1);
    
    // draw
    gfx_draw_indexed(mesh->index_count, 0);
    
}

function void
renderer_set_camera(renderer_t* renderer, camera_t* camera) {
    renderer->camera = camera;
    
    // fill buffer
    gfx_buffer_fill(renderer->camera_constant_buffer, &camera->constants, sizeof(camera_constants_t));
    
}

#endif // RENDERER_CPP