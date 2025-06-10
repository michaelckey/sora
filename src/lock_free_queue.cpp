// lock_free_queue.cpp

//~ includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "ui/sora_ui_inc.h"
#include "ui/sora_ui_inc.cpp"

#include "utils/queue.h"
#include "utils/queue.cpp"

#include "utils/task.h"
#include "utils/task.cpp"

//~ defines 

#define num_threads 5
#define items_per_producer 1000
#define items_total (num_threads * items_per_producer)

//~ structs 

struct profile_slot_t {
    cstr name;
    i32 depth;
    u64 start;
    u64 end;
};

struct profile_graph_t {
    
    profile_slot_t slots[1024];
    u32 slot_count;
    
    u32 slot_indices[512];
    u32 top_slot_index;
    
    u32 current_depth;
    
    u64 cpu_freq;
};

//~ globals

global arena_t* arena;
global os_handle_t window;
global gfx_handle_t context;
global ui_context_t* ui;
global b8 quit = false;

global profile_graph_t profile_graph;

//~ functions

// profiler
function void profile_init();
function void profile_release();
function void profile_begin();
function void profile_end();

function void ui_profile_graph(str_t label);
function void ui_profile_slot_draw(ui_node_t* node);

// app
function void app_init();
function void app_release();
function void app_frame();

//~ implementation 

//- profile functions 

function void 
profile_init() {
    profile_graph.cpu_freq = os_get_cpu_freq();
}

function void 
profile_release() {
    
}

function void 
profile_begin(cstr name) {
    
    // get index
    u32 index = profile_graph.slot_count;
    
    profile_slot_t* slot = &profile_graph.slots[index];
    profile_graph.slot_count++;
    
    // push this index to the stack
    profile_graph.slot_indices[++profile_graph.top_slot_index] = index;
    
    slot->name = name;
    slot->depth = profile_graph.current_depth;
    slot->start =  os_get_cpu_time();
    
    profile_graph.current_depth++;
}

function void 
profile_end() {
    
    u64 end_time = os_get_cpu_time();
    
    // pop index from stack
    u32 index = profile_graph.slot_indices[profile_graph.top_slot_index--];
    
    // get slot
    profile_slot_t* slot = &profile_graph.slots[index];
    profile_graph.current_depth--;
    slot->end = end_time;
}

function void 
ui_profile_graph(str_t label) {
    
    ui_node_flags graph_flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_background |
        ui_flag_draw_border |
        ui_flag_clip;
    
    ui_node_t* profile_graph_node = ui_node_from_string(graph_flags, label);
    
    u64 start_ms = profile_graph.slots[0].start;
    f64 total_ms = 0.0f;
    for (i32 i = 0; i < profile_graph.slot_count; i++) {
        profile_slot_t* slot = &profile_graph.slots[i];
        if (slot->depth == 0) {
            total_ms += (f64)(slot->end - slot->start) * 1000.0f / (f64)(profile_graph.cpu_freq);
        }
    }
    
    ui_push_parent(profile_graph_node);
    
    f32 graph_width = rect_width(profile_graph_node->rect);
    f32 graph_height = rect_height(profile_graph_node->rect);
    
    for (i32 i = 0; i < profile_graph.slot_count; i++) {
        profile_slot_t* slot = &profile_graph.slots[i];
        
        // calculate rect
        f64 min_ms =  (f64)(slot->start - start_ms) * 1000.0f / (f64)(profile_graph.cpu_freq);
        f64 max_ms =  (f64)(slot->end - start_ms) * 1000.0f / (f64)(profile_graph.cpu_freq);
        vec2_t min_pos = vec2((min_ms / total_ms) * graph_width, 25.0f * slot->depth); 
        vec2_t max_pos = vec2((max_ms / total_ms) * graph_width, 25.0f * (slot->depth + 1)); 
        
        f32 slot_ms = max_ms - min_ms;
        
        // build ui node
        ui_set_next_rect(rect(min_pos, max_pos));
        ui_node_t* slot_node = ui_node_from_stringf(ui_flag_mouse_interactable | ui_flag_draw_border | ui_flag_draw_custom, "%s_%p", label.data, slot);
        ui_node_set_display_string(slot_node, str_format(ui_build_arena(), "%s : %.2f ms", slot->name, slot_ms));
        ui_node_set_custom_draw(slot_node, ui_profile_slot_draw, nullptr);
        
        ui_interaction interaction = ui_interaction_from_node(slot_node);
        
        if (interaction & ui_hovered) {
            ui_tooltip_begin();
            
            ui_set_next_size(ui_size_by_text(2.0f), ui_size_by_text(2.0f));
            ui_labelf("%s : %.2f ms", slot->name, slot_ms);
            
            ui_tooltip_end();
        }
        
        
    }
    
    ui_pop_parent();
    
    // interaction
    ui_interaction interaction = ui_interaction_from_node(profile_graph_node);
    
}

function void
ui_profile_slot_draw(ui_node_t* node) {
    
    f32 hue = (f32)((u32)node % 360) / 360.0f;
    
    color_t col = color_rgb_from_hsv(color(hue, 0.4f, 0.5f, 1.0f));
    
    // draw background
    ui_r_set_next_color(col );
    ui_r_draw_rect(node->rect, 0.0f, 0.33f, node->rounding);
    
    // draw text
    ui_r_push_clip_mask(node->rect);
    
    color_t text_color = ui_color_from_key_name(node->tags_key, str("text"));
    color_t shadow_color = ui_color_from_key_name(node->tags_key, str("text shadow"));
    vec2_t text_pos = ui_text_align(node->font, node->font_size, node->label, node->rect, node->text_alignment);
    
    ui_r_set_next_color(shadow_color);
    ui_r_draw_text(node->label, vec2_add(text_pos, 1.0f), node->font, node->font_size);
    
    ui_r_set_next_color(text_color);
    ui_r_draw_text(node->label, text_pos, node->font, node->font_size);
    
    ui_r_pop_clip_mask();
}

//- app functions 

function void 
app_init() {
    
    profile_begin("app_init");
    
    // create arena
    arena = arena_create(gigabytes(2));
    
    // open window and create renderer
    profile_begin("window_create");
    window = os_window_open(str("lock free queue"), 1280, 720);
    os_window_set_frame_function(window, app_frame);
    profile_end();
    
    // create graphics context
    profile_begin("graphics_create");
    context = gfx_context_create(window);
    profile_end();
    
    // create ui context
    profile_begin("ui_create");
    ui = ui_context_create(window, context);
    ui_context_default_theme(ui);
    profile_end();
    
    profile_end();
    
}


function void 
app_release() {
    gfx_context_release(context);
    os_window_close(window);
    arena_release(arena);
}

function void
app_frame() {
    os_get_events();
    
    // update window and context
    os_window_update(window);
    gfx_context_update(context);
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    
    // build ui
    ui_begin(ui);
    
    ui_set_next_rect(rect(25.0f, 25.0f, 1025.0f, 525.0f));
    ui_profile_graph(str("profile_graph"));
    
    ui_end(ui);
    
    
    // render
    gfx_set_context(context);
    gfx_context_clear(context, color(0x1D1F20ff));
    
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
    profile_init();
    task_init(4, 8);
    
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    task_release();
    
    profile_release();
    ui_release();
    font_release();
    gfx_release();
    os_release();
    
    return 0;
}