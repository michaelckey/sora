// mesh.h

#ifndef MESH_H
#define MESH_H

//- defines 

#define sora_mesh_magic_number 0x48534D52
#define sora_mesh_file_version 1

//-structs 

struct face_index_t {
    i32 position_index;
    i32 texcoord_index;
    i32 normal_index;
};

struct face_t {
    face_index_t indices[4];
    u32 index_count;
};

struct mesh_object_t {
    u32 position_offset;
    u32 position_count;
    u32 texcoord_offset;
    u32 texcoord_count;
    u32 normal_offset;
    u32 normal_count;
    u32 face_index;
    u32 face_count;
};

struct vertex_lookup_t {
    i32 position_index;
    i32 texcoord_index;
    i32 normal_index;
    i32 new_index;
};

struct vertex_t {
    vec3_t position;
    vec3_t normal;
    vec3_t tangent;
    vec2_t texcoord;
    vec4_t color;
};

struct mesh_t {
    vertex_t* vertices;
    u32 vertex_count;
    
    i32* indices;
    u32 index_count;
};

struct mesh_file_header_t {
    u32 magic;
    u32 version;
    u32 vertex_count;
    u32 index_count;
    u32 flags;
};

//- functions 

function mesh_t* mesh_load(arena_t* arena, str_t filepath);

// internal
function void _mesh_load_from_obj(arena_t* arena, mesh_t* mesh, str_t filepath);
function void _mesh_load_from_binary(arena_t* arena, mesh_t* mesh, str_t filepath);
function void _mesh_write_to_obj(mesh_t* mesh, str_t filepath);
function void _mesh_write_to_binary(mesh_t* mesh, str_t filepath);

#endif // MESH_H