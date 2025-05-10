// ui_testing.cpp

//~ includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "ui/sora_ui_inc.h"
#include "ui/sora_ui_inc.cpp"

//~ enums

typedef u32 node_prop_flags;
enum {
    node_prop_flag_none = 0,
    node_prop_flag_input = (1 << 0),
    node_prop_flag_output = (1 << 1),
};

//~ structs 

// list 
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

// node graph

struct graph_t;
struct node_t;
struct node_prop_t;

struct node_t {
    node_t* stack_next;
    
    // prop list
    node_prop_t* prop_first;
    node_prop_t* prop_last;
    u32 input_count;
    u32 output_count;
    
    graph_t* graph;
    
    str_t label;
    vec2_t pos;
    vec2_t size;
    color_t color;
    
    ui_node_t* ui_node;
};

struct node_list_item_t {
    node_list_item_t* next;
    node_list_item_t* prev;
    node_t* node;
};

struct node_prop_t {
    node_prop_t* next;
    node_prop_t* prev;
    
    str_t label;
    node_prop_flags flags;
    f32 value;
};


struct graph_t {
    
    arena_t* arena;
    
    node_t* node_free;
    
    node_list_item_t* node_list_first;
    node_list_item_t* node_list_last;
    node_list_item_t* node_list_item_free;
    
};

//~ globals

global os_handle_t window;
global gfx_handle_t renderer;
global ui_context_t* ui;
global arena_t* arena;
global b8 quit = false;

global font_handle_t font;

global list_t list;
global graph_t* graph;

//~ functions

function list_item_t* list_item_add(arena_t* arena, list_t* list, str_t label);

// node graph
function graph_t* graph_create();
function void graph_release(graph_t* graph);

function node_t* graph_node_create(graph_t* graph, str_t label, vec2_t pos, color_t col = color(0x563523ff));
function void graph_node_release(graph_t* graph, node_t* node);
function void graph_node_prop_add(node_t* node, str_t label, node_prop_flags flags = 0);
function void graph_node_render(node_t* node);
function void graph_node_title_draw_func(ui_node_t* node);

function void graph_node_prop_render(node_prop_t* prop);

// app
function void app_init();
function void app_release();
function void app_frame();


//~ implementation

//- list functions 

function list_item_t*
list_item_add(arena_t* arena, list_t* list, str_t label) {
    list_item_t* item = (list_item_t*)arena_alloc(arena, sizeof(list_item_t));
    item->label = label;
    dll_push_back(list->first, list->last, item);
    return item;
}

//- node graph functions

function graph_t* 
graph_create() {
    
    arena_t* arena = arena_create(megabytes(256));
    graph_t* graph = (graph_t*)arena_alloc(arena, sizeof(graph_t));
    graph->arena = arena;
    
    graph->node_list_first = nullptr;
    graph->node_list_last= nullptr;
    
    return graph;
}

function void 
graph_release(graph_t* graph) {
    arena_release(graph->arena);
}

function node_t*
graph_node_create(graph_t* graph, str_t label, vec2_t pos, color_t col) {
    
    // grab from free list or allocate one.
    node_t* node = graph->node_free;
    if (node != nullptr) {
        stack_pop_n(graph->node_free, stack_next);
    } else {
        node = (node_t*)arena_alloc(graph->arena, sizeof(node_t));
    }
    memset(node, 0, sizeof(node_t));
    
    node->graph = graph;
    node->label = label;
    node->pos = pos;
    node->color = col;
    
    // add to list
    node_list_item_t* list_item = graph->node_list_item_free;
    if (list_item != nullptr) {
        stack_pop(graph->node_list_item_free);
    } else {
        list_item = (node_list_item_t*)arena_alloc(graph->arena, sizeof(node_list_item_t));
    }
    memset(list_item, 0, sizeof(node_list_item_t));
    list_item->node = node;
    dll_push_back(graph->node_list_first, graph->node_list_last, list_item);
    
    return node;
}

function void
graph_node_release(graph_t* graph, node_t* node) {
    
    node_list_item_t* list_item = nullptr;
    for (node_list_item_t* i = graph->node_list_first; i != nullptr; i = i->next) {
        if (i->node == node) {
            list_item = i;
            break;
        }
    }
    
    // remove from list
    if (list_item != nullptr) {
        dll_remove(graph->node_list_first, graph->node_list_last, list_item);
        
        // push to free stack
        stack_push(graph->node_list_item_free, list_item);
        stack_push_n(graph->node_free, node, stack_next);
    }
    
}

function void 
graph_node_prop_add(node_t* node, str_t label, node_prop_flags flags) {
    
    arena_t* arena = node->graph->arena;
    
    node_prop_t* prop = (node_prop_t*)arena_alloc(arena, sizeof(node_prop_t));
    
    prop->label = label;
    prop->flags = flags;
    
    dll_push_back(node->prop_first, node->prop_last, prop);
    
}

function void
graph_node_render(node_t* node) {
    
    // container node
    ui_node_flags node_flags =
        ui_flag_mouse_interactable |
        ui_flag_draw_background |
        ui_flag_draw_border |
        ui_flag_draw_shadow;
    
    ui_set_next_fixed_pos(node->pos.x, node->pos.y);
    ui_set_next_size(ui_size_pixels(150.0f), ui_size_by_children(1.0f));
    node->ui_node = ui_node_from_string( node_flags, node->label);
    
    ui_push_parent(node->ui_node);
    
    // title node
    ui_set_next_size(ui_size_percent(1.0f), ui_size_pixels(25.0f));
    ui_node_t* title_node = ui_node_from_key(ui_flag_draw_custom, { 0 });
    title_node->label = node->label;
    title_node->hover_t = node->ui_node->hover_t;
    title_node->active_t = node->ui_node->active_t;
    ui_node_set_custom_draw(title_node, graph_node_title_draw_func, node);
    
    // properties
    for (node_prop_t* prop = node->prop_first; prop != nullptr; prop = prop->next) {
        graph_node_prop_render(prop);
    }
    
    ui_pop_parent();
    
    // root interaction
    ui_interaction node_interaction = ui_interaction_from_node(node->ui_node);
    if (node_interaction & ui_left_pressed) {
        node_list_item_t* list_item = nullptr;
        for (node_list_item_t* i = node->graph->node_list_first; i != nullptr; i = i->next) {
            if (i->node == node) {
                list_item = i;
                break;
            }
        }
        
        dll_remove(node->graph->node_list_first, node->graph->node_list_last, list_item);
        dll_push_front(node->graph->node_list_first, node->graph->node_list_last, list_item);
    }
    
    if (node_interaction & ui_left_dragging) {
        vec2_t mouse_delta = os_window_get_mouse_delta(window);
        node->pos.x += mouse_delta.x;
        node->pos.y += mouse_delta.y;
    }
    
}

function void 
graph_node_title_draw_func(ui_node_t* node) {
    
    node_t* data = (node_t*)node->custom_draw_data;
    
    // background
    f32 height = rect_height(node->rect);
    rect_t top_rect = rect_cut_bottom(node->rect, roundf(height * 0.5f));
    rect_t bottom_rect = rect_cut_top(node->rect, roundf(height * 0.25f));
    
    // colors
    color_t color_main = data->color;
    color_t color_sec = color_blend(color_main, color(0x00000090));
    color_t color_effect = color_blend(color_main, color(0x292a2f50));
    color_t color_effect_sec = color_blend(color_effect, color(0x00000090));
    
    color_main = color_lerp(color_main, color_blend(color_main, color(0xffffff05)), node->hover_t);
    color_sec = color_lerp(color_sec, color_blend(color_sec, color(0xffeedd25)), node->hover_t);
    color_effect = color_lerp(color_effect, color_blend(color_effect, color(0xffeedd65)), node->hover_t);
    color_effect_sec = color_lerp(color_effect_sec, color_blend(color_effect_sec, color(0xffeedd65)), node->hover_t);
    
    color_main = color_lerp(color_main, color_blend(color_main, color(0xffffff05)), node->active_t);
    color_sec = color_lerp(color_sec, color_blend(color_sec, color(0xffffff05)), node->active_t);
    color_effect = color_lerp(color_effect, color_blend(color_effect, color(0xffeedd65)), node->active_t);
    color_effect_sec = color_lerp(color_effect_sec, color_blend(color_effect_sec, color(0xffeedd65)), node->active_t);
    
    // draw main background
    draw_push_radius0(0.0f);
    draw_push_radius1(node->rounding.y);
    draw_push_radius2(0.0f);
    draw_push_radius3(node->rounding.w);
    
    draw_set_next_color0(color_main);
    draw_set_next_color1(color_main);
    draw_set_next_color2(color_sec);
    draw_set_next_color3(color_sec);
    draw_rect(node->rect);
    
    // draw effects
    draw_set_next_color0(color_main);
    draw_set_next_color1(color_effect);
    draw_set_next_color2(color_sec);
    draw_set_next_color3(color_effect_sec);
    draw_rect(top_rect);
    
    draw_set_next_color0(color_effect);
    draw_set_next_color1(color_main);
    draw_set_next_color2(color_effect_sec);
    draw_set_next_color3(color_sec);
    draw_rect(bottom_rect);
    
    
    // border
    draw_set_next_color0(color(0xffffff35));
    draw_set_next_color1(color(0xffffff35));
    draw_set_next_color2(color(0xffffff55));
    draw_set_next_color3(color(0xffffff15));
    draw_set_next_thickness(1.0f);
    draw_rect(rect_shrink(node->rect, 1.0f));
    
    draw_set_next_color(color(0x131313ff));
    draw_set_next_thickness(1.0f);
    draw_rect(node->rect);
    
    draw_pop_rounding();
    
    // text
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

function void
graph_node_prop_render(node_prop_t* prop) {
    
    ui_set_next_size(ui_size_percent(1.0f), ui_size_by_children(1.0f));
    ui_set_next_padding(8.0f);
    ui_node_t* padding_node = ui_node_from_stringf(0, "%s##padding", prop->label.data);
    ui_push_parent(padding_node);
    
    ui_node_flags input_output_flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_background |
        ui_flag_draw_border |
        ui_flag_draw_hover_effects |
        ui_flag_draw_active_effects;
    
    if (prop->flags & node_prop_flag_input) {
        
        ui_set_next_fixed_pos(-8.0f, 13.0f);
        ui_set_next_fixed_size(15.0f, 15.0f);
        ui_set_next_rounding(vec4(6.0f));
        ui_node_t* input_node = ui_node_from_stringf( input_output_flags, "%s##input", prop->label.data);
        ui_interaction input_interaction = ui_interaction_from_node(input_node);
    }
    
    if (prop->flags & node_prop_flag_output) {
        
        ui_set_next_fixed_pos(padding_node->size.x - 7.0f, 13.0f);
        ui_set_next_fixed_size(15.0f, 15.0f);
        ui_set_next_rounding(vec4(6.0f));
        ui_node_t* output_node = ui_node_from_stringf( input_output_flags, "%s##output", prop->label.data);
        ui_interaction output_interaction = ui_interaction_from_node(output_node);
    }
    
    ui_set_next_size(ui_size_percent(1.0f), ui_size_pixels(25.0f));
    ui_float_edit(prop->label, &prop->value);
    
    ui_pop_parent();
    
}

//- app functions 

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
    
    for (i32 i = 0; i < 8; i++) {
        str_t string = str_format(arena, " Item %u", i);
        list_item_add(arena, &list, string);
    }
    
    font = font_open(str("res/fonts/deja_vu_sans.ttf"));
    
    graph = graph_create();
    node_t* node = graph_node_create(graph, str("node"), vec2(25.0f, 25.0f));
    graph_node_prop_add(node, str("test"), node_prop_flag_input | node_prop_flag_output);
    
    node_t* node2 = graph_node_create(graph, str("node 2"), vec2(225.0f, 100.0f), color(0x235634ff));
    graph_node_prop_add(node2, str("test_2"), node_prop_flag_input | node_prop_flag_output);
    
    ui_context_add_color(ui, str("background"), color(0x252629ff));
    ui_context_add_color(ui, str("shadow"), color(0x00000080));
    ui_context_add_color(ui, str("text"), color(0xe2e2e3ff));
    ui_context_add_color(ui, str("border"), color(0xffffff35));
    ui_context_add_color(ui, str("effect"), color(0x393a3fff));
    
    ui_context_add_color(ui, str("textbox background"), color(0x151619ff));
    ui_context_add_color(ui, str("textbox shadow"), color(0x00000080));
    ui_context_add_color(ui, str("textbox text"), color(0xe2e2e3ff));
    ui_context_add_color(ui, str("textbox border"), color(0xffffff35));
    ui_context_add_color(ui, str("textbox effect"), color(0x090a0fff));
    
}

function void
app_release() {
	
    graph_release(graph);
    
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
	gfx_renderer_update(renderer);
    
    
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
        
        f32 dt = os_window_get_delta_time(window);
        ui_push_size(ui_size_pixels(250.0f), ui_size_pixels(25.0f));
        ui_labelf("frame_time: %.1f ms (%.1f fps)", dt * 1000.0f, 1.0f / dt);
        ui_pop_size();
        
        //- reorderable list 
        
        ui_push_size(ui_size_pixels(150.0f), ui_size_pixels(25.0f));
        vec2_t mouse_pos = os_window_get_cursor_pos(window);
        
        b8 dragged_this_frame = false;
        for (list_item_t* item = list.first,*next = nullptr; item != nullptr; item = next) {
            next = item->next;
            
            // build root node
            
            
            // build node
            ui_node_flags node_flags = 
                ui_flag_mouse_interactable |
                ui_flag_draw_background | 
                ui_flag_draw_border | 
                ui_flag_draw_hover_effects |
                ui_flag_draw_active_effects |
                ui_flag_draw_text |
                ui_flag_anim_pos_y;
            
            ui_set_next_hover_cursor(os_cursor_hand_point);
            item->node = ui_node_from_string(node_flags, item->label);
            
            //ui_push_parent();
            //ui_spacer(ui_size_percent(1.0f));
            
            
            
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
        ui_pop_size();
        
        //- scrollable list
        
        ui_set_next_size(ui_size_pixels(250.0f), ui_size_pixels(25.0f));
        ui_labelf("scrollable list");
        
        ui_node_flags flags = 
            ui_flag_mouse_interactable |
            ui_flag_view_scroll_y |
            ui_flag_view_clamp_y | 
            ui_flag_clip ;
        
        ui_set_next_size(ui_size_pixels(250.0f), ui_size_pixels(300.0f));
        ui_node_t* scrollable_container = ui_node_from_stringf(flags, "scrollable_container");
        
        ui_push_parent(scrollable_container);
        ui_push_seed_key(scrollable_container->key);
        ui_push_size(ui_size_percent(1.0f), ui_size_pixels(25.0f));
        
        for (i32 i = 0; i < 25; i++) {
            ui_buttonf("item %i", i);
        }
        
        ui_pop_size();
        ui_pop_parent();
        ui_pop_seed_key();
        ui_interaction interaction = ui_interaction_from_node(scrollable_container);
        
        //- text box
        
        persist ui_text_point_t cursor = {1, 1};
        persist ui_text_point_t mark = {1, 1};
        persist u8 edit_buffer[128];
        persist u32 out_size;
        
        ui_set_next_tag(str("textbox"));
        ui_set_next_rect(rect(400.0f, 10.0f, 700.0f, 35.0f));
        ui_text_edit(&cursor, &mark, edit_buffer, 128, &out_size, str(""), str("text_edit"));
        
        //- color edits
        
        persist color_t hsv_col = color_rgb_to_hsv(color(0x24e3b8ff));
        ui_set_next_size(ui_size_pixels(150.0f), ui_size_pixels(150.0f));
        ui_color_sv_quad(hsv_col.h, &hsv_col.s, &hsv_col.v, str("col_edit"));
        
        //- graph
        
        ui_set_next_rect(rect(400.0f, 150.0f, 1200.0f, 650.0f));
        ui_canvas_begin(str("canvas"));
        
        for (node_list_item_t* item = graph->node_list_first, *next = nullptr; item != nullptr; item = next) {
            next = item->next;
            graph_node_render(item->node);
        }
        
        ui_canvas_end();
        
        ui_pop_font_size();
        ui_pop_font();
        ui_end(ui);
        
        //- draw ui 
        draw_begin(renderer);
        ui_context_set_active(ui);
        
        for (ui_node_t* node = ui->node_root; node != nullptr;) {
            ui_node_rec_t rec = ui_node_rec_depth_first(node);
            
            // clipping
            if (node->flags & ui_flag_clip) {
                rect_t top_clip = draw_top_clip_mask();
                rect_t new_clip = node->rect;
                if (top_clip.x1 != 0.0f || top_clip.y1 != 0.0f) {
                    new_clip = rect_intersection(new_clip, top_clip);
                }
                rect_validate(new_clip);
                draw_push_clip_mask(new_clip);
            }
            
            // shadow
            if (node->flags & ui_flag_draw_shadow) {
                draw_set_next_color(color(0x00000090));
                draw_set_next_softness(6.0f);
                draw_rect(rect_translate(rect_grow(node->rect, 4.0f), 4.0f));
            }
            
            // background
            if (node->flags & ui_flag_draw_background) {
                
                // colors
                color_t color_background = ui_color_from_key(ui_key_from_string(node->tags_key, str("background")));
                color_t color_effect = ui_color_from_key(ui_key_from_string(node->tags_key, str("effect")));
                
                if (node->flags & ui_flag_draw_hover_effects) {
                    color_background = color_lerp(color_background, color_blend(color_background, color(0xffffff05)), node->hover_t);
                    color_effect = color_lerp(color_effect, color_blend(color_effect, color(0xffeedd25)), node->hover_t);
                }
                
                if (node->flags & ui_flag_draw_active_effects) {
                    color_background = color_lerp(color_background, color_blend(color_background, color(0xffffff05)), node->active_t);
                    color_effect = color_lerp(color_effect, color_blend(color_effect, color(0xffeedd65)), node->active_t);
                }
                
                // draw main background
                draw_set_next_color(color_background);
                draw_set_next_rounding(node->rounding);
                draw_rect(node->rect);
                
                // draw effects
                if (node->flags & ui_flag_draw_hover_effects | ui_flag_draw_active_effects) {
                    f32 height = rect_height(node->rect);
                    f32 height_top = 0.5f;
                    f32 height_bot = 0.25f;
                    
                    if (ui_key_equals(ui_key_from_string({ 0 }, str("textbox")), node->tags_key)) {
                        height_top = 0.25f;
                        height_bot = 0.5f;
                    }
                    
                    rect_t top_rect = rect_cut_bottom(node->rect, roundf(height * height_top));
                    rect_t bottom_rect = rect_cut_top(node->rect, roundf(height * height_bot));
                    color_t color_transparent = color_effect;
                    color_transparent.a = 0.0f;
                    
                    draw_set_next_color0(color_transparent);
                    draw_set_next_color1(color_effect);
                    draw_set_next_color2(color_transparent);
                    draw_set_next_color3(color_effect);
                    draw_set_next_rounding(vec4(node->rounding.x, 0.0f, node->rounding.z, 0.0f));
                    draw_rect(top_rect);
                    
                    draw_set_next_color0(color_effect);
                    draw_set_next_color1(color_transparent);
                    draw_set_next_color2(color_effect);
                    draw_set_next_color3(color_transparent);
                    draw_set_next_rounding(vec4(0.0f, node->rounding.y, 0.0f, node->rounding.w));
                    draw_rect(bottom_rect);
                }
                
            }
            
            // border
            if (node->flags & ui_flag_draw_border) {
                
                color_t border_main_color = ui_color_from_key(ui_key_from_string(node->tags_key, str("border")));
                color_t border_highlight_color = color_blend(border_main_color, color(0xffffff35));
                color_t border_lowlight_color = color_blend(border_main_color, color(0x00000035));
                
                draw_set_next_color0(border_main_color);
                draw_set_next_color1(border_main_color);
                draw_set_next_color2(border_highlight_color);
                draw_set_next_color3(border_lowlight_color);
                draw_set_next_rounding(node->rounding);
                draw_set_next_thickness(1.0f);
                draw_rect(rect_shrink(node->rect, 1.0f));
                
                color_t border_dark_color = ui_color_from_key(ui_key_from_string(node->tags_key, str("background")));
                border_dark_color = color(0x131313ff);
                
                draw_set_next_color(border_dark_color);
                draw_set_next_rounding(node->rounding);
                draw_set_next_thickness(1.0f);
                draw_rect(node->rect);
            }
            
            // text
            if (node->flags & ui_flag_draw_text) {
                color_t shadow_color = ui_color_from_key(ui_key_from_string(node->tags_key, str("shadow")));
                color_t text_color = ui_color_from_key(ui_key_from_string(node->tags_key, str("text")));
                vec2_t text_pos = ui_text_align(node->font, node->font_size, node->label, node->rect, node->text_alignment);
                
                draw_push_font(node->font);
                draw_push_font_size(node->font_size);
                
                draw_set_next_color(shadow_color);
                draw_text(node->label, vec2_add(text_pos, 1.0f));
                
                draw_set_next_color(text_color);
                draw_text(node->label, text_pos);
                
                draw_pop_font();
                draw_pop_font_size();
            }
            
            // custom draw
            if (node->flags & ui_flag_draw_custom) {
                if (node->custom_draw_func != nullptr) {
                    node->custom_draw_func(node);
                }
            }
            
            // pop clipping
            i32 pop_index = 0;
            for (ui_node_t* n = node; n != nullptr && pop_index <= rec.pop_count; n = n->tree_parent) {
                pop_index++;
                if (n == node && rec.push_count != 0) { continue; }
                if (n->flags & ui_flag_clip) { draw_pop_clip_mask(); }
            }
            
            node = rec.next;
        }
        
        ui_context_set_active(nullptr);
        draw_end(renderer);
        
        gfx_renderer_end(renderer);
    }
}

//~ entry point

function i32
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    gfx_init();
    font_init();
    draw_init();
    ui_init();
    
    // init
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    // release layers
    ui_release();
    draw_release();
    font_release();
    gfx_release();
    os_release();
    
    return 0;
}