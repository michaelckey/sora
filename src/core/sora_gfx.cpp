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
    
#if 1
    gfx_shader_desc_t desc = {
        name, filepath, flags, 0
    };
    gfx_handle_t shader = gfx_shader_create_ex(desc);
    
    str_t data = os_file_read_all(scratch.arena, filepath);
    gfx_shader_compile(shader, data);
    
#else
    
    str_t filepath_no_ext = str_substr(filepath, 0, str_find_substr(filepath, str("."), 0, str_match_flag_find_last));
    str_t filepath_hlsl = str_format(scratch.arena, "%.*s.hlsl", filepath_no_ext.size, filepath_no_ext.data);
    str_t filepath_binary = str_format(scratch.arena, "%.*s.bin", filepath_no_ext.size, filepath_no_ext.data);
    
    // create shader
    gfx_shader_desc_t desc = {
        name, filepath, flags, 0
    };
    gfx_handle_t shader = gfx_shader_create_ex(desc);
    
    os_file_info_t file_binary_info = os_file_get_info(filepath_binary);
    if (os_file_exists(filepath_binary)) {
        str_t data = os_file_read_all(scratch.arena, filepath_binary);
        gfx_shader_set_binary(shader, data.data, data.size);
        printf("loaded from binary\n");
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
    
#endif
    
    scratch_end(scratch);
    
    return shader;
}


//- render graph functions
//
//function gfx_handle_t 
//gfx_render_graph_create(gfx_handle_t renderer) {
//
//arena_t* arena = arena_create(megabytes(256));
//
//gfx_render_graph_t* graph = (gfx_render_graph_t*)arena_alloc(arena, sizeof(gfx_render_graph_t));
//
//graph->arena = arena;
//graph->renderer = renderer;
//
//gfx_handle_t handle = { (u64)graph };
//
//return handle;
//}
//
//function void
//gfx_render_graph_release(gfx_handle_t render_graph) {
//
//gfx_render_graph_t* graph = (gfx_render_graph_t*)(render_graph.data[0]);
//
//arena_release(graph->arena);
//
//}
//
//function void
//gfx_render_graph_compile(gfx_handle_t render_graph) {
//
//gfx_render_graph_t* graph = (gfx_render_graph_t*)(render_graph.data[0]);
//
// clear execute arena
//temp_t scratch = scratch_begin();
//
// topological sort
//
// count in degrees of all passes
//for (gfx_render_pass_t* pass = graph->pass_first; pass != nullptr; pass = pass->next) {
//pass->in_degree = 0;
//for (gfx_render_port_t* port = pass->inputs.first; port != nullptr; port = port->next) {
//if (port->connected_port != nullptr) {
//pass->in_degree++;
//}
//}
//}
//
// create queue
//gfx_render_pass_ts_queue_t* queue_first = nullptr;
//gfx_render_pass_ts_queue_t* queue_last = nullptr;
//for (gfx_render_pass_t* pass = graph->pass_first; pass != nullptr; pass = pass->next) {
//if (pass->in_degree == 0) {
// add to queue if in_degree is 0
//gfx_render_pass_ts_queue_t* queue_node = (gfx_render_pass_ts_queue_t*)arena_alloc(scratch.arena, sizeof(gfx_render_pass_ts_queue_t));
//queue_node->pass = pass;
//queue_push(queue_first, queue_last, queue_node);
//}
//}
//
//while (queue_first != nullptr) {
//
// grab the top of the queue
//gfx_render_pass_ts_queue_t* queue_top = queue_first;
//queue_pop(queue_first, queue_last);
//gfx_render_pass_t* pass = queue_top->pass;
//
// add pass to execute list
//dll_push_back_np(graph->execute_first, graph->execute_last, pass, execute_next, execute_prev);
//
// update in degree for connected passess
//for (gfx_render_port_t* port = pass->outputs.first; port != nullptr; port = port->next) {
//if (port->connected_port != nullptr) {
//gfx_render_pass_t* connected_pass = port->connected_port->pass;
//if (connected_pass != nullptr) {
// update in degree
//connected_pass->in_degree--;
//
// add to queue if in degree is 0
//if (connected_pass->in_degree == 0) {
//gfx_render_pass_ts_queue_t* queue_node = (gfx_render_pass_ts_queue_t*)arena_alloc(scratch.arena, sizeof(gfx_render_pass_ts_queue_t));
//queue_node->pass = connected_pass;
//queue_push(queue_first, queue_last, queue_node);
//}
//}
//}
//}
//}
//
//scratch_end(scratch);
//
// invoke init functions
//for (gfx_render_pass_t* pass = graph->execute_first; pass != nullptr; pass = pass->execute_next) {
//if (pass->init_func != nullptr) {
//gfx_handle_t handle = { (u64)pass };
//pass->init_func(handle);
//}
//}
//
// pass resouces from outputs to inputs 
//for (gfx_render_pass_t* pass = graph->execute_first; pass != nullptr; pass = pass->execute_next) {
// set outputs resources 
//for (gfx_render_port_t* port = pass->outputs.first; port != nullptr; port = port->next) {
//if (port->connected_port != nullptr) {
//port->connected_port->resource = port->resource;
//}
//}
//}
//
//
//}
//
//function void
//gfx_render_graph_execute(gfx_handle_t render_graph) {
//
//gfx_render_graph_t* graph = (gfx_render_graph_t*)(render_graph.data[0]);
//
// invoke execute functions
//for (gfx_render_pass_t* pass = graph->execute_first; pass != nullptr; pass = pass->execute_next) {
//if (pass->execute_func != nullptr) {
//gfx_handle_t handle = { (u64)pass };
//pass->execute_func(handle);
//}
//}
//
//}
//
//
//
//- render pass functions
//
//function gfx_handle_t
//gfx_render_pass_create(gfx_handle_t render_graph, gfx_render_pass_init_func init_func, gfx_render_pass_execute_func execute_func) {
//
//gfx_render_graph_t* graph = (gfx_render_graph_t*)(render_graph.data[0]);
//
//gfx_render_pass_t* pass = graph->pass_free;
//if (pass != nullptr) {
//stack_pop(graph->pass_free);
//} else {
//pass = (gfx_render_pass_t*)arena_alloc(graph->arena, sizeof(gfx_render_pass_t));
//}
//memset(pass, 0, sizeof(gfx_render_pass_t));
//
// add to global pass list
//dll_push_back(graph->pass_first, graph->pass_last, pass);
//
// fill struct
//pass->graph = graph;
//pass->init_func =  init_func;
//pass->execute_func = execute_func;
//
//gfx_handle_t handle = { (u64)pass };
//
//return handle;
//}
//
//function void
//gfx_render_pass_release(gfx_handle_t render_pass) {
//gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
//gfx_render_graph_t* graph = pass->graph;
//
//dll_remove(graph->pass_first, graph->pass_last, pass);
//stack_push(graph->pass_free, pass);
//}
//
//function void
//gfx_render_pass_add_input(gfx_handle_t render_pass, str_t input_label) {
//gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
//gfx_render_graph_t* graph = pass->graph;
//
// allocate port
//gfx_render_port_t* port = _gfx_render_port_alloc(graph);
//dll_push_back(pass->inputs.first, pass->inputs.last, port);
//
// fill struct
//port->pass = pass;
//port->label = input_label;
//
//}
//
//function void
//gfx_render_pass_add_output(gfx_handle_t render_pass, str_t output_label) {
//gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
//gfx_render_graph_t* graph = pass->graph;
//
// allocate port
//gfx_render_port_t* port = _gfx_render_port_alloc(graph);
//dll_push_back(pass->outputs.first, pass->outputs.last, port);
//
// fill port struct
//port->pass = pass;
//port->label = output_label;
//
//}
//
//function void
//gfx_render_pass_link(gfx_handle_t src, str_t output_label, gfx_handle_t dst, str_t input_label) {
//
//gfx_render_pass_t* src_pass = (gfx_render_pass_t*)(src.data[0]);
//gfx_render_pass_t* dst_pass = (gfx_render_pass_t*)(dst.data[0]);
//gfx_render_graph_t* graph = src_pass->graph;
//
// find input and output port
//gfx_render_port_t* input_port = _gfx_render_port_find(dst_pass->inputs, input_label);
//gfx_render_port_t* output_port = _gfx_render_port_find(src_pass->outputs, output_label);
//
//if (input_port != nullptr && output_port != nullptr) {
//input_port->resource = output_port->resource;
//}
//
// add connection
//gfx_render_connection_t* connection = _gfx_render_connection_alloc(graph);
//connection->from = output_port;
//connection->to = input_port;
//dll_push_back(graph->connection_first, graph->connection_last, connection);
//
// set connected ports
//input_port->connected_port = output_port;
//output_port->connected_port = input_port;
//
//}
//
//function void
//gfx_render_pass_unlink(gfx_render_port_t* from, gfx_render_port_t* to) {
// TODO:
//}
//
//function void
//gfx_render_pass_set_output(gfx_handle_t render_pass, str_t output_label, gfx_handle_t resource) {
//gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
//gfx_render_graph_t* graph = pass->graph;
//
// find the output
//gfx_render_port_t* output_port = _gfx_render_port_find(pass->outputs, output_label);
//
// set the resource
//if (output_port != nullptr) {
//output_port->resource = resource;
//}
//
//}
//
//function gfx_handle_t 
//gfx_render_pass_get_input(gfx_handle_t render_pass, str_t input_label) {
//gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
//gfx_render_graph_t* graph = pass->graph;
//gfx_handle_t result = { 0 };
//
// find the input
//gfx_render_port_t* input_port = _gfx_render_port_find(pass->inputs, input_label);
//
//if (input_port != nullptr) {
//result = input_port->resource;
//}
//
//return result;
//}
//
//function gfx_handle_t 
//gfx_render_pass_get_output(gfx_handle_t render_pass, str_t output_label) {
//gfx_render_pass_t* pass = (gfx_render_pass_t*)(render_pass.data[0]);
//gfx_render_graph_t* graph = pass->graph;
//gfx_handle_t result = { 0 };
//
// find the output
//gfx_render_port_t* output_port = _gfx_render_port_find(pass->outputs, output_label);
//
//if (output_port != nullptr) {
//result = output_port->resource;
//}
//
//return result;
//}
//
//
//- internal functions
//
//function gfx_render_port_t*
//_gfx_render_port_alloc(gfx_render_graph_t* render_graph) {
//
//gfx_render_port_t* port = render_graph->port_free;
//if (port != nullptr) {
//stack_pop(render_graph->port_free);
//} else {
//port = (gfx_render_port_t*)arena_alloc(render_graph->arena, sizeof(gfx_render_port_t));
//}
//memset(port, 0, sizeof(gfx_render_port_t));
//
//return port;
//}
//
//function gfx_render_port_t* 
//_gfx_render_port_find(gfx_render_port_list_t list, str_t label) {
//
//gfx_render_port_t* result = nullptr;
//for (gfx_render_port_t* port = list.first; port != nullptr; port = port->next) {
//if (str_match(port->label, label)) {
//result = port;
//break;
//}
//}
//
//return result;
//}
//
//function gfx_render_connection_t* 
//_gfx_render_connection_alloc(gfx_render_graph_t* render_graph) {
//gfx_render_connection_t* connection = render_graph->connection_free;
//if (connection != nullptr) {
//stack_pop(render_graph->connection_free);
//} else {
//connection = (gfx_render_connection_t*)arena_alloc(render_graph->arena, sizeof(gfx_render_connection_t));
//}
//memset(connection, 0, sizeof(gfx_render_connection_t));
//
//return connection;
//}
//
//function gfx_render_connection_t* 
//_gfx_render_connection_find(gfx_render_graph_t* render_graph, gfx_render_port_t* from, gfx_render_port_t* to) {
//
//gfx_render_connection_t* result = nullptr;
//
//for (gfx_render_connection_t* connection = render_graph->connection_first; connection != nullptr; connection = connection->next) {
//if (connection->from == from && connection->to == to) {
//result = connection;
//break;
//}
//}
//
//return result;
//}
//
//function void 
//_gfx_mesh_load_from_obj(arena_t* arena, gfx_mesh_t* mesh, str_t filepath) {
//temp_t scratch = scratch_begin();
//
// load data
//str_t data = os_file_read_all(scratch.arena, filepath);
//
// count the vertex positions, texcoords, normals, and faces.
//u32 position_count = 0;
//u32 texcoord_count = 0;
//u32 normal_count = 0;
//u32 face_count = 0;
//
//for (i32 i = 0; i <= data.size; i++) {
//u8 current_char = data.data[i];
//u8 next_char = data.data[i + 1]; 
//
//if (current_char == 'v') {
//if (next_char == ' ') {
//position_count++;
//} else if (next_char == 't') {
//texcoord_count++;
//} else if (next_char == 'n') {
//normal_count++;
//}
//} else if (current_char == 'f') {
//face_count++;
//}
//}
//
// allocate temp arrays
//vec3_t* positions = (vec3_t*)arena_calloc(scratch.arena, sizeof(vec3_t) * position_count);
//vec2_t* texcoords = (vec2_t*)arena_calloc(scratch.arena, sizeof(vec2_t) * texcoord_count);
//vec3_t* normals = (vec3_t*)arena_calloc(scratch.arena, sizeof(vec3_t) * normal_count);
//gfx_face_t* faces = (gfx_face_t*)arena_calloc(scratch.arena, sizeof(gfx_face_t) * face_count);
//u32 position_index = 0;
//u32 texcoord_index = 0;
//u32 normal_index = 0;
//u32 face_index = 0;
//
// parse data into arrays
//str_list_t lines = str_split(scratch.arena, data, (u8*)"\n", 1);
//for (str_node_t* node = lines.first; node != nullptr; node = node->next) {
//str_t line = node->string;
//
//str_list_t tokens = str_split(scratch.arena, line, (u8*)" ", 1);
//
//if (tokens.first == nullptr) {
//continue;
//}
//
// positions
//if (str_match(tokens.first->string, str("v"))) {
//f32 x = f32_from_str(str_list_get_index(tokens, 1));
//f32 y = f32_from_str(str_list_get_index(tokens, 2));
//f32 z = f32_from_str(str_list_get_index(tokens, 3));
//positions[position_index++] = vec3(x, y, z);
//}
//
// texcoords
//if (str_match(tokens.first->string, str("vt"))) {
//f32 u = f32_from_str(str_list_get_index(tokens, 1));
//f32 v = f32_from_str(str_list_get_index(tokens, 2));
//texcoords[texcoord_index++] = vec2(u, v);
//}
//
// normals
//if (str_match(tokens.first->string, str("vn"))) {
//f32 x = f32_from_str(str_list_get_index(tokens, 1));
//f32 y = f32_from_str(str_list_get_index(tokens, 2));
//f32 z = f32_from_str(str_list_get_index(tokens, 3));
//normals[normal_index++] = vec3(x, y, z);
//}
//
// faces
//if (str_match(tokens.first->string, str("f"))) {
//
// scan through each face index
//for (str_node_t* token = tokens.first->next; token != nullptr; token = token->next) {
//
//gfx_face_t* face = &faces[face_index];
//
// split token
//str_list_t indices = str_split(scratch.arena, token->string, (u8*)"/", 1);
//
//str_t position_index_str = str_list_get_index(indices, 0);
//str_t texcoord_index_str = str_list_get_index(indices, 1);
//str_t normal_index_str = str_list_get_index(indices, 2);
//
//i32 position_index = position_index_str.size == 0 ? -1 : i32_from_str(position_index_str); 
//i32 texcoord_index = texcoord_index_str.size == 0 ? -1 : i32_from_str(texcoord_index_str); 
//i32 normal_index = normal_index_str.size == 0 ? -1 : i32_from_str(normal_index_str); 
//
//face->indices[face->index_count++] = {position_index, texcoord_index, normal_index };
//}
//
//face_index++;
//}
//
//}
//
// create unique vertices lookup table
//
//u32 max_vertices = 0;
//for (u32 i = 0; i < face_count; i++) { max_vertices += faces[i].index_count; }
//
//gfx_vertex_lookup_t* lookup = (gfx_vertex_lookup_t*)arena_alloc(scratch.arena, sizeof(gfx_vertex_lookup_t) * max_vertices);
//
//u32 unique_vertex_count = 0;
//u32 total_indices = 0;
//
//for (u32 i = 0; i < face_count; i++) {
//gfx_face_t* face = &faces[i];
//total_indices += (face->index_count - 2) * 3;
//
//for (u32 j = 0; j < face->index_count; j++) {
//gfx_face_index_t* face_index = &face->indices[j];
//
//b8 found = false;
//for (u32 k = 0; k < unique_vertex_count; k++) {
//if (lookup[k].position_index == face_index->position_index &&
//lookup[k].texcoord_index == face_index->texcoord_index &&
//lookup[k].normal_index == face_index->normal_index) {
//found = true;
//break;
//}
//}
//
//if (!found) {
//lookup[unique_vertex_count].position_index = face_index->position_index;
//lookup[unique_vertex_count].texcoord_index = face_index->texcoord_index;
//lookup[unique_vertex_count].normal_index = face_index->normal_index;
//lookup[unique_vertex_count].new_index = unique_vertex_count;
//unique_vertex_count++;
//}
//}
//}
//
//mesh->vertex_count = unique_vertex_count;
//mesh->vertices = (gfx_vertex_t*)arena_calloc(arena, sizeof(gfx_vertex_t) * mesh->vertex_count);
//
//mesh->index_count = total_indices;
//mesh->indices = (i32*)arena_calloc(arena, sizeof(i32) * mesh->index_count);
//
// fill vertex data
//for (u32 i = 0; i < unique_vertex_count; i++) {
//gfx_vertex_t* vertex = &mesh->vertices[i];
//
//vertex->position = positions[lookup[i].position_index - 1];
//vertex->texcoord = texcoords[lookup[i].texcoord_index - 1];
//vertex->normal = normals[lookup[i].normal_index - 1];
//vertex->color = vec4(1.0f);
//}
//
// fill index data
//u32 index_offset = 0;
//
//for (u32 i = 0; i < face_count; i++) {
//gfx_face_t* face = &faces[i];
//
//i32 face_indices[4];
//
//for (u32 j = 0; j < face->index_count; j++) {
//gfx_face_index_t* face_index = &face->indices[j];
//
//for (u32 k = 0; k < unique_vertex_count; k++) {
//if (lookup[k].position_index == face_index->position_index &&
//lookup[k].texcoord_index == face_index->texcoord_index &&
//lookup[k].normal_index == face_index->normal_index) {
//face_indices[j] = lookup[k].new_index;
//break;
//}
//}
//}
//
//for (u32 j = 0; j < face->index_count - 2; j++) {
//mesh->indices[index_offset++] = face_indices[0];
//mesh->indices[index_offset++] = face_indices[j + 2];
//mesh->indices[index_offset++] = face_indices[j + 1];
//}
//}
//
//scratch_end(scratch);
//}
//
//function void 
//_gfx_mesh_load_from_binary(arena_t* arena, gfx_mesh_t* mesh, str_t filepath) {
//
//os_handle_t file = os_file_open(filepath);
//
//u32 read_pos = 0;
//
// read header
//gfx_mesh_file_header_t header;
//read_pos += os_file_read(file, read_pos, &header, sizeof(gfx_mesh_file_header_t));
//
// validate
//if (header.magic != sora_gfx_mesh_magic_number || header.version > sora_gfx_mesh_file_version) {
//os_file_close(file);
//return;
//}
//
// allocate data
//mesh->vertex_count = header.vertex_count;
//mesh->vertices = (gfx_vertex_t*)arena_alloc(arena, sizeof(gfx_vertex_t) * mesh->vertex_count);
//mesh->index_count = header.index_count;
//mesh->indices = (i32*)arena_alloc(arena, sizeof(i32) * mesh->index_count);
//
// read data
//read_pos += os_file_read(file, read_pos, mesh->vertices, sizeof(gfx_vertex_t) * mesh->vertex_count);
//read_pos += os_file_read(file, read_pos, mesh->indices, sizeof(i32) * mesh->index_count);
//
//os_file_close(file);
//
//}
//
//function void 
//_gfx_mesh_write_to_obj(gfx_mesh_t* mesh, str_t filepath) {
// TODO:
//}
//
//function void 
//_gfx_mesh_write_to_binary(gfx_mesh_t* mesh, str_t filepath) {
//os_handle_t file = os_file_open(filepath, os_file_access_flag_write);
//u32 write_pos = 0;
//
// write header
//gfx_mesh_file_header_t header = {
//sora_gfx_mesh_magic_number,
//sora_gfx_mesh_file_version,
//mesh->vertex_count,
//mesh->index_count,
//0
//};
//
//write_pos += os_file_write(file, write_pos, &header, sizeof(gfx_mesh_file_header_t));
//
// write vertex data
//write_pos += os_file_write(file, write_pos, mesh->vertices, sizeof(gfx_vertex_t) * mesh->vertex_count);
//
// write index data
//write_pos += os_file_write(file, write_pos, mesh->indices, sizeof(i32) * mesh->index_count);
//
//os_file_close(file);
//}
//
//
//function void
//_gfx_meshes_load_from_obj(arena_t* arena, str_t filepath, gfx_handle_t** meshes, u32* mesh_count) {
//
//temp_t scratch = scratch_begin();
//
// load data
//str_t data = os_file_read_all(scratch.arena, filepath);
//
// count the data
//u32 object_count = 0;
//u32 position_count = 0;
//u32 texcoord_count = 0;
//u32 normal_count = 0;
//u32 face_count = 0;
//for (i32 i = 0; i <= data.size; i++) {
//u8 current_char = data.data[i];
//u8 next_char = data.data[i + 1];
//
//if (current_char == 'v') {
//if (next_char == ' ') {
//position_count++;
//} else if (next_char == 't') {
//texcoord_count++;
//} else if (next_char == 'n') {
//normal_count++;
//}
//} else if (current_char == 'f') {
//face_count++;
//} else if (current_char == 'o' && next_char == ' ') {
//object_count++;
//}
//}
//
//
// allocate temp arrays
//gfx_mesh_object_t* objects = (gfx_mesh_object_t*)arena_alloc(scratch.arena, sizeof(gfx_mesh_object_t) * object_count);
//vec3_t* positions = (vec3_t*)arena_calloc(scratch.arena, sizeof(vec3_t) * position_count);
//vec2_t* texcoords = (vec2_t*)arena_calloc(scratch.arena, sizeof(vec2_t) * texcoord_count);
//vec3_t* normals = (vec3_t*)arena_calloc(scratch.arena, sizeof(vec3_t) * normal_count);
//gfx_face_t* faces = (gfx_face_t*)arena_calloc(scratch.arena, sizeof(gfx_face_t) * face_count);
//u32 position_index = 0;
//u32 texcoord_index = 0;
//u32 normal_index = 0;
//u32 face_index = 0;
//u32 object_index = 0;
//
//u32 current_position_count = 0;
//u32 current_texcoord_count = 0;
//u32 current_normal_count = 0;
//u32 current_face_count = 0;
//
// parse data into arrays
//str_list_t lines = str_split(scratch.arena, data, (u8*)"\n", 1);
//for (str_node_t* node = lines.first; node != nullptr; node = node->next) {
//str_t line = node->string;
//
//str_list_t tokens = str_split(scratch.arena, line, (u8*)" ", 1);
//
//if (tokens.first == nullptr) {
//continue;
//}
//
// positions
//if (str_match(tokens.first->string, str("v"))) {
//f32 x = f32_from_str(str_list_get_index(tokens, 1));
//f32 y = f32_from_str(str_list_get_index(tokens, 2));
//f32 z = f32_from_str(str_list_get_index(tokens, 3));
//positions[position_index++] = vec3(x, y, z);
//current_position_count++;
//}
//
// texcoords
//else if (str_match(tokens.first->string, str("vt"))) {
//f32 u = f32_from_str(str_list_get_index(tokens, 1));
//f32 v = f32_from_str(str_list_get_index(tokens, 2));
//texcoords[texcoord_index++] = vec2(u, v);
//current_texcoord_count++;
//}
//
// normals
//else if (str_match(tokens.first->string, str("vn"))) {
//f32 x = f32_from_str(str_list_get_index(tokens, 1));
//f32 y = f32_from_str(str_list_get_index(tokens, 2));
//f32 z = f32_from_str(str_list_get_index(tokens, 3));
//normals[normal_index++] = vec3(x, y, z);
//current_normal_count++;
//}
//
// faces
//else if (str_match(tokens.first->string, str("f"))) {
//
// scan through each face index
//for (str_node_t* token = tokens.first->next; token != nullptr; token = token->next) {
//
//gfx_face_t* face = &faces[face_index];
//
// split token
//str_list_t indices = str_split(scratch.arena, token->string, (u8*)"/", 1);
//
//str_t position_index_str = str_list_get_index(indices, 0);
//str_t texcoord_index_str = str_list_get_index(indices, 1);
//str_t normal_index_str = str_list_get_index(indices, 2);
//
//i32 position_index = position_index_str.size == 0 ? -1 : i32_from_str(position_index_str); 
//i32 texcoord_index = texcoord_index_str.size == 0 ? -1 : i32_from_str(texcoord_index_str); 
//i32 normal_index = normal_index_str.size == 0 ? -1 : i32_from_str(normal_index_str); 
//
//face->indices[face->index_count++] = { position_index, texcoord_index, normal_index };
//}
//
//face_index++;
//current_face_count++;
//}
//
// objects
//else if (str_match(tokens.first->string, str("o"))) {
//
// store counts for previous object
//if (object_index != 0) {
//objects[object_index - 1].position_count = current_position_count;
//objects[object_index - 1].texcoord_count = current_texcoord_count;
//objects[object_index - 1].normal_count = current_normal_count;
//objects[object_index - 1].face_count = current_face_count;
//
//current_position_count = 0;
//current_texcoord_count = 0;
//current_normal_count = 0;
//current_face_count = 0;
//}
//
// store indices offset
//objects[object_index++] = { position_index, 0, texcoord_index, 0, normal_index, 0, face_index, 0 };
//}
//
//}
//
// allocate mesh list
//*meshes = (gfx_handle_t*)arena_calloc(arena, sizeof(gfx_handle_t) * object_count);
//
// for every object
//for (i32 o = 0; o < object_count; o++) {
//
// get object
//gfx_mesh_object_t* object = &objects[o];
//
// create unique vertices lookup table for this mesh
//u32 max_vertices = 0;
//for (u32 i = object->face_index; i < object->face_index + object->face_count; i++) { max_vertices += faces[i].index_count; }
//
//gfx_vertex_lookup_t* lookup = (gfx_vertex_lookup_t*)arena_alloc(scratch.arena, sizeof(gfx_vertex_lookup_t) * max_vertices);
//
//u32 unique_vertex_count = 0;
//u32 total_indices = 0;
//
//for (u32 i = object->face_index; i < object->face_index + object->face_count; i++) {
//gfx_face_t* face = &faces[i];
//total_indices += (face->index_count - 2) * 3;
//
//for (u32 j = 0; j < face->index_count; j++) {
//gfx_face_index_t* face_index = &face->indices[j];
//
// convert to local mesh indices
//u32 position_index = face_index->position_index; 
//u32 texcoord_index = face_index->texcoord_index; 
//u32 normal_index = face_index->normal_index; 
//
//b8 found = false;
//for (u32 k = 0; k < unique_vertex_count; k++) {
//if (lookup[k].position_index == position_index &&
//lookup[k].texcoord_index == texcoord_index &&
//lookup[k].normal_index == normal_index) {
//found = true;
//break;
//}
//}
//
//if (!found) {
//lookup[unique_vertex_count].position_index = position_index;
//lookup[unique_vertex_count].texcoord_index = texcoord_index;
//lookup[unique_vertex_count].normal_index = normal_index;
//lookup[unique_vertex_count].new_index = unique_vertex_count;
//unique_vertex_count++;
//}
//}
//}
//
// allocate new mesh
//
//gfx_mesh_t* mesh = (gfx_mesh_t*)arena_calloc(arena, sizeof(gfx_mesh_t)); 
//
//mesh->vertex_count = unique_vertex_count;
//mesh->vertices = (gfx_vertex_t*)arena_calloc(arena, sizeof(gfx_vertex_t) * unique_vertex_count);
//
//mesh->index_count = total_indices;
//mesh->indices = (i32*)arena_calloc(arena, sizeof(i32) * total_indices);
//
// compute centroid
//vec3_t centroid = vec3(0.0f);
//for (u32 i = 0; i < unique_vertex_count; i++) {
//vec3_t position = positions[lookup[i].position_index - 1];
//centroid = vec3_add(centroid, position);
//}
//centroid = vec3_div(centroid, unique_vertex_count);
//
// fill vertex data
//for (u32 i = 0; i < unique_vertex_count; i++) {
//gfx_vertex_t* vertex = &mesh->vertices[i];
//
//vertex->position = vec3_sub(positions[lookup[i].position_index - 1], centroid);
//vertex->texcoord = texcoords[lookup[i].texcoord_index - 1];
//vertex->normal = normals[lookup[i].normal_index - 1];
//vertex->color = vec4(1.0f);
//}
//
// fill index data
//u32 index_offset = 0;
//
//for (u32 i = object->face_index; i < object->face_index + object->face_count; i++) {
//gfx_face_t* face = &faces[i];
//
//i32 face_indices[4];
//
//for (u32 j = 0; j < face->index_count; j++) {
//gfx_face_index_t* face_index = &face->indices[j];
//
//for (u32 k = 0; k < unique_vertex_count; k++) {
//if (lookup[k].position_index == face_index->position_index &&
//lookup[k].texcoord_index == face_index->texcoord_index &&
//lookup[k].normal_index == face_index->normal_index) {
//face_indices[j] = lookup[k].new_index;
//break;
//}
//}
//}
//
//for (u32 j = 0; j < face->index_count - 2; j++) {
//mesh->indices[index_offset++] = face_indices[0];
//mesh->indices[index_offset++] = face_indices[j + 2];
//mesh->indices[index_offset++] = face_indices[j + 1];
//}
//}
//
// create handle and store into array
//gfx_handle_t mesh_handle = { (u64)mesh };
//(*meshes)[o] = mesh_handle;
//
//}
//
//*mesh_count = object_count;
//
//scratch_end(scratch);
//}
//
//

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