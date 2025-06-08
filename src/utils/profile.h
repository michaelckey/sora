// profile.h

#ifndef PROFILE_H
#define PROFILE_H

//- defines 

#define prof_begin(name) profile_begin(name)
#define prof_end() profile_end()

//- structs 

struct profile_entry_t {
    cstr name;
    u64 cycles;
    u32 hit_count;
};

struct profile_scope_t {
    cstr name;
    u64 start_time;
};

struct thread_profiler_t {
    
    arena_t* arena;
    
    profile_entry_t* entries;
    u32 entry_count;
    
    profile_scope_t* scopes;
    u32 scope_count;
    
}

struct profiler_t {
    arena_t* arena;
    
    queue_t* entry_queue;
    
    u64 cpu_freq;
};


//- globals 

global profiler_t profiler;
thread_global thread_profiler_t thread_profiler;

//- functions 

function void profile_init();
function void profile_release();
function void profile_clear();
function void profile_begin_frame();
function void profile_end_frame();

function void profile_begin(const char* name);
function void profile_end();

// internal

function i32 _profile_qsort_compare_entries(const void* a, const void* b);

#endif // PROFILE_H
