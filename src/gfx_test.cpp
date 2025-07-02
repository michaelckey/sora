// gfx_test.cpp

//~ includes

#include "core/sora_base.h"
#include "core/sora_os_new.h"

#include "core/sora_base.cpp"
#include "core/sora_os_new.cpp"

//~ globals

global arena_t* arena;
global os_handle_t window;

//~ functions

function void app_init();
function void app_release();
function void app_frame();

//~ implementation

function void
app_init() {
    
    
}

function void
app_release() {
    
}

function void
app_frame() {
    
}

//- entry point 

function i32 
app_entry_point(i32 argc, char** argv) {
    
    app_init();
    
    printf("Hello World!\n");
    
    return 0;
}