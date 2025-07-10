// sora_os.cpp

#ifndef SORA_OS_CPP
#define SORA_OS_CPP

//~ implementation 

//- handle functions  

function b8 
os_window_equals(os_window_t window_a, os_window_t window_b) {
    return (window_a.id == window_b.id);
}

//- events functions 

function os_event_t* 
os_event_create(os_event_type type) {
    os_event_t* event = os_core_state.event_free;
    if (event != nullptr) {
        stack_pop(os_core_state.event_free);
    } else {
        event = (os_event_t*)arena_alloc(os_core_state.events_arena, sizeof(os_event_t));
    }
    memset(event, 0, sizeof(os_event_t));
    event->type = type;
    return event;
}

function void
os_event_release(os_event_t* event) {
    stack_push(os_core_state.event_free, event);
}

function void
os_events_push(os_event_t* event) {
    dll_push_back(os_core_state.event_first, os_core_state.event_last, event);
}

function os_event_t*
os_events_pop() {
    os_event_t* event = os_core_state.event_last;
    dll_remove(os_core_state.event_first, os_core_state.event_last, os_core_state.event_last);
    return event;
}

function void
os_events_remove(os_event_t* event) {
    dll_remove(os_core_state.event_first, os_core_state.event_last, event);
}

function os_event_t*
os_events_find(os_window_t window, os_event_type event_type) {
    os_event_t* result = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = event->next) {
        if (os_window_equals(window, event->window) && event->type == event_type) {
            result = event;
            break;
        }
    }
    return result;
}

function void 
os_events_clear() {
    arena_clear(os_core_state.events_arena);
    os_core_state.event_first = nullptr;
    os_core_state.event_last= nullptr;
}


function b8
os_key_press(os_window_t window, os_key key, os_modifier modifiers) {
    b8 result = false;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = event->next) {
        if (os_window_equals(window, event->window) && (event->type = os_event_key_press) && 
            (event->key == key) && (event->modifiers & modifiers)) {
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function b8
os_key_release(os_window_t window, os_key key, os_modifier modifiers) {
    b8 result = false;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = event->next) {
        if (os_window_equals(window, event->window) && (event->type = os_event_key_release) && 
            (event->key == key) && (event->modifiers & modifiers)) {
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function b8
os_key_is_down(os_window_t window, os_key key) {
    b8 result = false;
    if (os_window_is_focused(window)) {
        result = os_core_state.keys[key];
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
    os_core_state.event_first = nullptr;
    os_core_state.event_last = nullptr;
    os_core_state.event_free = nullptr;
    
}

function void
_os_release_core() {
    
    // release arenas
    arena_release(os_core_state.events_arena);
    
}


#endif // SORA_OS_CPP