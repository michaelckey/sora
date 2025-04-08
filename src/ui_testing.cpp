// ui_testing.cpp

//- includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "ui/sora_ui_inc.h"
#include "ui/sora_ui_inc.cpp"

//- structs 

struct list_item_t {
    list_item_t* next;
    list_item_t* prev;
    
    str_t label;
    ui_node_t* node;
};

struct list_t {
    list_item_t* first;
    list_item_t* last;
};

struct list_drag_data_t {
    list_item_t* current;
    list_item_t* prev;
    f32 offset;
};

//- globals

global os_handle_t window;
global gfx_handle_t renderer;
global ui_context_t* ui;
global arena_t* arena;
global b8 quit = false;

global font_handle_t font;

global list_t list;

//- functions

function list_item_t* list_item_add(arena_t* arena, list_t* list, str_t label);

function void app_init();
function void app_release();
function void app_frame();

//- implementation

function list_item_t*
list_item_add(arena_t* arena, list_t* list, str_t label) {
    list_item_t* item = (list_item_t*)arena_alloc(arena, sizeof(list_item_t));
    item->label = label;
    dll_push_back(list->first, list->last, item);
    return item;
}

function void
app_init() {
    
	// open window and create renderer
	window = os_window_open(str("sora ui testing"), 1280, 960);
	renderer = gfx_renderer_create(window, color(0x131315ff));
    ui = ui_context_create(window, renderer);
    arena = arena_create(megabytes(128));
    
	// set frame function
	os_window_set_frame_function(window, app_frame);
    
    list.first = nullptr;
    list.last = nullptr;
    
    for (i32 i = 0; i < 15; i++) {
        str_t string = str_format(arena, " Item %u", i);
        list_item_add(arena, &list, string);
    }
    
    font = font_open(str("res/fonts/deja_vu_sans.ttf"));
    
}

function void
app_release() {
	
	// release renderer and window
	ui_context_release(ui);
    gfx_renderer_release(renderer);
	os_window_close(window);
    
    
}

function void
app_frame() {
    
	// update layers
	os_update();
    os_window_update(window);
	gfx_update();
    
    
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
        uvec2_t renderer_size = gfx_renderer_get_size(renderer);
		gfx_renderer_begin(renderer);
        
        //- build ui 
        ui_begin(ui);
        ui_push_font(font);
        ui_push_font_size(10.0f);
        ui_push_size(ui_size_pixels(150.0f), ui_size_pixels(25.0f));
        vec2_t mouse_pos = os_window_get_cursor_pos(window);
        
        //- list 
        b8 dragged_this_frame = false;
        for (list_item_t* item = list.first,*next = nullptr; item != nullptr; item = next) {
            next = item->next;
            
            // build node
            ui_node_flags node_flags = 
                ui_flag_mouse_interactable |
                ui_flag_draw_background | 
                ui_flag_draw_border | 
                ui_flag_draw_text |
                ui_flag_anim_pos_y;
            
            ui_set_next_hover_cursor(os_cursor_hand_point);
            item->node = ui_node_from_string(node_flags, item->label);
            ui_interaction interaction = ui_interaction_from_node(item->node);
            
            // begin drag
            if ((interaction & ui_left_pressed) && !ui_drag_is_active()) {
                dragged_this_frame = true;
                ui_drag_begin(item->node->key);
                
                // store drag state
                list_drag_data_t drag_data = { 0 };
                drag_data.current = item;
                drag_data.prev = item->prev;
                drag_data.offset =  ui->drag_start_pos.y - item->node->rect.y0;
                ui_drag_store_data(&drag_data, sizeof(list_drag_data_t));
                
                // move item to top of list
                dll_remove(list.first, list.last, item);
                dll_push_front(list.first, list.last, item);
            }
            
            // update drag data
            if (ui_drag_is_active()) {
                list_drag_data_t* data = (list_drag_data_t*)ui_drag_get_data();
                
                // set dragged node at mouse pos
                if (item == data->current) {
                    item->node->flags |= ui_flag_fixed_pos_y;
                    item->node->flags |= ui_flag_draw_shadow;
                    item->node->pos_fixed.y = mouse_pos.y - data->offset;
                }
                
                // build blank node
                if ((dragged_this_frame && data->current == item) || 
                    (data->prev == item)) {
                    ui_set_next_size_y(data->current->node->size_wanted[1]);
                    ui_node_t* blank_node = ui_node_from_key(0, { 0 });
                }
                
                // drop drag
                if (interaction & ui_left_released) {
                    dll_remove(list.first, list.last, data->current);
                    dll_insert(list.first, list.last, data->prev == data->current ? nullptr : data->prev, data->current);
                }
                
            }
            
        }
        
        // update drag data
        if (ui_drag_is_active()) {
            list_drag_data_t* data = (list_drag_data_t*)ui_drag_get_data();
            
            // get closest item in list
            ui_node_t* dragged_node = data->current->node;
            f32 dragged_center = (dragged_node->rect.y0 + dragged_node->rect.y1) * 0.5f;
            
            list_item_t* closest_item = list.first;
            for (list_item_t* i = list.first->next; i != nullptr; i = i->next) {
                ui_node_t* current_node = i->node;
                f32 current_center = (current_node->rect.y0 + current_node->rect.y1) * 0.5f;
                
                if (dragged_center > current_center) {
                    closest_item = i;
                }
            }
            
            data->prev = closest_item;
        }
        
        
        
        //- 
        
        ui_pop_size();
        ui_pop_font();
        ui_pop_font_size();
        
        
        
        //- table 
        
        temp_t scratch = scratch_begin();
        
        ui_set_next_rect(rect(400.0f, 150.0f, 700.0f, 450.0f));
        ui_set_next_layout_dir(ui_dir_right);
        ui_node_t* container = ui_node_from_key(ui_flag_draw_background, { 0 });
        
        ui_push_parent(container);
        
        ui_push_size(ui_size_percent(1.0f), ui_size_percent(1.0f));
        
        for (i32 i = 0; i < 5; i++) {
            
            ui_set_next_layout_dir(ui_dir_down);
            ui_node_t* row = ui_node_from_key(ui_flag_draw_border, { 0 });
            ui_push_parent(row);
            
            for (i32 j = 0; j < 5; j ++) {
                /*ui_node_flags node_flags = 
                    ui_flag_mouse_interactable |
                    ui_flag_draw_background | 
                    ui_flag_draw_border | 
                    ui_flag_draw_text;
                
                ui_set_next_text_alignment(ui_text_alignment_center);
                ui_node_t* item = ui_node_from_string(node_flags, str_format(scratch.arena, "%i, %i", i, j));
                ui_interaction interaction = ui_interaction_from_node(item);*/
                
                ui_node_t* item = ui_node_from_key(ui_flag_draw_border, { 0 });
                
                
            }
            ui_pop_parent();
        }
        ui_pop_size();
        
        ui_pop_parent();
        
        scratch_end(scratch);
        
        
        
        
        
        
        
        ui_end(ui);
        
        //- draw ui 
        draw_begin(renderer);
        
        for (ui_node_t* node = ui->node_root; node != nullptr;) {
            ui_node_rec_t rec = ui_node_rec_depth_first(node);
            
            if (node->flags & ui_flag_draw_shadow) {
                draw_set_next_color(color(0x00000090));
                draw_set_next_softness(6.0f);
                draw_rect(rect_translate(rect_grow(node->rect, 4.0f), 4.0f));
            }
            
            if (node->flags & ui_flag_draw_background) {
                
                f32 height = rect_height(node->rect);
                
                rect_t top_rect = rect_cut_bottom(node->rect, roundf(height * 0.5f));
                rect_t bottom_rect = rect_cut_top(node->rect, roundf(height * 0.25f));
                
                color_t col_mid = color(0x252629ff);
                color_t col_end = color(0x292a2fff);
                
                col_mid = color_lerp(col_mid, color_blend(col_mid, color(0xffffff05)), node->hover_t);
                col_mid = color_lerp(col_mid, color_blend(col_mid, color(0xffffff05)), node->active_t);
                
                col_end = color_lerp(col_end, color_blend(col_end, color(0xffeedd25)), node->hover_t);
                col_end = color_lerp(col_end, color_blend(col_end, color(0xffeedd65)), node->active_t);
                
                draw_set_next_color(col_mid);
                draw_set_next_rounding(node->rounding);
                draw_rect(node->rect);
                
                draw_set_next_color0(col_mid);
                draw_set_next_color1(col_end);
                draw_set_next_color2(col_mid);
                draw_set_next_color3(col_end);
                draw_set_next_rounding(node->rounding);
                draw_rect( top_rect);
                
                draw_set_next_color0(col_end);
                draw_set_next_color1(col_mid);
                draw_set_next_color2(col_end);
                draw_set_next_color3(col_mid);
                draw_set_next_rounding(node->rounding);
                draw_rect(bottom_rect );
                
                
            }
            
            if (node->flags & ui_flag_draw_border) {
                
                draw_set_next_color0(color(0xffffff35));
                draw_set_next_color1(color(0xffffff35));
                draw_set_next_color2(color(0xffffff55));
                draw_set_next_color3(color(0xffffff15));
                draw_set_next_rounding(node->rounding);
                draw_set_next_thickness(1.0f);
                draw_rect(rect_shrink(node->rect, 1.0f));
                
                draw_set_next_color(color(0x131313ff));
                draw_set_next_rounding(node->rounding);
                draw_set_next_thickness(1.0f);
                draw_rect(node->rect);
                
            }
            
            if (node->flags & ui_flag_draw_text) {
                vec2_t text_pos = ui_text_align(node->font, node->font_size, node->label, node->rect, node->text_alignment);
                draw_push_font(node->font);
                draw_push_font_size(node->font_size);
                
                draw_set_next_color(color(0x00000090));
                draw_text(node->label, vec2_add(text_pos, 1.0f));
                
                draw_set_next_color(color(0xe2e2e2ff));
                draw_text(node->label, text_pos);
                
                draw_pop_font();
                draw_pop_font_size();
            }
            
            node = rec.next;
        }
        
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