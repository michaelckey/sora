// sora_os.cpp

#ifndef SORA_OS_CPP
#define SORA_OS_CPP

// implementation

// handles

function b8 
os_handle_equals(os_handle_t a, os_handle_t b) {
	return a.data[0] == b.data[0];
}

// events

function void
os_event_push(os_event_t* event) {
	os_event_t* new_event = (os_event_t*)arena_alloc(os_state.event_list_arena, sizeof(os_event_t));
	memcpy(new_event, event, sizeof(os_event_t));
	dll_push_back(os_state.event_list.first, os_state.event_list.last, new_event);
	os_state.event_list.count++;
}

function void
os_event_pop(os_event_t* event) {
	dll_remove(os_state.event_list.first, os_state.event_list.last, event);
	os_state.event_list.count--;
}

function os_event_t*
os_event_get(os_event_type type) {
	os_event_t* result = nullptr;
	for (os_event_t* e = os_state.event_list.first; e != 0; e = e->next) {
		if (e->type == type) {
			result = e;
			break;
		}
	}
	return result;
}

function b8
os_key_press(os_handle_t window, os_key key, os_modifiers modifiers) {
	b8 result = false;
	for (os_event_t* e = os_state.event_list.first; e != 0; e = e->next) {
		if (e->type == os_event_type_key_press && os_handle_equals(window, e->window) &&
			e->key == key &&
            ((e->modifiers & modifiers) != 0 || (e->modifiers == 0 && modifiers == 0) || modifiers == os_modifier_any)) {
            
            os_event_pop(e);
			result = true;
			break;
		}
	}
	return result;
}

function b8
os_key_release(os_handle_t window, os_key key, os_modifiers modifiers) {
	b8 result = false;
	for (os_event_t* e = os_state.event_list.first; e != 0; e = e->next) {
		if (e->type == os_event_type_key_release && os_handle_equals(window, e->window) &&
			e->key == key &&
			((e->modifiers & modifiers) || (e->modifiers == 0 && modifiers == 0) || modifiers == os_modifier_any)) {
			os_event_pop(e);
			result = true;
			break;
		}
	}
	return result;
}

function b8
os_mouse_press(os_handle_t window, os_mouse_button button, os_modifiers modifiers) {
	b8 result = false;
	for (os_event_t* e = os_state.event_list.first; e != 0; e = e->next) {
		if (e->type == os_event_type_mouse_press && os_handle_equals(window, e->window) &&
			e->mouse == button &&
			((e->modifiers & modifiers) || (e->modifiers == 0 && modifiers == 0) || modifiers == os_modifier_any)) {
			os_event_pop(e);
			result = true;
			break;
		}
	}
	return result;
}

function b8
os_mouse_release(os_handle_t window, os_mouse_button button, os_modifiers modifiers) {
	b8 result = false;
	for (os_event_t* e = os_state.event_list.first; e != 0; e = e->next) {
		if (e->type == os_event_type_mouse_release && os_handle_equals(window, e->window) &&
			e->mouse == button &&
			((e->modifiers & modifiers) || (e->modifiers == 0 && modifiers == 0) || modifiers == os_modifier_any)) {
			os_event_pop(e);
			result = true;
			break;
		}
	}
	return result;
}

function f32
os_mouse_scroll(os_handle_t window) {
	f32 result = 0.0f;
	for (os_event_t* e = os_state.event_list.first; e != 0; e = e->next) {
		if (e->type == os_event_type_mouse_scroll && os_handle_equals(window, e->window)) {
			os_event_pop(e);
			result = e->scroll.y;
			break;
		}
	}
	return result;
}

function vec2_t
os_mouse_move(os_handle_t window) {
    
	vec2_t result = vec2(0.0f, 0.0f);
    
	for (os_event_t* e = os_state.event_list.first; e != 0; e = e->next) {
		if (e->type == os_event_type_mouse_move && os_handle_equals(window, e->window)) {
			os_event_pop(e);
			result = e->position;
			break;
		}
	}
    
	return result;
}


// backend implementation

#if defined(OS_BACKEND_WIN32)
#include "backends/os/sora_os_win32.cpp"
#elif defined(OS_BACKEND_LINUX)
#include "backends/os/sora_os_linux.cpp"
#elif defined(OS_BACKEND_MACOS)
#include "backends/os/sora_os_macos.cpp"
#else
#error undefined os backend.
#endif

#endif // SORA_OS_CPP