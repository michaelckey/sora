// profile.cpp

#ifndef PROFILE_CPP
#define PROFILE_CPP

//- implementation 


function void
profile_init() {
    
    profiler.arena = arena_create(megabytes(64));
    
    
    
    // allocate
    profiler.current_entries = (profile_entry_t*)arena_calloc(profiler.arena, sizeof(profile_entry_t) * 1024);
    profiler.current_entry_count = 0;
    
    profiler.last_entries = (profile_entry_t*)arena_calloc(profiler.arena, sizeof(profile_entry_t) * 1024);
    profiler.last_entry_count = 0;
    
    profiler.scopes = (profile_scope_t*)arena_calloc(profiler.arena, sizeof(profile_scope_t) * 512);
    profiler.scope_count = 0;
    
    profiler.cpu_freq = os_get_cpu_freq();
}

function void
profile_release() {
    arena_release(profiler.arena);
}


function void
profile_begin_frame() {
    
    memset(profiler.current_entries, 0, sizeof(profile_entry_t) * 1024);
    memset(profiler.scopes, 0, sizeof(profile_scope_t) * 512);
    profiler.current_entry_count = 0;
    profiler.scope_count = 0;
    
}

function void
profile_end_frame() {
    
    // sort the current entries
    qsort(profiler.current_entries, profiler.current_entry_count, sizeof(profile_entry_t), _profile_qsort_compare_entries);
    
    // copies the current entries into the last frame
    memcpy(profiler.last_entries, profiler.current_entries, sizeof(profile_entry_t) * profiler.current_entry_count);
    profiler.last_entry_count = profiler.current_entry_count;
    
}

function void
profile_begin(const char* name) {
    
    // grab a scope 
    profile_scope_t* scope = &profiler.scopes[profiler.scope_count++];
    
    // copy
    scope->name = name;
    scope->start_time = os_get_cpu_time();
    
}

function void
profile_end() {
    
    // get scope
    u64 end_time = os_get_cpu_time();
    profile_scope_t* scope = &profiler.scopes[--profiler.scope_count];
    u64 elapsed = end_time - scope->start_time;
    
    // find entry
    profile_entry_t* entry = nullptr;
    for (i32 i = 0; i < profiler.current_entry_count; i++) {
        if (cstr_equals(profiler.current_entries[i].name, scope->name)) {
            entry = &profiler.current_entries[i];
            break;
        }
    }
    
    if (entry == nullptr) {
        entry = &profiler.current_entries[profiler.current_entry_count++];
        
        entry->name = scope->name;
        entry->cycles = 0;
        entry->hit_count = 0;
    }
    
    if (entry) {
        entry->hit_count++;
        entry->cycles += elapsed;
    }
    
}

//- internal functions 

function i32
_profile_qsort_compare_entries(const void* a, const void* b) {
    
    profile_entry_t* entry_a = (profile_entry_t*)a;
    profile_entry_t* entry_b = (profile_entry_t*)b;
    
    i32 result = 0;
    
    if (entry_a->cycles > entry_b->cycles) {
        result = -1;
    } else if (entry_a->cycles < entry_b->cycles) {
        result = 1;
    }
    
    return result;
}




#endif // PROFILE_CPP