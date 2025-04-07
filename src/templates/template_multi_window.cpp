// template_multi_window.cpp

// includes

#include "sora_inc.h"
#include "sora_inc.cpp"

// app structs

struct app_window_t {
	app_window_t* next;
	app_window_t* prev;
    
	str_t title;
    
    os_handle_t os_window;
    gfx_handle_t gfx_renderer;
};

struct app_state_t {
    
	arena_t* arena;
	arena_t* scratch;
    
	app_window_t* window_first;
	app_window_t* window_last;
	app_window_t* window_free;
};

// globals
global app_state_t app_state;

// functions

function app_window_t* app_window_open(str_t title, u32 width, u32 height);
function void app_window_close(app_window_t* window);
function app_window_t* app_window_from_os_window(os_handle_t os_window);
function b8 app_no_active_windows();

function void app_init();
function void app_release();
function void app_frame();

// implementation

function app_window_t* 
app_window_open(str_t title, u32 width, u32 height) {
    
	// try to get unused window from app state
	app_window_t* window = app_state.window_free;
	if (window != nullptr) {
		stack_pop(app_state.window_free);
	} else {
		// else allocate a new window
		window = (app_window_t*)arena_alloc(app_state.arena, sizeof(app_window_t));
	}
	memset(window, 0, sizeof(app_window_t));
    
	// add to app state window list
	dll_push_back(app_state.window_first, app_state.window_last, window);
    
	window->title = title;
    
	// open os window
	window->os_window = os_window_open(title, width, height);
    
	// create gfx renderer
	window->gfx_renderer = gfx_renderer_create(window->os_window, color(0x131313ff));
    
	os_window_set_frame_function(window->os_window, app_frame);
    
	return window;
}

function void
app_window_close(app_window_t* window) {
    
	// remove from app state window list
	dll_remove(app_state.window_first, app_state.window_last, window);
    
	// add to app state window free stack
	stack_push(app_state.window_free, window);
    
	// release os window and gfx renderer
	gfx_renderer_release(window->gfx_renderer);
	os_window_close(window->os_window);
    
}

function app_window_t*
app_window_from_os_window(os_handle_t os_window) {
	app_window_t* result = nullptr;
	for (app_window_t* app_window = app_state.window_first; app_window != 0; app_window = app_window->next) {
		if (os_handle_equals(app_window->os_window, os_window)) {
			result = app_window;
			break;
		}
	}
	return result;
}

function b8
app_no_active_windows() {
	return (app_state.window_first == 0);
}

function void
app_init() {
    
	// allocate memory arena
	app_state.arena = arena_create(gigabytes(2));
    
	// window list
	app_state.window_first = nullptr;
	app_state.window_last = nullptr;
	app_state.window_free = nullptr;
	
	// open windows
	app_window_open(str("multi-window 1"), 640, 480);
	app_window_open(str("multi-window 2"), 640, 480);
    
}

function void
app_release() {
    
	// release memory arena
	arena_release(app_state.arena);
}


function void 
app_frame() {
    
	// update layers
	os_update();
	gfx_update();
    
	// update every window
	for (app_window_t* window = app_state.window_first, *next = 0; window != 0; window = next) {
		next = window->next;
        
		// hotkeys
		if (os_key_press(window->os_window, os_key_F11)) {
			os_window_fullscreen(window->os_window);
		}
        
		if (os_key_press(window->os_window, os_key_esc)) {
			app_window_close(window);
			continue;
		}
        
        if (os_key_press(window->os_window, os_key_N, os_modifier_ctrl)) {
            app_window_open(str("another window"), 640, 480);
        }
        
        // render
        if (!gfx_handle_equals(window->gfx_renderer, {0})) {
            
            uvec2_t renderer_size = gfx_renderer_get_size(window->gfx_renderer); 
            
            gfx_renderer_begin(window->gfx_renderer);
			draw_begin(window->gfx_renderer);
            
            str_t text = str("press 'Ctrl+N' for a new window.");
            f32 text_width = font_text_get_width(draw_top_font(), draw_top_font_size(), text);
            f32 text_height = font_text_get_height(draw_top_font(), draw_top_font_size(), text);
            vec2_t text_pos = vec2((renderer_size.x - text_width) * 0.5f, (renderer_size.y - text_height) * 0.5f);
            
            draw_text(text, text_pos);
            
			draw_end(window->gfx_renderer);
			gfx_renderer_end(window->gfx_renderer);
		}
	}
    
	// get close events
	os_event_t* close_event = os_event_get(os_event_type_window_close);
	if (close_event != nullptr) {
		app_window_t* window = app_window_from_os_window(close_event->window);
		if (window != nullptr) {
			app_window_close(window);
		}
		os_event_pop(close_event);
	}
}

// entry point

function i32
app_entry_point(i32 argc, char** argv) {
    
	// init layers
	os_init();
	gfx_init();
	font_init();
	draw_init();
	
	// init
	app_init();
    
	// main loop
	while (!app_no_active_windows()) {
		app_frame();
	}
    
	// release
	app_release();
    
	// release layers
	draw_release();
	font_release();
	gfx_release();
	os_release();
    
	return 0;
}