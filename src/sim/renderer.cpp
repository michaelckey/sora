// renderer.cpp

#ifndef RENDERER_CPP
#define RENDERER_CPP

//- implementation 

function void
renderer_init(gfx_handle_t context) {
    
    renderer.arena = arena_create(gigabytes(2));
    renderer.context = context;
    
    // create render target
    gfx_render_target_desc_t render_target_desc;
    render_target_desc.colorbuffer_format = gfx_texture_format_rgba8;
    render_target_desc.depthbuffer_format = gfx_texture_format_d32;
    render_target_desc.size = gfx_context_get_size(context);
    render_target_desc.sample_count = 8;
    renderer.render_target = gfx_render_target_create_ex(render_target_desc);
    renderer.render_target_texture = gfx_render_target_get_texture(renderer.render_target);
    
    // point renderer
    renderer.point_instance_buffer = gfx_buffer_create(gfx_buffer_type_vertex, renderer_max_batch_size);
    renderer.point_vertex_shader = gfx_shader_load(str("res/shaders/shader_point.hlsl"), gfx_shader_flag_vertex | gfx_shader_flag_per_instance);
    renderer.point_pixel_shader = gfx_shader_load(str("res/shaders/shader_point.hlsl"), gfx_shader_flag_pixel);
    renderer.point_instance_data = (point_instance_t*)arena_alloc(renderer.arena, renderer_max_batch_size);
    renderer.point_instance_count = 0;
    
    // line renderer
    renderer.line_vertex_buffer = gfx_buffer_create(gfx_buffer_type_vertex, renderer_max_batch_size);
    renderer.line_vertex_shader = gfx_shader_load(str("res/shaders/shader_line.hlsl"), gfx_shader_flag_vertex);
    renderer.line_pixel_shader = gfx_shader_load(str("res/shaders/shader_line.hlsl"), gfx_shader_flag_pixel);
    renderer.line_vertex_data = (line_vertex_t*)arena_alloc(renderer.arena, renderer_max_batch_size);
    renderer.line_vertex_count = 0;
    
    // constant buffer
    renderer.constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, kilobytes(4));
    
}

function void
renderer_release() {
    
    // release buffers
    gfx_buffer_release(renderer.constant_buffer);
    gfx_buffer_release(renderer.point_instance_buffer);
    gfx_buffer_release(renderer.line_vertex_buffer);
    
    // release shaders
    gfx_shader_release(renderer.point_vertex_shader);
    gfx_shader_release(renderer.point_pixel_shader);
    gfx_shader_release(renderer.line_vertex_shader);
    gfx_shader_release(renderer.line_pixel_shader);
    
    // release arena
    arena_release(renderer.arena);
    
}

function void
renderer_begin(camera_t* camera) {
    
    // clear the counts
    renderer.point_instance_count = 0;
    renderer.line_vertex_count = 0;
    
    // get updated size
    uvec2_t context_size = gfx_context_get_size(renderer.context);
    renderer.viewport = rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y);
    
    // update the constant data
    gfx_buffer_fill(renderer.constant_buffer, &camera->constants, sizeof(camera_constants_t));
    
    // resize render target if needed
    uvec2_t render_target_size = gfx_render_target_get_size(renderer.render_target);
    if (!uvec2_equals(render_target_size, context_size)) {
        gfx_render_target_resize(renderer.render_target, context_size);
    }
    
    // set the render target
    gfx_set_render_target(renderer.render_target);
    gfx_render_target_clear(renderer.render_target);
    
}

function void
renderer_end() {
    renderer_flush_points();
    renderer_flush_lines();
}

function void 
renderer_flush_points() {
    if (renderer.point_instance_count != 0) {
        
        // fill the buffer
        gfx_buffer_fill(renderer.point_instance_buffer, renderer.point_instance_data, sizeof(point_instance_t) * renderer.point_instance_count);
        
        // set shaders
        gfx_set_shader(renderer.point_vertex_shader);
        gfx_set_shader(renderer.point_pixel_shader);
        
        // set buffers
        gfx_set_buffer(renderer.point_instance_buffer, 0, sizeof(point_instance_t));
        gfx_set_buffer(renderer.constant_buffer, 0);
        
        // set state
        gfx_set_viewport(renderer.viewport);
        gfx_set_scissor(renderer.viewport);
        gfx_set_rasterizer(gfx_fill_solid, gfx_cull_none);
        gfx_set_topology(gfx_topology_tri_strip);
        gfx_set_sampler(gfx_filter_nearest, gfx_wrap_repeat, 0);
        gfx_set_depth_mode(gfx_depth);
        
        // draw
        gfx_draw_instanced(4, renderer.point_instance_count);
        
        renderer.point_instance_count = 0;
    }
}

function void
renderer_flush_lines() {
    if (renderer.line_vertex_count != 0.0f) {
        
        // fill the buffer
        gfx_buffer_fill(renderer.line_vertex_buffer, renderer.line_vertex_data, sizeof(line_vertex_t) * renderer.line_vertex_count);
        
        // set shaders
        gfx_set_shader(renderer.line_vertex_shader);
        gfx_set_shader(renderer.line_pixel_shader);
        
        // set buffers
        gfx_set_buffer(renderer.line_vertex_buffer, 0, sizeof(line_vertex_t));
        gfx_set_buffer(renderer.constant_buffer, 0);
        
        // set state
        gfx_set_viewport(renderer.viewport);
        gfx_set_scissor(renderer.viewport);
        gfx_set_rasterizer(gfx_fill_solid, gfx_cull_back);
        gfx_set_topology(gfx_topology_lines);
        gfx_set_sampler(gfx_filter_nearest, gfx_wrap_repeat, 0);
        gfx_set_depth_mode(gfx_depth);
        
        // draw
        gfx_draw(renderer.line_vertex_count, 0);
        
        renderer.line_vertex_count= 0;
        
    }
}

function void
renderer_draw_point(vec3_t position, u32 color) {
    
    // flush the current batch if needed
    if ((renderer.point_instance_count + 1) * sizeof(point_instance_t) >= renderer_max_batch_size) {
        renderer_flush_points();
    }
    
    point_instance_t* instance = &renderer.point_instance_data[renderer.point_instance_count++];
    instance->position = position;;
    instance->color = color;
    
}

function void 
renderer_draw_point_bulk(vec3_t* positions, u32* colors, u32 count) {
    
    // flush the current batch if needed
    if ((renderer.point_instance_count + count) * sizeof(point_instance_t) >= renderer_max_batch_size) {
        renderer_flush_points();
    }
    
    u32 start_index = renderer.point_instance_count;
    for (u32 i = 0; i < count; i++) {
        renderer.point_instance_data[start_index + i].position = positions[i];
        renderer.point_instance_data[start_index + i].color = colors[i];
    }
    renderer.point_instance_count += count;
    
}

function void 
renderer_draw_line(vec3_t p0, vec3_t p1, u32 color) {
    
    // flush the current batch if needed
    if ((renderer.line_vertex_count + 1) * sizeof(line_vertex_t) >= renderer_max_batch_size) {
        renderer_flush_lines();
    }
    
    renderer.line_vertex_data[renderer.line_vertex_count + 0].position = p0;
    renderer.line_vertex_data[renderer.line_vertex_count + 0].color = color;
    renderer.line_vertex_data[renderer.line_vertex_count + 1].position = p1;
    renderer.line_vertex_data[renderer.line_vertex_count + 1].color = color;
    renderer.line_vertex_count += 2;
    
}

function void
renderer_draw_cube(vec3_t from, vec3_t to, u32 color) {
    
    f32 x0 = from.x, x1 = to.x;
    f32 y0 = from.y, y1 = to.y;
    f32 z0 = from.z, z1 = to.z;
    
    vec3_t v0 = {x0, y0, z0}, v1 = {x1, y0, z0};
    vec3_t v2 = {x1, y1, z0}, v3 = {x0, y1, z0};
    vec3_t v4 = {x0, y0, z1}, v5 = {x1, y0, z1};
    vec3_t v6 = {x1, y1, z1}, v7 = {x0, y1, z1};
    
    renderer_draw_line(v0, v1, color);
    renderer_draw_line(v1, v2, color);
    renderer_draw_line(v2, v3, color);
    renderer_draw_line(v3, v0, color);
    
    renderer_draw_line(v4, v5, color);
    renderer_draw_line(v5, v6, color);
    renderer_draw_line(v6, v7, color);
    renderer_draw_line(v7, v4, color);
    
    renderer_draw_line(v0, v4, color);
    renderer_draw_line(v1, v5, color);
    renderer_draw_line(v2, v6, color);
    renderer_draw_line(v3, v7, color);
    
}




#endif // RENDERER_CPP