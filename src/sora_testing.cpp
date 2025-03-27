// sora_testing.cpp

//- includes

#include "sora_inc.h"
#include "sora_inc.cpp"

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
    renderer = gfx_renderer_create(window, color(0x111317ff));
    
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
    
    // update layers
    os_update();
    gfx_update();
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    // render
    if (!gfx_handle_equals(renderer, { 0 })) {
        gfx_renderer_begin(renderer);
        uvec2_t renderer_size = gfx_renderer_get_size(renderer);
        
        draw_begin(renderer);
        
        
        draw_text(str("Hello, World!"), vec2(5.0f, 5.0f));
        
        vec2_t center = vec2(renderer_size.x * 0.5f, renderer_size.y *0.5f);
        
        vec2_t p0 = vec2_add(center, vec2(-100.0f, 50.0f));
        vec2_t p1 = vec2_add(center, vec2(0.0f, -123.0f));
        vec2_t p2 = vec2_add(center, vec2(100.0f, 50.0f));
        
        draw_set_next_color0(color(0xff4545ff));
        draw_set_next_color1(color(0x45ff45ff));
        draw_set_next_color2(color(0x4545ffff));
        draw_tri(p0, p1, p2);
        
        draw_end(renderer);
        gfx_renderer_end(renderer);
    }
    
}

//- entry point

function i32 
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    gfx_init();
    font_init();
    draw_init();
    
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    draw_release();
    font_release();
    gfx_release();
    os_release();
    
	return 0;
}