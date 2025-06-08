// scene_renderer_test.cpp

//- includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "ui/sora_ui_inc.h"
#include "ui/sora_ui_inc.cpp"

//- globals

global os_handle_t window;
global gfx_handle_t context;
global b8 quit = false;

global ui_context_t* ui;

//- functions

function void app_init();
function void app_release();
function void app_frame();

//- implementation

function void 
app_init() {
    
    // open window 
    window = os_window_open(str("2d renderer"), 1280, 720);
    os_window_set_frame_function(window, app_frame);
    
    // create graphics context
    context = gfx_context_create(window);
    
    // create ui context
    ui = ui_context_create(window, context);
    
    ui_theme_add_color(ui->arena, ui->theme, str("background"), color(0x252629ff));
    ui_theme_add_color(ui->arena, ui->theme, str("background alt"), color(0x353639ff));
    ui_theme_add_color(ui->arena, ui->theme, str("hover"), color(0xffffe725));
    ui_theme_add_color(ui->arena, ui->theme, str("active"), color(0xffffe725));
    ui_theme_add_color(ui->arena, ui->theme, str("border"), color(0xffffff35));
    ui_theme_add_color(ui->arena, ui->theme, str("shadow"), color(0x00000080));
    ui_theme_add_color(ui->arena, ui->theme, str("text"), color(0xe2e2e3ff));
    ui_theme_add_color(ui->arena, ui->theme, str("accent"), color(0x393a3fff));
    
    ui_theme_add_color(ui->arena, ui->theme, str("background red"), color(0xc33a3fff));
    
}

function void 
app_release() {
    
    ui_context_release(ui);
    
    gfx_context_release(context);
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
    
    // update window and context
    os_window_update(window);
    gfx_context_update(context);
    
    // build ui
    ui_begin(ui);
    
    
    
    ui_end(ui);
    
    // fill constant buffers
    uvec2_t window_size = gfx_context_get_size(context);
    rect_t viewport = rect(0.0f, 0.0f, (f32)window_size.x, (f32)window_size.y);
    
    // render
    gfx_set_context(context);
    gfx_context_clear(context, color(0x121416ff));
    
    ui_render(ui);
    
    gfx_context_present(context);
    
}

//- entry point

function i32 
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    gfx_init();
    font_init();
    ui_init();
    
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    ui_release();
    font_release();
    gfx_release();
    os_release();
    
    return 0;
}