// profile.h

#ifndef PROFILE_H
#define PROFILE_H

//- defines 

#define prof_begin(name) profile_begin(name)
#define prof_end() profile_end()

//- structs 

struct profile_slot_t {
    cstr name;
    i32 depth;
    u64 start;
    u64 end;
};

struct profiler_t {
    arena_t* arena;
    
    profile_slot_t* slots;
    u32 slot_count;
    
    u32 slot_indices[512];
    u32 top_slot_index;
    
    u32 current_depth;
    
    u64 cpu_freq;
};

struct profile_graph_draw_data_t {
    f64 start_ms;
    f64 current_start_ms;
    f64 current_end_ms;
};

//- globals 

global profiler_t profiler;

//- functions 

function void profile_init();
function void profile_release();

function void profile_begin(cstr name);
function void profile_end();

// ui
function void ui_profile_graph(str_t label);
function void ui_profile_graph_draw(ui_node_t* node);
function void ui_profile_slot_draw(ui_node_t* node);

#endif // PROFILE_H
