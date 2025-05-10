// sora_gfx.cpp

#ifndef SORA_GFX_CPP
#define SORA_GFX_CPP

//~ implementation

//- handle functions
function b8 
gfx_handle_equals(gfx_handle_t a, gfx_handle_t b) {
	return (a.data[0] == b.data[0]);
}

//- pipeline functions

function gfx_pipeline_t 
gfx_pipeline_create() {
	gfx_pipeline_t result = { 0 };
	result.fill_mode = gfx_fill_solid;
	result.cull_mode = gfx_cull_back;
	result.topology = gfx_topology_tris;
	result.filter_mode = gfx_filter_linear;
	result.wrap_mode = gfx_wrap_clamp;
	result.depth_mode = gfx_depth;
	result.viewport = rect(0.0f, 0.0f, 0.0f, 0.0f);
	result.scissor = rect(0.0f, 0.0f, 0.0f, 0.0f);
	return result;
}

function b8
gfx_pipeline_equals(gfx_pipeline_t a, gfx_pipeline_t b) {
	b8 result = (
                 a.fill_mode == b.fill_mode &&
                 a.cull_mode == b.cull_mode &&
                 a.topology == b.topology &&
                 a.filter_mode == b.filter_mode &&
                 a.wrap_mode == b.wrap_mode &&
                 a.depth_mode == b.depth_mode &&
                 rect_equals(a.viewport, b.viewport) &&
                 rect_equals(a.scissor, b.scissor)
                 );
    
	return result;
}

//- enum helper functions

function b8
gfx_texture_format_is_depth(gfx_texture_format format) {
	b8 result = false;
	switch (format) {
		case gfx_texture_format_d24s8:
		case gfx_texture_format_d32: {
			result = true;
			break;
		}
	}
	return result;
}

//- resource functions 

function gfx_handle_t 
gfx_texture_load(str_t filepath, gfx_texture_flags flags) {
    
    // TODO: save off the binary texture and load that first. or use qoi
    
    // load file
	i32 width = 0;
	i32 height = 0;
	i32 bpp = 0;
    
	stbi_set_flip_vertically_on_load(1);
	unsigned char* buffer = stbi_load((char*)filepath.data, &width, &height, &bpp, 4);
    
    //fill description
    gfx_texture_desc_t desc = { 0 };
	desc.name = str_get_file_name(filepath);
	desc.size = uvec2(width, height);
	desc.format = gfx_texture_format_rgba8;
	desc.flags = flags;
    desc.type = gfx_texture_type_2d;
	desc.sample_count = 1;
	desc.usage = gfx_usage_dynamic;
	
    
    //create and return texture
    gfx_handle_t handle = gfx_texture_create_ex(desc, buffer);
    
    stbi_image_free(buffer);
    
	return handle;
}

function gfx_handle_t 
gfx_shader_load(str_t filepath, gfx_shader_flags flags) {
    
    temp_t scratch = scratch_begin();
    
    str_t name = str_get_file_name(filepath);
    
    gfx_shader_desc_t desc = {
        name, filepath, flags, 0
    };
    gfx_handle_t shader = gfx_shader_create_ex(desc);
    
    str_t data = os_file_read_all(scratch.arena, filepath);
    gfx_shader_compile(shader, data);
    
    /*str_t filepath_no_ext = str_substr(filepath, 0, str_find_substr(filepath, str("."), 0, str_match_flag_find_last));
                str_t filepath_hlsl = str_format(scratch.arena, "%.*s.hlsl", filepath_no_ext.size, filepath_no_ext.data);
                str_t filepath_binary = str_format(scratch.arena, "%.*s.bin", filepath_no_ext.size, filepath_no_ext.data);
                
                // get last write time
                os_file_info_t file_info =  os_file_get_info(filepath_hlsl);
                
                // create shader
                gfx_shader_desc_t desc = {
                    name, filepath, flags, file_info.last_write_time
                };
                gfx_handle_t shader = gfx_shader_create_ex(desc);
                
                
                os_file_info_t file_binary_info = os_file_get_info(filepath_binary);
                if (os_file_exists(filepath_binary) && file_binary_info.last_write_time == file_info.last_write_time) {
                    str_t data = os_file_read_all(scratch.arena, filepath_binary);
                    gfx_shader_set_binary(shader, data.data, data.size);
                } else {
                    str_t data = os_file_read_all(scratch.arena, filepath);
                    gfx_shader_compile(shader, data);
                    
                    void* out_data = nullptr;
                    u32 out_size = 0;
                    
                    gfx_shader_get_binary(shader, &out_data, &out_size);
                    
                    // write to binary file
                    os_handle_t handle = os_file_open(filepath_binary, os_file_access_flag_write);
                    os_file_write(handle, 0, out_data, out_size);
                    os_file_close(handle);
                }
                */
    
    scratch_end(scratch);
    
    return shader;
}

function gfx_handle_t
gfx_mesh_load(arena_t* arena, str_t filepath) {
    temp_t scratch = scratch_begin();
    
    // create resource
    gfx_mesh_t* mesh = (gfx_mesh_t*)arena_alloc(arena, sizeof(gfx_mesh_t)); 
    
    str_t name = str_get_file_name(filepath);
    str_t filepath_no_ext = str_substr(filepath, 0, str_find_substr(filepath, str("."), 0, str_match_flag_find_last));
    str_t filepath_obj = str_format(scratch.arena, "%.*s.obj", filepath_no_ext.size, filepath_no_ext.data);
    str_t filepath_binary = str_format(scratch.arena, "%.*s.bin", filepath_no_ext.size, filepath_no_ext.data);
    
    if (os_file_exists(filepath_binary)) {
        _gfx_mesh_load_from_binary(arena, mesh, filepath_binary);
    } else {
        _gfx_mesh_load_from_obj(arena, mesh, filepath_obj);
        _gfx_mesh_write_to_binary(mesh, filepath_binary);
    }
    
    scratch_end(scratch);
    
    gfx_handle_t handle = {(u64)mesh};
    
    return handle;
}

function u32 
gfx_mesh_get_vertex_count(gfx_handle_t mesh_handle) {
    gfx_mesh_t* mesh = (gfx_mesh_t*)(mesh_handle.data[0]);
    return mesh->vertex_count;
}

function u32 
gfx_mesh_get_index_count(gfx_handle_t mesh_handle) {
    gfx_mesh_t* mesh = (gfx_mesh_t*)(mesh_handle.data[0]);
    return mesh->index_count;
}

function gfx_vertex_t* 
gfx_mesh_get_vertices(gfx_handle_t mesh_handle) {
    gfx_mesh_t* mesh = (gfx_mesh_t*)(mesh_handle.data[0]);
    return mesh->vertices;
}

function i32* 
gfx_mesh_get_indices(gfx_handle_t mesh_handle) {
    gfx_mesh_t* mesh = (gfx_mesh_t*)(mesh_handle.data[0]);
    return mesh->indices;
}




//- camera functions

function gfx_handle_t
gfx_camera_create(arena_t* arena, gfx_camera_mode mode, f32 fov, f32 z_near, f32 z_far) {
    
    gfx_camera_t* camera = (gfx_camera_t*)arena_alloc(arena, sizeof(gfx_camera_t));
    
    camera->mode = mode;
    camera->target_orientation = quat(0.0f, -1.0f, 0.0f, 0.0);
    camera->orientation = quat(0.0f, -1.0f, 0.0f, 0.0);
    
    camera->target_fov = fov;
    camera->fov = fov;
    camera->z_near = z_near;
    camera->z_far = z_far;
    camera->speed = 1.0f;
    
    gfx_handle_t handle = {(u64)camera};
    
    return handle;
}


function void 
gfx_camera_free_mode_input(gfx_handle_t camera_handle, os_handle_t window) {
    
    gfx_camera_t* camera = (gfx_camera_t*)(camera_handle.data[0]);
    
    // get delta time
    f32 dt = os_window_get_delta_time(window);
    
    // get input
    f32 forward_input = 0.0f;
    f32 right_input = 0.0f;
    f32 up_input = 0.0f;
    f32 roll_input = 0.0f;
    f32 pitch_input = 0.0f;
    f32 yaw_input = 0.0f;
    f32 target_speed = 0.5f;
    f32 speed = 0.5f;
    
    // only get input if window is focused
    if (os_window_is_active(window)) {
        forward_input = (f32)(os_key_is_down(os_key_W) - os_key_is_down(os_key_S));
        right_input = (f32)(os_key_is_down(os_key_D) - os_key_is_down(os_key_A));
        up_input = (f32)(os_key_is_down(os_key_space) - os_key_is_down(os_key_ctrl));
        roll_input = (f32)(os_key_is_down(os_key_E) - os_key_is_down(os_key_Q));
        speed = os_key_is_down(os_key_shift) ? 10.0f : 2.5f;
    }
    
    persist vec2_t mouse_start;
    
    // mouse input
    if (os_mouse_press(window, os_mouse_button_right)) {
        mouse_start =  os_window_get_cursor_pos(window);
        os_set_cursor(os_cursor_null);
    }
    
    if (os_mouse_release(window, os_mouse_button_right)) {
        os_set_cursor(os_cursor_pointer);
    }
    
    if (os_mouse_is_down(os_mouse_button_right) && os_window_is_active(window)) {
        vec2_t mouse_pos = os_window_get_cursor_pos(window);
        vec2_t delta = vec2_sub(mouse_start, mouse_pos);
        os_window_set_cursor_pos(window, mouse_start);
        yaw_input = delta.x;
        pitch_input = delta.y;
    }
    
    if (os_key_is_down(os_key_shift)) {
        camera->target_speed = 5.0f;
    } else {
        camera->target_speed = 0.5f;
    }
    
    // fov
    f32 scroll_delta = os_mouse_scroll(window);
    camera->target_fov -= scroll_delta * 1.5f;
    camera->target_fov = clamp(camera->target_fov, 1.0f, 160.0f);
    camera->fov = lerp(camera->fov, camera->target_fov, 30.0f * dt);
    camera->speed = lerp(camera->speed, camera->target_speed, 15.0f * dt);
    
    camera->translational_input = vec3(forward_input, right_input, up_input);
    camera->rotational_input = vec3(pitch_input, yaw_input, roll_input);
    
}

function void 
gfx_camera_update(gfx_handle_t camera_handle, rect_t viewport, f32 dt) {
    
    gfx_camera_t* camera = (gfx_camera_t*)(camera_handle.data[0]);
    
    
    f32 pitch_input = camera->rotational_input.x;
    f32 yaw_input = camera->rotational_input.y;
    f32 roll_input = camera->rotational_input.z;
    
    f32 forward_input = camera->translational_input.x;
    f32 right_input = camera->translational_input.y;
    f32 up_input = camera->translational_input.z;
    
    // clamp input
    vec3_t euler_angle = quat_to_euler_angle(camera->target_orientation);
    
    if (camera->mode & gfx_camera_mode_clamp_pitch) {
        pitch_input = max(degrees(camera->min_pitch - euler_angle.x), pitch_input);
        pitch_input = min(degrees(camera->max_pitch - euler_angle.x), pitch_input);
    }
    
    if (camera->mode & gfx_camera_mode_clamp_yaw) {
        yaw_input = max(degrees(camera->min_yaw - euler_angle.y), yaw_input);
        yaw_input = min(degrees(camera->max_yaw - euler_angle.y), yaw_input);
    }
    
    if (camera->mode & gfx_camera_mode_clamp_roll) {
        roll_input = max(degrees(camera->min_roll - euler_angle.z), roll_input);
        roll_input = min(degrees(camera->max_roll - euler_angle.z), roll_input);
    }
    
    const f32 sensitivity = 1.0f;
    f32 zoom_adjustment = (camera->fov / 160.0f);
    quat_t pitch = quat_from_axis_angle({ 1.0f, 0.0f, 0.0f }, zoom_adjustment * sensitivity * pitch_input * dt);
    quat_t yaw = quat_from_axis_angle({ 0.0f, 1.0f, 0.0f }, zoom_adjustment * sensitivity * yaw_input * dt);
    quat_t roll = quat_from_axis_angle({ 0.0f, 0.0f, 1.0f }, 2.5f * sensitivity * roll_input * dt);
    
    // orientation
    if (camera->mode & gfx_camera_mode_disable_roll) {
        camera->target_orientation = quat_mul(pitch, camera->target_orientation);
        camera->target_orientation = quat_mul(camera->target_orientation, yaw);
    } else {
        camera->target_orientation = quat_mul(pitch, camera->target_orientation);
        camera->target_orientation = quat_mul(yaw, camera->target_orientation);
        camera->target_orientation = quat_mul(roll, camera->target_orientation);
    }
    
    // smooth orientation
    camera->orientation = quat_slerp(camera->orientation, camera->target_orientation, 30.0f * dt);
    
    // translate
    vec3_t translation = vec3_mul(vec3_normalize(vec3(right_input, up_input, forward_input)), dt * camera->speed);
    vec3_t rotated_translation = vec3_rotate(translation, camera->orientation);
    camera->target_position = vec3_add(camera->target_position, rotated_translation);
    camera->position = vec3_lerp(camera->position, camera->target_position, 60.0f * dt);
    
    // update constants
    vec2_t size = vec2(viewport.x1 - viewport.x0, viewport.y1 - viewport.y0);
    
    camera->constants.view = mat4_mul(mat4_from_quat(camera->orientation), mat4_translate(vec3_negate(camera->position)));
    camera->constants.inv_view = mat4_inverse(camera->constants.view);
    camera->constants.projection = mat4_perspective(camera->fov, size.x / size.y, camera->z_near, camera->z_far);
    camera->constants.inv_projection = mat4_inverse(camera->constants.projection);
    camera->constants.view_projection = mat4_mul(camera->constants.projection, camera->constants.view);
    camera->constants.camera_position = camera->position;
    
    // reset input
    camera->translational_input = vec3(0.0f);
    camera->rotational_input = vec3(0.0f);
    
}

function gfx_camera_constants_t* 
gfx_camera_get_constants(gfx_handle_t camera_handle) {
    gfx_camera_t* camera = (gfx_camera_t*)(camera_handle.data[0]);
    return &camera->constants;
}

function mat4_t 
gfx_camera_get_view(gfx_handle_t camera_handle) {
    gfx_camera_t* camera = (gfx_camera_t*)(camera_handle.data[0]);
    return camera->constants.view;
}
function mat4_t 
gfx_camera_get_projection(gfx_handle_t camera_handle) {
    gfx_camera_t* camera = (gfx_camera_t*)(camera_handle.data[0]);
    return camera->constants.projection;
}

//- render graph functions

function gfx_handle_t 
gfx_render_graph_create(gfx_handle_t renderer) {
    
    arena_t* arena = arena_create(megabytes(256));
    
    gfx_render_graph_t* graph = (gfx_render_graph_t*)arena_alloc(arena, sizeof(gfx_render_graph_t));
    
    graph->arena = arena;
    graph->renderer = renderer;
    
    gfx_handle_t handle = { (u64)graph };
    
    return handle;
}

function void
gfx_render_graph_release(gfx_handle_t render_graph) {
    
    gfx_render_graph_t* graph = (gfx_render_graph_t*)(render_graph.data[0]);
    
    arena_release(graph->arena);
    
}

function void
gfx_render_graph_compile(gfx_handle_t render_graph) {
    
    gfx_render_graph_t* graph = (gfx_render_graph_t*)(render_graph.data[0]);
    
    // clear execute arena
    temp_t scratch = scratch_begin();
    
    // topological sort
    
    // count in degrees of all passes
    for (gfx_render_pass_t* pass = graph->pass_first; pass != nullptr; pass = pass->next) {
        pass->in_degree = 0;
        for (gfx_render_port_t* port = pass->inputs.first; port != nullptr; port = port->next) {
            if (port->connected_port != nullptr) {
                pass->in_degree++;
            }
        }
    }
    
    // create queue
    gfx_render_pass_ts_queue_t* queue_first = nullptr;
    gfx_render_pass_ts_queue_t* queue_last = nullptr;
    for (gfx_render_pass_t* pass = graph->pass_first; pass != nullptr; pass = pass->next) {
        if (pass->in_degree == 0) {
            // add to queue if in_degree is 0
            gfx_render_pass_ts_queue_t* queue_node = (gfx_render_pass_ts_queue_t*)arena_alloc(scratch.arena, sizeof(gfx_render_pass_ts_queue_t));
            queue_node->pass = pass;
            queue_push(queue_first, queue_last, queue_node);
        }
    }
    
    while (queue_first != nullptr) {
        
        // grab the top of the queue
        gfx_render_pass_ts_queue_t* queue_top = queue_first;
        queue_pop(queue_first, queue_last);
        gfx_render_pass_t* pass = queue_top->pass;
        
        // add pass to execute list
        dll_push_back_np(graph->execute_first, graph->execute_last, pass, execute_next, execute_prev);
        
        // update in degree for connected passess
        for (gfx_render_port_t* port = pass->outputs.first; port != nullptr; port = port->next) {
            if (port->connected_port != nullptr) {
                gfx_render_pass_t* connected_pass = port->connected_port->pass;
                if (connected_pass != nullptr) {
                    // update in degree
                    connected_pass->in_degree--;
                    
                    // add to queue if in degree is 0
                    if (connected_pass->in_degree == 0) {
                        gfx_render_pass_ts_queue_t* queue_node = (gfx_render_pass_ts_queue_t*)arena_alloc(scratch.arena, sizeof(gfx_render_pass_ts_queue_t));
                        queue_node->pass = connected_pass;
                        queue_push(queue_first, queue_last, queue_node);
                    }
                }
            }
        }
    }
    
    scratch_end(scratch);
    
    // invoke init functions
    for (gfx_render_pass_t* pass = graph->execute_first; pass != nullptr; pass = pass->execute_next) {
        if (pass->init_func != nullptr) {
            gfx_handle_t handle = { (u64)pass };
            pass->init_func(handle);
        }
    }
    
    // pass resouces from outputs to inputs 
    for (gfx_render_pass_t* pass = graph->execute_first; pass != nullptr; pass = pass->execute_next) {
        // set outputs resources 
        for (gfx_render_port_t* port = pass->outputs.first; port != nullptr; port = port->next) {
            if (port->connected_port != nullptr) {
                port->connected_port->resource = port->resource;
            }
        }
    }
    
    
}

function void
gfx_render_graph_execute(gfx_handle_t render_graph) {
    
    gfx_render_graph_t* graph = (gfx_render_graph_t*)(render_graph.data[0]);
    
    // invoke execute functions
    for (gfx_render_pass_t* pass = graph->execute_first; pass != nullptr; pass = pass->execute_next) {
        if (pass->execute_func != nullptr) {
            gfx_handle_t handle = { (u64)pass };
            pass->execute_func(handle);
        }
    }
    
}



//- render pass functions

function gfx_handle_t
gfx_render_pass_create(gfx_handle_t render_graph, gfx_render_pass_init_func init_func, gfx_render_pass_execute_func execute_func) {
    
    gfx_render_graph_t* graph = (gfx_render_graph_t*)(render_graph.data[0]);
    
    gfx_render_pass_t* pass = graph->pass_free;
    if (pass != nullptr) {
        stack_pop(graph->pass_free);
    } else {
        pass = (gfx_render_pass_t*)arena_alloc(graph->arena, sizeof(gfx_render_pass_t));
    }
    memset(pass, 0, sizeof(gfx_render_pass_t));
    
    // add to global pass list
    dll_push_back(graph->pass_first, graph->pass_last, pass);
    
    // fill struct
    pass->graph = graph;
    pass->init_func =  init_func;
    pass->execute_func = execute_func;
    
    gfx_handle_t handle = { (u64)pass };
    
    return handle;
}

function void
gfx_render_pass_release(gfx_handle_t render_pass) {
    gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
    gfx_render_graph_t* graph = pass->graph;
    
    dll_remove(graph->pass_first, graph->pass_last, pass);
    stack_push(graph->pass_free, pass);
}

function void
gfx_render_pass_add_input(gfx_handle_t render_pass, str_t input_label) {
    gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
    gfx_render_graph_t* graph = pass->graph;
    
    // allocate port
    gfx_render_port_t* port = _gfx_render_port_alloc(graph);
    dll_push_back(pass->inputs.first, pass->inputs.last, port);
    
    // fill struct
    port->pass = pass;
    port->label = input_label;
    
}

function void
gfx_render_pass_add_output(gfx_handle_t render_pass, str_t output_label) {
    gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
    gfx_render_graph_t* graph = pass->graph;
    
    // allocate port
    gfx_render_port_t* port = _gfx_render_port_alloc(graph);
    dll_push_back(pass->outputs.first, pass->outputs.last, port);
    
    // fill port struct
    port->pass = pass;
    port->label = output_label;
    
}

function void
gfx_render_pass_link(gfx_handle_t src, str_t output_label, gfx_handle_t dst, str_t input_label) {
    
    gfx_render_pass_t* src_pass = (gfx_render_pass_t*)(src.data[0]);
    gfx_render_pass_t* dst_pass = (gfx_render_pass_t*)(dst.data[0]);
    gfx_render_graph_t* graph = src_pass->graph;
    
    // find input and output port
    gfx_render_port_t* input_port = _gfx_render_port_find(dst_pass->inputs, input_label);
    gfx_render_port_t* output_port = _gfx_render_port_find(src_pass->outputs, output_label);
    
    if (input_port != nullptr && output_port != nullptr) {
        input_port->resource = output_port->resource;
    }
    
    // add connection
    gfx_render_connection_t* connection = _gfx_render_connection_alloc(graph);
    connection->from = output_port;
    connection->to = input_port;
    dll_push_back(graph->connection_first, graph->connection_last, connection);
    
    // set connected ports
    input_port->connected_port = output_port;
    output_port->connected_port = input_port;
    
}

function void
gfx_render_pass_unlink(gfx_render_port_t* from, gfx_render_port_t* to) {
    // TODO:
}

function void
gfx_render_pass_set_output(gfx_handle_t render_pass, str_t output_label, gfx_handle_t resource) {
    gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
    gfx_render_graph_t* graph = pass->graph;
    
    // find the output
    gfx_render_port_t* output_port = _gfx_render_port_find(pass->outputs, output_label);
    
    // set the resource
    if (output_port != nullptr) {
        output_port->resource = resource;
    }
    
}

function gfx_handle_t 
gfx_render_pass_get_input(gfx_handle_t render_pass, str_t input_label) {
    gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
    gfx_render_graph_t* graph = pass->graph;
    gfx_handle_t result = { 0 };
    
    // find the input
    gfx_render_port_t* input_port = _gfx_render_port_find(pass->inputs, input_label);
    
    if (input_port != nullptr) {
        result = input_port->resource;
    }
    
    return result;
}

function gfx_handle_t 
gfx_render_pass_get_output(gfx_handle_t render_pass, str_t output_label) {
    gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
    gfx_render_graph_t* graph = pass->graph;
    gfx_handle_t result = { 0 };
    
    // find the output
    gfx_render_port_t* output_port = _gfx_render_port_find(pass->outputs, output_label);
    
    if (output_port != nullptr) {
        result = output_port->resource;
    }
    
    return result;
}


//- internal functions

function gfx_render_port_t*
_gfx_render_port_alloc(gfx_render_graph_t* render_graph) {
    
    gfx_render_port_t* port = render_graph->port_free;
    if (port != nullptr) {
        stack_pop(render_graph->port_free);
    } else {
        port = (gfx_render_port_t*)arena_alloc(render_graph->arena, sizeof(gfx_render_port_t));
    }
    memset(port, 0, sizeof(gfx_render_port_t));
    
    return port;
}

function gfx_render_port_t* 
_gfx_render_port_find(gfx_render_port_list_t list, str_t label) {
    
    gfx_render_port_t* result = nullptr;
    for (gfx_render_port_t* port = list.first; port != nullptr; port = port->next) {
        if (str_match(port->label, label)) {
            result = port;
            break;
        }
    }
    
    return result;
}

function gfx_render_connection_t* 
_gfx_render_connection_alloc(gfx_render_graph_t* render_graph) {
    gfx_render_connection_t* connection = render_graph->connection_free;
    if (connection != nullptr) {
        stack_pop(render_graph->connection_free);
    } else {
        connection = (gfx_render_connection_t*)arena_alloc(render_graph->arena, sizeof(gfx_render_connection_t));
    }
    memset(connection, 0, sizeof(gfx_render_connection_t));
    
    return connection;
}

function gfx_render_connection_t* 
_gfx_render_connection_find(gfx_render_graph_t* render_graph, gfx_render_port_t* from, gfx_render_port_t* to) {
    
    gfx_render_connection_t* result = nullptr;
    
    for (gfx_render_connection_t* connection = render_graph->connection_first; connection != nullptr; connection = connection->next) {
        if (connection->from == from && connection->to == to) {
            result = connection;
            break;
        }
    }
    
    return result;
}

function void 
_gfx_mesh_load_from_obj(arena_t* arena, gfx_mesh_t* mesh, str_t filepath) {
    temp_t scratch = scratch_begin();
    
    // load data
    str_t data = os_file_read_all(scratch.arena, filepath);
    
    // count the vertex positions, texcoords, normals, and faces.
    u32 position_count = 0;
    u32 texcoord_count = 0;
    u32 normal_count = 0;
    u32 face_count = 0;
    
    for (i32 i = 0; i <= data.size; i++) {
        u8 current_char = data.data[i];
        u8 next_char = data.data[i + 1]; 
        
        if (current_char == 'v') {
            if (next_char == ' ') {
                position_count++;
            } else if (next_char == 't') {
                texcoord_count++;
            } else if (next_char == 'n') {
                normal_count++;
            }
        } else if (current_char == 'f') {
            face_count++;
        }
    }
    
    // allocate temp arrays
    vec3_t* positions = (vec3_t*)arena_calloc(scratch.arena, sizeof(vec3_t) * position_count);
    vec2_t* texcoords = (vec2_t*)arena_calloc(scratch.arena, sizeof(vec2_t) * texcoord_count);
    vec3_t* normals = (vec3_t*)arena_calloc(scratch.arena, sizeof(vec3_t) * normal_count);
    gfx_face_t* faces = (gfx_face_t*)arena_calloc(scratch.arena, sizeof(gfx_face_t) * face_count);
    u32 position_index = 0;
    u32 texcoord_index = 0;
    u32 normal_index = 0;
    u32 face_index = 0;
    
    // parse data into arrays
    str_list_t lines = str_split(scratch.arena, data, (u8*)"\n", 1);
    for (str_node_t* node = lines.first; node != nullptr; node = node->next) {
        str_t line = node->string;
        
        str_list_t tokens = str_split(scratch.arena, line, (u8*)" ", 1);
        
        // positions
        if (str_match(tokens.first->string, str("v"))) {
            f32 x = f32_from_str(str_list_get_index(tokens, 1));
            f32 y = f32_from_str(str_list_get_index(tokens, 2));
            f32 z = f32_from_str(str_list_get_index(tokens, 3));
            positions[position_index++] = vec3(x, y, z);
        }
        
        // texcoords
        if (str_match(tokens.first->string, str("vt"))) {
            f32 u = f32_from_str(str_list_get_index(tokens, 1));
            f32 v = f32_from_str(str_list_get_index(tokens, 2));
            texcoords[texcoord_index++] = vec2(u, v);
        }
        
        // normals
        if (str_match(tokens.first->string, str("vn"))) {
            f32 x = f32_from_str(str_list_get_index(tokens, 1));
            f32 y = f32_from_str(str_list_get_index(tokens, 2));
            f32 z = f32_from_str(str_list_get_index(tokens, 3));
            normals[normal_index++] = vec3(x, y, z);
        }
        
        // faces
        if (str_match(tokens.first->string, str("f"))) {
            
            // scan through each face index
            for (str_node_t* token = tokens.first->next; token != nullptr; token = token->next) {
                
                gfx_face_t* face = &faces[face_index];
                
                // split token
                str_list_t indices = str_split(scratch.arena, token->string, (u8*)"/", 1);
                
                str_t position_index_str = str_list_get_index(indices, 0);
                str_t texcoord_index_str = str_list_get_index(indices, 1);
                str_t normal_index_str = str_list_get_index(indices, 2);
                
                i32 position_index = position_index_str.size == 0 ? -1 : i32_from_str(position_index_str); 
                i32 texcoord_index = texcoord_index_str.size == 0 ? -1 : i32_from_str(texcoord_index_str); 
                i32 normal_index = normal_index_str.size == 0 ? -1 : i32_from_str(normal_index_str); 
                
                face->indices[face->index_count++] = {position_index, texcoord_index, normal_index };
            }
            
            face_index++;
        }
        
    }
    
    // create unique vertices lookup table
    
    u32 max_vertices = 0;
    for (u32 i = 0; i < face_count; i++) { max_vertices += faces[i].index_count; }
    
    gfx_vertex_lookup_t* lookup = (gfx_vertex_lookup_t*)arena_alloc(scratch.arena, sizeof(gfx_vertex_lookup_t) * max_vertices);
    
    u32 unique_vertex_count = 0;
    u32 total_indices = 0;
    
    for (u32 i = 0; i < face_count; i++) {
        gfx_face_t* face = &faces[i];
        total_indices += (face->index_count - 2) * 3;
        
        for (u32 j = 0; j < face->index_count; j++) {
            gfx_face_index_t* face_index = &face->indices[j];
            
            b8 found = false;
            for (u32 k = 0; k < unique_vertex_count; k++) {
                if (lookup[k].position_index == face_index->position_index &&
                    lookup[k].texcoord_index == face_index->texcoord_index &&
                    lookup[k].normal_index == face_index->normal_index) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                lookup[unique_vertex_count].position_index = face_index->position_index;
                lookup[unique_vertex_count].texcoord_index = face_index->texcoord_index;
                lookup[unique_vertex_count].normal_index = face_index->normal_index;
                lookup[unique_vertex_count].new_index = unique_vertex_count;
                unique_vertex_count++;
            }
        }
    }
    
    mesh->vertex_count = unique_vertex_count;
    mesh->vertices = (gfx_vertex_t*)arena_calloc(arena, sizeof(gfx_vertex_t) * mesh->vertex_count);
    
    mesh->index_count = total_indices;
    mesh->indices = (i32*)arena_calloc(arena, sizeof(i32) * mesh->index_count);
    
    // fill vertex data
    for (u32 i = 0; i < unique_vertex_count; i++) {
        gfx_vertex_t* vertex = &mesh->vertices[i];
        
        vertex->position = positions[lookup[i].position_index - 1];
        vertex->texcoord = texcoords[lookup[i].texcoord_index - 1];
        vertex->normal = normals[lookup[i].normal_index - 1];
        vertex->color = vec4(1.0f);
    }
    
    // fill index data
    u32 index_offset = 0;
    
    for (u32 i = 0; i < face_count; i++) {
        gfx_face_t* face = &faces[i];
        
        i32 face_indices[4];
        
        for (u32 j = 0; j < face->index_count; j++) {
            gfx_face_index_t* face_index = &face->indices[j];
            
            for (u32 k = 0; k < unique_vertex_count; k++) {
                if (lookup[k].position_index == face_index->position_index &&
                    lookup[k].texcoord_index == face_index->texcoord_index &&
                    lookup[k].normal_index == face_index->normal_index) {
                    face_indices[j] = lookup[k].new_index;
                    break;
                }
            }
        }
        
        for (u32 j = 0; j < face->index_count - 2; j++) {
            mesh->indices[index_offset++] = face_indices[0];
            mesh->indices[index_offset++] = face_indices[j + 2];
            mesh->indices[index_offset++] = face_indices[j + 1];
        }
    }
    
    scratch_end(scratch);
}

function void 
_gfx_mesh_load_from_binary(arena_t* arena, gfx_mesh_t* mesh, str_t filepath) {
    
    os_handle_t file = os_file_open(filepath);
    
    u32 read_pos = 0;
    
    // read header
    gfx_mesh_file_header_t header;
    read_pos += os_file_read(file, read_pos, &header, sizeof(gfx_mesh_file_header_t));
    
    // validate
    if (header.magic != sora_gfx_mesh_magic_number || header.version > sora_gfx_mesh_file_version) {
        os_file_close(file);
        return;
    }
    
    // allocate data
    mesh->vertex_count = header.vertex_count;
    mesh->vertices = (gfx_vertex_t*)arena_alloc(arena, sizeof(gfx_vertex_t) * mesh->vertex_count);
    mesh->index_count = header.index_count;
    mesh->indices = (i32*)arena_alloc(arena, sizeof(i32) * mesh->index_count);
    
    // read data
    read_pos += os_file_read(file, read_pos, mesh->vertices, sizeof(gfx_vertex_t) * mesh->vertex_count);
    read_pos += os_file_read(file, read_pos, mesh->indices, sizeof(i32) * mesh->index_count);
    
    os_file_close(file);
    
}

function void 
_gfx_mesh_write_to_obj(gfx_mesh_t* mesh, str_t filepath) {
    // TODO:
}

function void 
_gfx_mesh_write_to_binary(gfx_mesh_t* mesh, str_t filepath) {
    os_handle_t file = os_file_open(filepath, os_file_access_flag_write);
    u32 write_pos = 0;
    
    // write header
    gfx_mesh_file_header_t header = {
        sora_gfx_mesh_magic_number,
        sora_gfx_mesh_file_version,
        mesh->vertex_count,
        mesh->index_count,
        0
    };
    
    write_pos += os_file_write(file, write_pos, &header, sizeof(gfx_mesh_file_header_t));
    
    // write vertex data
    write_pos += os_file_write(file, write_pos, mesh->vertices, sizeof(gfx_vertex_t) * mesh->vertex_count);
    
    // write index data
    write_pos += os_file_write(file, write_pos, mesh->indices, sizeof(i32) * mesh->index_count);
    
    os_file_close(file);
}



//- per backend includes

#ifdef GFX_BACKEND_D3D11
#    include "backends/gfx/sora_gfx_d3d11.cpp"
#elif GFX_BACKEND_D3D12
#    include "backends/gfx/sora_gfx_d3d12.cpp"
#elif GFX_BACKEND_OPENGL
#    include "backends/gfx/sora_gfx_opengl.cpp"
#elif GFX_BACKEND_METAL
#    include "backends/gfx/sora_gfx_metal.cpp"
#elif GFX_BACKEND_VULKAN
#    include "backends/gfx/sora_gfx_vulkan.cpp"
#endif

#endif // SORA_GFX_CPP