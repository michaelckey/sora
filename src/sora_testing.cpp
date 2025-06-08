// sora_testing.cpp

//- includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

//- globals

global os_handle_t window;
global gfx_handle_t renderer;
global b8 quit = false;

//- functions

function void app_init();
function void app_release();
function void app_frame();

//- implementation

function void 
app_init() {
    
    // open window and create renderer
    window = os_window_open(str("sora testing"), 1280, 720);
    renderer = gfx_renderer_create(window);
    
    // set window frame function
    os_window_set_frame_function(window, app_frame);
    
}

function void 
app_release() {
    
    // release renderer and close window
    gfx_renderer_release(renderer);
    os_window_close(window);
    
}

function void
app_frame() {
    
    // get events
    os_get_events();
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    // update window and renderer
    os_window_update(window);
    gfx_renderer_update(renderer);
    
    // render
    gfx_renderer_clear(renderer, color(0x121416ff));
    
    gfx_renderer_present(renderer);
    
}

//- entry point

function i32 
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    gfx_init();
    font_init();
    
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    font_release();
    gfx_release();
    os_release();
    
	return 0;
}