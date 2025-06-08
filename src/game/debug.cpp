// debug.cpp

#ifndef DEBUG_CPP
#define DEBUG_CPP

//~ implementation

function void 
debug_init() {
    
    debug_state.arena = arena_create(gigabytes(1));
    
    // load assets
    debug_state.vertex_shader = gfx_shader_load(str("res/shaders/shader_debug.hlsl"), gfx_shader_flag_vertex);
    debug_state.pixel_shader = gfx_shader_load(str("res/shaders/shader_debug.hlsl"), gfx_shader_flag_pixel);
    
    // create buffers
    debug_state.vertex_buffer = gfx_buffer_create(gfx_buffer_type_vertex, megabytes(64));
    debug_state.index_buffer = gfx_buffer_create(gfx_buffer_type_index, megabytes(64));
    debug_state.constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, kilobytes(64));
    
    // reset batches
    debug_state.batch_first = nullptr;
    debug_state.batch_last = nullptr;
    debug_state.batch_free = nullptr;
    
}

function void
debug_release() {
    
    // release buffers
    gfx_buffer_release(debug_state.vertex_buffer);
    gfx_buffer_release(debug_state.index_buffer);
    gfx_buffer_release(debug_state.constant_buffer);
    
    // release assets
    gfx_shader_release(debug_state.vertex_shader);
    gfx_shader_release(debug_state.pixel_shader);
    
    // release arena
    arena_release(debug_state.arena);
}

function void
debug_render() {
    
    // set shaders and constant buffer
    gfx_set_shader(debug_state.vertex_shader);
    gfx_set_shader(debug_state.pixel_shader);
    gfx_set_buffer(debug_state.constant_buffer, 0);
    
    // draw batches
    for (debug_batch_t* batch = debug_state.batch_first; batch != nullptr; batch = batch->next) {
        
        b8 wireframe = false;
        switch (batch->type) {
            
            case debug_batch_type_line: {
                
                // set pipeline state
                gfx_set_rasterizer(gfx_fill_solid, gfx_cull_none);
                gfx_set_topology(gfx_topology_lines);
                gfx_set_depth_mode(gfx_depth);
                
                // fill vertex buffer
                gfx_buffer_fill(debug_state.vertex_buffer, batch->vertices, sizeof(debug_vertex_t) * batch->vertex_count);
                gfx_set_buffer(debug_state.vertex_buffer, 0, sizeof(debug_vertex_t));
                
                // draw
                gfx_draw(batch->vertex_count);
                
                break;
            }
            
            case debug_batch_type_mesh_wireframe: {
                wireframe = true;
                // fallthrough
            }
            
            case debug_batch_type_mesh_solid: { 
                
                // set pipeline state
                gfx_set_rasterizer(wireframe ? gfx_fill_wireframe : gfx_fill_solid, gfx_cull_none);
                gfx_set_topology(gfx_topology_tris);
                gfx_set_depth_mode(gfx_depth);
                
                // fill buffers
                gfx_buffer_fill(debug_state.vertex_buffer, batch->vertices, sizeof(debug_vertex_t) * batch->vertex_count);
                gfx_set_buffer(debug_state.vertex_buffer, 0, sizeof(debug_vertex_t));
                gfx_buffer_fill(debug_state.index_buffer, batch->indices, sizeof(i32) * batch->index_count);
                gfx_set_buffer(debug_state.index_buffer, 0, sizeof(i32));
                
                // draw
                gfx_draw_indexed(batch->index_count);
                
                break;
            }
            
        }
        
    }
    
    // clear state
    arena_clear(debug_state.arena);
    debug_state.batch_first = nullptr;
    debug_state.batch_last = nullptr;
    debug_state.batch_free = nullptr;
    
}


function void
debug_set_camera(gfx_handle_t camera) {
    debug_state.camera = camera;
}

function void 
debug_draw_line(vec3_t p0, vec3_t p1, color_t col) {
    debug_batch_t* batch = _debug_batch_find(debug_batch_type_line, 2);
    _debug_batch_push_vertex(batch, {p0, col});
    _debug_batch_push_vertex(batch, {p1, col});
}

function void 
debug_draw_line(vec3_t p0, vec3_t p1, color_t c0, color_t c1) {
    debug_batch_t* batch = _debug_batch_find(debug_batch_type_line, 2);
    _debug_batch_push_vertex(batch, {p0, c0});
    _debug_batch_push_vertex(batch, {p1, c1});
}

function void
debug_draw_tri(vec3_t p0, vec3_t p1, vec3_t p2, color_t col) {
    debug_batch_t* batch = _debug_batch_find(debug_batch_type_mesh_solid, 3, 3);
    
    // add indices
    u32 vertex_count = batch->vertex_count;
    _debug_batch_push_indices(batch, vertex_count + 0, vertex_count + 2, vertex_count + 1);
    
    // add vertices
    _debug_batch_push_vertex(batch, { p0, col });
    _debug_batch_push_vertex(batch, { p1, col });
    _debug_batch_push_vertex(batch, { p2, col });
    
}

function void 
debug_draw_quad(vec3_t p0, vec3_t p1, vec3_t p2, vec3_t p3, color_t col) {
    debug_batch_t* batch = _debug_batch_find(debug_batch_type_mesh_solid, 4, 6);
    
    // add indices
    u32 vertex_count = batch->vertex_count;
    _debug_batch_push_indices(batch, vertex_count + 0, vertex_count + 2, vertex_count + 1);
    _debug_batch_push_indices(batch, vertex_count + 1, vertex_count + 2, vertex_count + 3);
    
    // add vertices
    _debug_batch_push_vertex(batch, { p0, col });
    _debug_batch_push_vertex(batch, { p1, col });
    _debug_batch_push_vertex(batch, { p2, col });
    _debug_batch_push_vertex(batch, { p3, col });
    
}

function void 
debug_draw_sphere(vec3_t center, f32 radius, color_t color) {
    
    const i32 hor_resolution = 5;
    const i32 ver_resolution = 7;
    
    vec3_t p0;
    vec3_t p1;
    
    for (int i = 0; i < hor_resolution; i++) {
        
        f32 lat = f32_pi * (f32)i / hor_resolution;
        f32 sin_lat = sinf(lat);
        f32 cos_lat = cosf(lat);
        
        for (i32 j = 0; j < ver_resolution; j++) {
            
            f32 lon_0 = 2.0f * f32_pi * (f32)j / ver_resolution;
            f32 lon_1 = 2.0f * f32_pi * (f32)((j + 1) % ver_resolution) / ver_resolution;
            
            f32 sin_lon_0 = sinf(lon_0);
            f32 cos_lon_0 = cosf(lon_0);
            f32 sin_lon_1 = sinf(lon_1);
            f32 cos_lon_1 = cosf(lon_1);
            
            p0.x = center.x + radius * sin_lat * cos_lon_0;
            p0.y = center.y + radius * cos_lat;
            p0.z = center.z + radius * sin_lat * sin_lon_0;
            
            p1.x = center.x + radius * sin_lat * cos_lon_1;
            p1.y = center.y + radius * cos_lat;
            p1.z = center.z + radius * sin_lat * sin_lon_1;
            
            debug_draw_line(p0, p1, color);
        }
    }
    
    
    for (i32 j = 0; j < ver_resolution; j++) {
        
        f32 lon = 2.0f * f32_pi * (f32)j / ver_resolution;
        f32 sin_lon = sinf(lon);
        f32 cos_lon = cosf(lon);
        
        for (i32 i = 0; i < hor_resolution; i++) {
            f32 lat_0 = f32_pi * (f32)i / hor_resolution;
            f32 lat_1 = f32_pi * (f32)(i + 1) / hor_resolution;
            
            f32 sin_lat_0 = sinf(lat_0);
            f32 cos_lat_0 = cosf(lat_0);
            f32 sin_lat_1 = sinf(lat_1);
            f32 cos_lat_1 = cosf(lat_1);
            
            p0.x = center.x + radius * sin_lat_0 * cos_lon;
            p0.y = center.y + radius * cos_lat_0;
            p0.z = center.z + radius * sin_lat_0 * sin_lon;
            
            p1.x = center.x + radius * sin_lat_1 * cos_lon;
            p1.y = center.y + radius * cos_lat_1;
            p1.z = center.z + radius * sin_lat_1 * sin_lon;
            
            debug_draw_line(p0, p1, color);
        }
    }
    
}

//- internal functions 

function debug_batch_t* 
_debug_batch_find(debug_batch_type type, u32 vertex_count, u32 index_count) {
    
    debug_batch_t* batch = nullptr;
    for (debug_batch_t* b = debug_state.batch_first; b != nullptr; b = b->next) {
        if ((b->type == type) && 
            (b->vertex_count + vertex_count) * sizeof(debug_vertex_t) < megabytes(64) &&
            (b->index_count + index_count) * sizeof(i32) < megabytes(64)) {
            batch = b;
            break;
        }
    }
    
    if (batch == nullptr) {
        
        batch = debug_state.batch_free;
        if (batch != nullptr) {
            stack_pop(debug_state.batch_free);
        } else {
            batch = (debug_batch_t*)arena_alloc(debug_state.arena, sizeof(debug_batch_t));
            batch->vertices = (debug_vertex_t*)arena_alloc(debug_state.arena, megabytes(64));
            batch->indices = (i32*)arena_alloc(debug_state.arena, megabytes(64));
        }
        
        batch->type = type;
        batch->next = nullptr;
        batch->prev = nullptr;
        batch->vertex_count = 0;
        batch->index_count = 0;
        dll_push_back(debug_state.batch_first, debug_state.batch_last, batch);
        
    }
    
    return batch;
}

inlnfunc void
_debug_batch_push_vertex(debug_batch_t* batch, debug_vertex_t vertex) {
    batch->vertices[batch->vertex_count++] = vertex;
}

inlnfunc void
_debug_batch_push_indices(debug_batch_t* batch, i32 i0, i32 i1, i32 i2) {
    batch->indices[batch->index_count + 0] = i0;
    batch->indices[batch->index_count + 1] = i1;
    batch->indices[batch->index_count + 2] = i2;
    batch->index_count += 3;
}

#endif // DEBUG_CPP