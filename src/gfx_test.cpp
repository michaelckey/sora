// gfx_test.cpp

//~ includes

#include "core/sora_base.h"
#include "core/sora_os.h"
#include "core/sora_gfx.h"

#include "core/sora_base.cpp"
#include "core/sora_os.cpp"
#include "core/sora_gfx.cpp"

//~ globals

global arena_t* arena;
global os_window_t window;
global gfx_context_t context;
global b8 running = true;

global u64 last_time;

//~ functions

function void app_init();
function void app_release();
function void app_update();
function void app_render();

//~ implementation

function void
app_init() {
    
    arena = arena_create(gigabytes(1));
    
    window = os_window_open(str("window"), 960, 720);
    os_window_set_resize_callback(window, app_render);
    
    context = gfx_context_create(window);
    
    last_time = os_get_time_ms();
    
    str_t test_string = str("This is a test string!");
    
}

function void
app_release() {
    
    gfx_context_release(context);
    os_window_close(window);
    arena_release(arena);
    
}

function void
app_update() {
    os_poll_events();
    persist b8 fullscreen = false;
    
    if (os_key_press(window, os_key_f11)) {
        fullscreen = !fullscreen;
        os_window_set_fullscreen(window, fullscreen);
    }
    
    if (os_key_press(window, os_key_esc) || os_events_find(window, os_event_window_close)) {
        running = false;
    }
    
    u64 current_time = os_get_time_ms();
    u64 delta_time = current_time - last_time;
    last_time = current_time;
    f64 dt = (f64)delta_time / 1000.0;
    
}

function void
app_render() {
    uvec2_t window_size = os_window_get_size(window);
    gfx_context_resize(context, window_size);
    gfx_context_set(context);
    gfx_context_clear(context, color(0x123456ff));
    
    
    
    gfx_context_present(context);
}

//- entry point 

function i32 
app_entry_point(i32 argc, char** argv) {
    
    os_init();
    gfx_init();
    
    app_init();
    
    while (running) {
        app_update();
        app_render();
    }
    
    app_release();
    
    gfx_release();
    os_release();
    
    return 0;
}