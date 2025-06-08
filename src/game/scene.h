// scene.h

#ifndef SCENE_H
#define SCENE_H

//~ enums

typedef u32 scene_node_flags;
enum {
    scene_node_flag_null = 0,
    scene_node_flag_dirty = (1 << 0),
};

//~ structs 

struct scene_node_t {
    
    // global list
    scene_node_t* global_next;
    scene_node_t* global_prev;
    
    // graph list
    scene_node_t* parent;
    scene_node_t* child_first;
    scene_node_t* child_last;
    scene_node_t* prev;
    scene_node_t* next;
    
    char name[128];
    u32 name_length;
    u64 id;
    scene_node_flags flags;
    
    vec3_t position;
    vec3_t rotation;
    vec3_t scale;
    mat4_t transform;
    
    mesh_t* mesh;
    
};

struct scene_node_rec_t {
    scene_node_t* next;
    u32 push_count;
    u32 pop_count;
};

// ui scene node list
struct ui_scene_node_t {
    ui_scene_node_t* next;
    ui_scene_node_t* prev;
    
    scene_node_t* node;
    b8 opened;
};

struct ui_scene_node_hash_list_t {
    ui_scene_node_t* first;
    ui_scene_node_t* last;
};

struct ui_scene_hierarchy_t {
    arena_t* arena;
    ui_scene_node_hash_list_t* hash_list;
    u32 hash_list_count;
    ui_scene_node_t* node_free;
};

// scene

struct scene_t {
    
    arena_t* arena;
    
    // graph
    scene_node_t* node_root;
    scene_node_t* node_first;
    scene_node_t* node_last;
    scene_node_t* node_free;
    
    u64 id_counter;
    
    scene_node_t* node_selected;
    
    // ui data
    ui_scene_hierarchy_t* ui_hierarchy;
    
};


//~ functions 

// scene
function scene_t* scene_create();
function void scene_release(scene_t* scene);

// scene node
function scene_node_t* scene_node_create(scene_t* scene, str_t name);
function void scene_node_release(scene_t* scene, scene_node_t* node);
function void scene_node_add_child(scene_node_t* parent, scene_node_t* child);
function void scene_node_remove_child(scene_node_t* parent, scene_node_t* child);
function scene_node_rec_t scene_node_rec_depth_first(scene_node_t* node, b8 skip_children);

// scene internal
function u64 _scene_get_id(scene_t* scene);
function scene_node_t* _scene_node_alloc(scene_t* scene);
function void _scene_node_release(scene_t* scene, scene_node_t* node);

//- ui

// views
function void ui_scene_hierarchy(scene_t* scene);
function void ui_scene_node_properties(scene_node_t* node);

// internal
function ui_scene_node_t* _ui_scene_node_find(ui_scene_hierarchy_t* list, scene_node_t* node);
function void _ui_scene_node_draw(ui_node_t* node);

#endif // SCENE_H