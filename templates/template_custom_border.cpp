// template_custom_border.cpp

// TODO: on win10, there seems to be missing a 
// single pixel line at the top of the window.
// this doesn't occur on win11.

// includes

#include "sora_inc.h"
#include "sora_inc.cpp"

// globals
global os_handle_t window;
global gfx_handle_t renderer;
global b8 quit = false;

// functions

function void app_init();
function void app_release();
function void app_frame();

// implementation

function void
app_init() {
    
	// open window and create renderer
	window = os_window_open(str("borderless"), 640, 480, os_window_flag_borderless);
	renderer = gfx_renderer_create(window, color(0x121212ff));
    
	os_window_set_frame_function(window, app_frame);
    
}

function void
app_release() {
	
	// release renderer and window
	gfx_renderer_release(renderer);
	os_window_close(window);
    
}

function void
app_frame() {
	
	// update layers
	os_update();
	gfx_update();
    
	// add custom title bar areas
    uvec2_t window_size = os_window_get_size(window);
	os_window_clear_title_bar_client_area(window);
	os_window_add_title_bar_client_area(window, rect((f32)window_size.x - 120.0f, 0.0f, (f32)window_size.x, 30.0f));
    
	// hotkeys
	if (os_key_press(window, os_key_F11)) {
		os_window_fullscreen(window);
	}
    
    // close
	if (os_key_press(window, os_key_esc) || os_event_get(os_event_type_window_close) != 0) {
		quit = true;
	}
    
	// render
	if (!gfx_handle_equals(renderer, { 0 })) {
		gfx_renderer_begin(renderer);
		draw_begin(renderer);
        
        
        
		draw_end(renderer);
		gfx_renderer_end(renderer);
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
	while (!quit) {
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
