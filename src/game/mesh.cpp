// mesh.cpp

#ifndef MESH_CPP
#define MESH_CPP

//- implementation 

function mesh_t*
mesh_load(arena_t* arena, str_t filepath) {
    temp_t scratch = scratch_begin();
    
    // create resource
    mesh_t* mesh = (mesh_t*)arena_alloc(arena, sizeof(mesh_t)); 
    
    str_t name = str_get_file_name(filepath);
    str_t filepath_no_ext = str_substr(filepath, 0, str_find_substr(filepath, str("."), 0, str_match_flag_find_last));
    str_t filepath_obj = str_format(scratch.arena, "%.*s.obj", filepath_no_ext.size, filepath_no_ext.data);
    str_t filepath_binary = str_format(scratch.arena, "%.*s.bin", filepath_no_ext.size, filepath_no_ext.data);
    
    if (os_file_exists(filepath_binary)) {
        _mesh_load_from_binary(arena, mesh, filepath_binary);
    } else {
        _mesh_load_from_obj(arena, mesh, filepath_obj);
        _mesh_write_to_binary(mesh, filepath_binary);
    }
    
    scratch_end(scratch);
    
    return mesh;
}



function void 
_mesh_load_from_obj(arena_t* arena, mesh_t* mesh, str_t filepath) {
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
    face_t* faces = (face_t*)arena_calloc(scratch.arena, sizeof(face_t) * face_count);
    u32 position_index = 0;
    u32 texcoord_index = 0;
    u32 normal_index = 0;
    u32 face_index = 0;
    
    // parse data into arrays
    str_list_t lines = str_split(scratch.arena, data, (u8*)"\n", 1);
    for (str_node_t* node = lines.first; node != nullptr; node = node->next) {
        str_t line = node->string;
        
        str_list_t tokens = str_split(scratch.arena, line, (u8*)" ", 1);
        
        if (tokens.first == nullptr) {
            continue;
        }
        
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
                
                face_t* face = &faces[face_index];
                
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
    
    vertex_lookup_t* lookup = (vertex_lookup_t*)arena_alloc(scratch.arena, sizeof(vertex_lookup_t) * max_vertices);
    
    u32 unique_vertex_count = 0;
    u32 total_indices = 0;
    
    for (u32 i = 0; i < face_count; i++) {
        face_t* face = &faces[i];
        total_indices += (face->index_count - 2) * 3;
        
        for (u32 j = 0; j < face->index_count; j++) {
            face_index_t* face_index = &face->indices[j];
            
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
    mesh->vertices = (vertex_t*)arena_calloc(arena, sizeof(vertex_t) * mesh->vertex_count);
    
    mesh->index_count = total_indices;
    mesh->indices = (i32*)arena_calloc(arena, sizeof(i32) * mesh->index_count);
    
    // fill vertex data
    for (u32 i = 0; i < unique_vertex_count; i++) {
        vertex_t* vertex = &mesh->vertices[i];
        
        vertex->position = positions[lookup[i].position_index - 1];
        vertex->texcoord = texcoords[lookup[i].texcoord_index - 1];
        vertex->normal = normals[lookup[i].normal_index - 1];
        vertex->color = vec4(1.0f);
    }
    
    // fill index data
    u32 index_offset = 0;
    
    for (u32 i = 0; i < face_count; i++) {
        face_t* face = &faces[i];
        
        i32 face_indices[4];
        
        for (u32 j = 0; j < face->index_count; j++) {
            face_index_t* face_index = &face->indices[j];
            
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
_mesh_load_from_binary(arena_t* arena, mesh_t* mesh, str_t filepath) {
    
    os_handle_t file = os_file_open(filepath);
    
    u32 read_pos = 0;
    
    // read header
    mesh_file_header_t header;
    read_pos += os_file_read(file, read_pos, &header, sizeof(mesh_file_header_t));
    
    // validate
    if (header.magic != sora_mesh_magic_number || header.version > sora_mesh_file_version) {
        os_file_close(file);
        return;
    }
    
    // allocate data
    mesh->vertex_count = header.vertex_count;
    mesh->vertices = (vertex_t*)arena_alloc(arena, sizeof(vertex_t) * mesh->vertex_count);
    mesh->index_count = header.index_count;
    mesh->indices = (i32*)arena_alloc(arena, sizeof(i32) * mesh->index_count);
    
    // read data
    read_pos += os_file_read(file, read_pos, mesh->vertices, sizeof(vertex_t) * mesh->vertex_count);
    read_pos += os_file_read(file, read_pos, mesh->indices, sizeof(i32) * mesh->index_count);
    
    os_file_close(file);
    
}

function void 
_mesh_write_to_obj(mesh_t* mesh, str_t filepath) {
    // TODO:
}

function void 
_mesh_write_to_binary(mesh_t* mesh, str_t filepath) {
    os_handle_t file = os_file_open(filepath, os_file_access_flag_write);
    u32 write_pos = 0;
    
    // write header
    mesh_file_header_t header = {
        sora_mesh_magic_number,
        sora_mesh_file_version,
        mesh->vertex_count,
        mesh->index_count,
        0
    };
    
    write_pos += os_file_write(file, write_pos, &header, sizeof(mesh_file_header_t));
    
    // write vertex data
    write_pos += os_file_write(file, write_pos, mesh->vertices, sizeof(vertex_t) * mesh->vertex_count);
    
    // write index data
    write_pos += os_file_write(file, write_pos, mesh->indices, sizeof(i32) * mesh->index_count);
    
    os_file_close(file);
}


function void
_meshes_load_from_obj(arena_t* arena, str_t filepath, handle_t** meshes, u32* mesh_count) {
    
    temp_t scratch = scratch_begin();
    
    // load data
    str_t data = os_file_read_all(scratch.arena, filepath);
    
    // count the data
    u32 object_count = 0;
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
        } else if (current_char == 'o' && next_char == ' ') {
            object_count++;
        }
    }
    
    
    // allocate temp arrays
    mesh_object_t* objects = (mesh_object_t*)arena_alloc(scratch.arena, sizeof(mesh_object_t) * object_count);
    vec3_t* positions = (vec3_t*)arena_calloc(scratch.arena, sizeof(vec3_t) * position_count);
    vec2_t* texcoords = (vec2_t*)arena_calloc(scratch.arena, sizeof(vec2_t) * texcoord_count);
    vec3_t* normals = (vec3_t*)arena_calloc(scratch.arena, sizeof(vec3_t) * normal_count);
    face_t* faces = (face_t*)arena_calloc(scratch.arena, sizeof(face_t) * face_count);
    u32 position_index = 0;
    u32 texcoord_index = 0;
    u32 normal_index = 0;
    u32 face_index = 0;
    u32 object_index = 0;
    
    u32 current_position_count = 0;
    u32 current_texcoord_count = 0;
    u32 current_normal_count = 0;
    u32 current_face_count = 0;
    
    // parse data into arrays
    str_list_t lines = str_split(scratch.arena, data, (u8*)"\n", 1);
    for (str_node_t* node = lines.first; node != nullptr; node = node->next) {
        str_t line = node->string;
        
        str_list_t tokens = str_split(scratch.arena, line, (u8*)" ", 1);
        
        if (tokens.first == nullptr) {
            continue;
        }
        
        // positions
        if (str_match(tokens.first->string, str("v"))) {
            f32 x = f32_from_str(str_list_get_index(tokens, 1));
            f32 y = f32_from_str(str_list_get_index(tokens, 2));
            f32 z = f32_from_str(str_list_get_index(tokens, 3));
            positions[position_index++] = vec3(x, y, z);
            current_position_count++;
        }
        
        // texcoords
        else if (str_match(tokens.first->string, str("vt"))) {
            f32 u = f32_from_str(str_list_get_index(tokens, 1));
            f32 v = f32_from_str(str_list_get_index(tokens, 2));
            texcoords[texcoord_index++] = vec2(u, v);
            current_texcoord_count++;
        }
        
        // normals
        else if (str_match(tokens.first->string, str("vn"))) {
            f32 x = f32_from_str(str_list_get_index(tokens, 1));
            f32 y = f32_from_str(str_list_get_index(tokens, 2));
            f32 z = f32_from_str(str_list_get_index(tokens, 3));
            normals[normal_index++] = vec3(x, y, z);
            current_normal_count++;
        }
        
        // faces
        else if (str_match(tokens.first->string, str("f"))) {
            
            // scan through each face index
            for (str_node_t* token = tokens.first->next; token != nullptr; token = token->next) {
                
                face_t* face = &faces[face_index];
                
                // split token
                str_list_t indices = str_split(scratch.arena, token->string, (u8*)"/", 1);
                
                str_t position_index_str = str_list_get_index(indices, 0);
                str_t texcoord_index_str = str_list_get_index(indices, 1);
                str_t normal_index_str = str_list_get_index(indices, 2);
                
                i32 position_index = position_index_str.size == 0 ? -1 : i32_from_str(position_index_str); 
                i32 texcoord_index = texcoord_index_str.size == 0 ? -1 : i32_from_str(texcoord_index_str); 
                i32 normal_index = normal_index_str.size == 0 ? -1 : i32_from_str(normal_index_str); 
                
                face->indices[face->index_count++] = { position_index, texcoord_index, normal_index };
            }
            
            face_index++;
            current_face_count++;
        }
        
        // objects
        else if (str_match(tokens.first->string, str("o"))) {
            
            // store counts for previous object
            if (object_index != 0) {
                objects[object_index - 1].position_count = current_position_count;
                objects[object_index - 1].texcoord_count = current_texcoord_count;
                objects[object_index - 1].normal_count = current_normal_count;
                objects[object_index - 1].face_count = current_face_count;
                
                current_position_count = 0;
                current_texcoord_count = 0;
                current_normal_count = 0;
                current_face_count = 0;
            }
            
            // store indices offset
            objects[object_index++] = { position_index, 0, texcoord_index, 0, normal_index, 0, face_index, 0 };
        }
        
    }
    
    // allocate mesh list
    *meshes = (handle_t*)arena_calloc(arena, sizeof(handle_t) * object_count);
    
    // for every object
    for (i32 o = 0; o < object_count; o++) {
        
        // get object
        mesh_object_t* object = &objects[o];
        
        // create unique vertices lookup table for this mesh
        u32 max_vertices = 0;
        for (u32 i = object->face_index; i < object->face_index + object->face_count; i++) { max_vertices += faces[i].index_count; }
        
        vertex_lookup_t* lookup = (vertex_lookup_t*)arena_alloc(scratch.arena, sizeof(vertex_lookup_t) * max_vertices);
        
        u32 unique_vertex_count = 0;
        u32 total_indices = 0;
        
        for (u32 i = object->face_index; i < object->face_index + object->face_count; i++) {
            face_t* face = &faces[i];
            total_indices += (face->index_count - 2) * 3;
            
            for (u32 j = 0; j < face->index_count; j++) {
                face_index_t* face_index = &face->indices[j];
                
                // convert to local mesh indices
                u32 position_index = face_index->position_index; 
                u32 texcoord_index = face_index->texcoord_index; 
                u32 normal_index = face_index->normal_index; 
                
                b8 found = false;
                for (u32 k = 0; k < unique_vertex_count; k++) {
                    if (lookup[k].position_index == position_index &&
                        lookup[k].texcoord_index == texcoord_index &&
                        lookup[k].normal_index == normal_index) {
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    lookup[unique_vertex_count].position_index = position_index;
                    lookup[unique_vertex_count].texcoord_index = texcoord_index;
                    lookup[unique_vertex_count].normal_index = normal_index;
                    lookup[unique_vertex_count].new_index = unique_vertex_count;
                    unique_vertex_count++;
                }
            }
        }
        
        // allocate new mesh
        
        mesh_t* mesh = (mesh_t*)arena_calloc(arena, sizeof(mesh_t)); 
        
        mesh->vertex_count = unique_vertex_count;
        mesh->vertices = (vertex_t*)arena_calloc(arena, sizeof(vertex_t) * unique_vertex_count);
        
        mesh->index_count = total_indices;
        mesh->indices = (i32*)arena_calloc(arena, sizeof(i32) * total_indices);
        
        // compute centroid
        vec3_t centroid = vec3(0.0f);
        for (u32 i = 0; i < unique_vertex_count; i++) {
            vec3_t position = positions[lookup[i].position_index - 1];
            centroid = vec3_add(centroid, position);
        }
        centroid = vec3_div(centroid, unique_vertex_count);
        
        // fill vertex data
        for (u32 i = 0; i < unique_vertex_count; i++) {
            vertex_t* vertex = &mesh->vertices[i];
            
            vertex->position = vec3_sub(positions[lookup[i].position_index - 1], centroid);
            vertex->texcoord = texcoords[lookup[i].texcoord_index - 1];
            vertex->normal = normals[lookup[i].normal_index - 1];
            vertex->color = vec4(1.0f);
        }
        
        // fill index data
        u32 index_offset = 0;
        
        for (u32 i = object->face_index; i < object->face_index + object->face_count; i++) {
            face_t* face = &faces[i];
            
            i32 face_indices[4];
            
            for (u32 j = 0; j < face->index_count; j++) {
                face_index_t* face_index = &face->indices[j];
                
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
        
        // store in array
        (*meshes)[o] = mesh;
        
    }
    
    *mesh_count = object_count;
    
    scratch_end(scratch);
}



#endif // MESH_CPP