// sora_os.cpp

#ifndef SORA_OS_CPP
#define SORA_OS_CPP

//~ implementation 

//- handle functions

function b8 
os_window_equals(os_window_t window_a, os_window_t window_b) {
    return (window_a.id == window_b.id);
}

function b8 
os_file_equals(os_file_t file_a, os_file_t file_b) {
    return (file_a.id == file_b.id);
}

function b8 
os_process_equals(os_process_t process_a, os_process_t process_b) {
    return (process_a.id == process_b.id);
}

function b8 
os_thread_equals(os_thread_t thread_a, os_thread_t thread_b) {
    return (thread_a.id == thread_b.id);
}

function b8 
os_fiber_equals(os_fiber_t fiber_a, os_fiber_t fiber_b) {
    return (fiber_a.id == fiber_b.id);
}

function b8 
os_mutex_equals(os_mutex_t mutex_a, os_mutex_t mutex_b) {
    return (mutex_a.id == mutex_b.id);
}

function b8 
os_rw_mutex_equals(os_rw_mutex_t rw_mutex_a, os_rw_mutex_t rw_mutex_b) {
    return (rw_mutex_a.id == rw_mutex_b.id);
}

function b8 
os_condition_variable_equals(os_condition_variable_t condition_variable_a, os_condition_variable_t condition_variable_b) {
    return (condition_variable_a.id == condition_variable_b.id);
}

function b8 
os_semaphore_equals(os_semaphore_t semaphore_a, os_semaphore_t semaphore_b) {
    return (semaphore_a.id == semaphore_b.id);
}

function b8 
os_socket_equals(os_socket_t socket_a, os_socket_t socket_b) {
    return (socket_a.id == socket_b.id);
}

//- events functions

function void
os_event_release(os_event_t* event) {
    stack_push(os_core_state.event_free, event);
}

function os_event_t*
os_events_push(os_window_t window, os_event_type type) {
    
    // allocate an event
    os_event_t* event = os_core_state.event_free;
    if (event != nullptr) {
        stack_pop(os_core_state.event_free);
    } else {
        event = (os_event_t*)arena_alloc(os_core_state.events_arena, sizeof(os_event_t));
    }
    memset(event, 0, sizeof(os_event_t));
    
    // copy the event and push to list
    dll_push_back(os_core_state.event_first, os_core_state.event_last, event);
    
    event->window = window;
    event->type = type;
    
    return event;
}

function os_event_t*
os_event_peek() {
    return os_core_state.event_last;
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
    os_event_release(event);
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

function os_event_t*
os_events_find_last(os_window_t window, os_event_type event_type) {
    os_event_t* result = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = event->next) {
        if (os_window_equals(window, event->window) && event->type == event_type) {
            result = event;
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

//- event queries functions

function b8
os_modifiers_equals(os_modifiers a, os_modifiers b) {
    b8 result = false;
    if (b == os_modifier_any) {
        result = true;
    } else if (b == os_modifier_none){
        result = a == os_modifier_none;
    } else {
        result = (a & b) == b;
    }
    return result;
}

function b8
os_key_press(os_window_t window, os_key key, os_modifiers modifiers) {
    b8 result = false;
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_key_press) && 
            (event->key == key) &&
            (os_modifiers_equals(event->modifiers, modifiers))) {
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function b8
os_key_release(os_window_t window, os_key key, os_modifiers modifiers) {
    b8 result = false;
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_key_release) && 
            (event->key == key) &&
            (os_modifiers_equals(event->modifiers, modifiers))) {
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function u32
os_key_text(os_window_t window) {
    u32 result = 0;
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_key_text)){
            result = event->keycode;
            os_events_remove(event);
            break;
        }
    }
    return result;
}


function b8
os_key_is_down(os_key key) {
    return os_core_state.keys[key];
}

function b8
os_mouse_move(os_window_t window) {
    b8 result = false;
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_mouse_move)){
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function b8
os_mouse_enter(os_window_t window) {
    b8 result = false;
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_mouse_enter)){
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function b8
os_mouse_leave(os_window_t window) {
    b8 result = false;
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_mouse_leave)){
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function b8
os_mouse_press(os_window_t window, os_mouse_button mouse_button, os_modifiers modifiers) {
    b8 result = false;
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_mouse_button_press) && 
            (event->mouse_button == mouse_button) && 
            (os_modifiers_equals(event->modifiers, modifiers))) {
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}


function b8
os_mouse_release(os_window_t window, os_mouse_button mouse_button, os_modifiers modifiers) {
    b8 result = false;
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_mouse_button_release) && 
            (event->mouse_button == mouse_button) && 
            (os_modifiers_equals(event->modifiers, modifiers))) {
            result = true;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function vec2_t
os_mouse_scroll(os_window_t window, os_modifiers modifiers) {
    vec2_t result = { 0 };
    os_event_t* next = nullptr;
    for (os_event_t* event = os_core_state.event_first; event != nullptr; event = next) {
        next = event->next;
        if (os_window_equals(window, event->window) && 
            (event->type == os_event_mouse_scroll) && 
            (os_modifiers_equals(event->modifiers, modifiers))) {
            result = event->mouse_scroll;
            os_events_remove(event);
            break;
        }
    }
    return result;
}

function b8
os_mouse_is_down(os_mouse_button mouse_button) {
    return os_core_state.mouse_buttons[mouse_button];
}


//~ internal implementation

//- core state functions 

function void 
_os_init_core() {
    
    // allocate arenas
    os_core_state.events_arena = arena_create(megabytes(1));
    
}

function void
_os_release_core() {
    
    // release arenas
    arena_release(os_core_state.events_arena);
    
}

//~ backend implementations

#if OS_BACKEND_WIN32
#    include "backends/os/sora_os_win32.cpp"
#elif OS_BACKEND_LINUX

#elif OS_BACKEND_MACOS

#else

#endif 

#endif // SORA_OS_CPP