// sora_os.cpp

#ifndef SORA_OS_CPP
#define SORA_OS_CPP

//~ implementation 

//- handle functions  

function b8 
os_handle_equals(os_handle_t handle_a, os_handle_t handle_b) {
    return (handle_a.data[0] == handle_b.data[0]);
}

//- events functions 

function void 
os_events_clear() {
    arena_clear(os_core_state.events_arena);
    os_core_state.events.first = nullptr;
    os_core_state.events.last= nullptr;
    os_core_state.events.count = 0;
}

function void
os_events_push(os_event_t* event) {
    dll_push_back(os_core_state.events.first, os_core_state.events.last, event);
    os_core_state.events.count++;
}

function os_event_t*
os_events_pop(os_event_t* event) {
    dll_remove(os_core_state.events.first, os_core_state.events.last, event);
    os_core_state.events.count--;
}

function os_event_t*
os_events_find(os_handle_t window, os_event_type event_type) {
    os_event_t* result = nullptr;
    for (os_event_t* event = os_core_state.events.first; event != nullptr; event = event->next) {
        if (os_handle_equals(window, event->window) && event->type == event_type) {
            result = event;
            break;
        }
    }
    return result;
}

function b8
os_key_press(os_handle_t window, os_key key, os_modifier modifiers) {
    b8 result = false;
    for (os_event_t* event = os_core_state.events.first; event != nullptr; event = event->next) {
        if (os_handle_equals(window, event->window) && (event->type = os_event_key_press) && 
            (event->key == key) && (event->modifiers & modifiers)) {
            result = true;
            os_events_pop(event);
            break;
        }
    }
    return result;
}

function b8
os_key_release(os_handle_t window, os_key key, os_modifier modifiers) {
    b8 result = false;
    for (os_event_t* event = os_core_state.events.first; event != nullptr; event = event->next) {
        if (os_handle_equals(window, event->window) && (event->type = os_event_key_release) && 
            (event->key == key) && (event->modifiers & modifiers)) {
            result = true;
            os_events_pop(event);
            break;
        }
    }
    return result;
}

//~ internal implementation

//- core state functions 

function void 
_os_init_core() {
    
    // allocate arenas
    os_core_state.events_arena = arena_create(megabytes(1));
    
    // inits events
    os_core_state.events.first = nullptr;
    os_core_state.events.last = nullptr;
    os_core_state.events.count = 0;
    
    
    
}

function void
_os_release_core() {
    
    // release arenas
    arena_release(os_core_state.events_arena);
    
}






#endif // SORA_OS_CPP