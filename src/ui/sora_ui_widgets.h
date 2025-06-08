// sora_ui_widgets.h

#ifndef SORA_UI_WIDGETS_H
#define SORA_UI_WIDGETS_H

//- typedefs

struct ui_view_t;
typedef void ui_view_function(ui_view_t*);

//- structs 

struct ui_text_edit_draw_data_t {
    str_t edit_string;
    f32 cursor_pos;
    f32 cursor_pos_delay;
    f32 mark_pos;
};

struct ui_panel_t;

struct ui_view_t {
    
    // global list
    ui_view_t* global_next;
    ui_view_t* global_prev;
    
    ui_view_t* next;
    ui_view_t* prev;
    
    ui_panel_t* panel;
    rect_t content_rect;
    
    str_t name;
    ui_view_function* view_func;
    
    ui_node_t* node;
    
};

struct ui_view_drag_data_t {
    ui_view_t* current;
    ui_view_t* prev;
    f32 offset;
};


struct ui_panel_t {
    
    // global list
    ui_panel_t* global_next;
    ui_panel_t* global_prev;
    
    // tree
    ui_panel_t* tree_next;
    ui_panel_t* tree_prev;
    ui_panel_t* tree_parent;
    ui_panel_t* tree_first;
    ui_panel_t* tree_last;
    
    f32 percent_of_parent;
    ui_axis split_axis;
    
    ui_node_t* node;
    
    // views
    ui_view_t* view_first;
    ui_view_t* view_last;
    ui_view_t* view_focus;
    
};

struct ui_panel_rec_t {
    ui_panel_t* next;
    u32 push_count;
    u32 pop_count;
};

struct ui_panel_rect_t {
    ui_panel_rect_t* next;
    rect_t rect;
};

struct ui_dockspace_t {
    
    arena_t* arena;
    ui_context_t* context;
    
    ui_panel_t* panel_first;
    ui_panel_t* panel_last;
    ui_panel_t* panel_root;
    ui_panel_t* panel_free;
    
    ui_view_t* view_first;
    ui_view_t* view_last;
    ui_view_t* view_free;
    ui_view_t* view_focus;
    ui_view_t* view_drag;
    
};

//- functions 

function void ui_spacer(ui_size_t size = ui_size_pixels(2.0f));

function ui_interaction ui_label(str_t string);
function ui_interaction ui_labelf(char* fmt, ...);

function ui_interaction ui_button(str_t string);
function ui_interaction ui_buttonf(char* fmt, ...);

function ui_interaction ui_icon_button(ui_icon icon);

function ui_interaction ui_slider(f32* value, f32 min, f32 max, str_t string);
function ui_interaction ui_sliderf(f32* value, f32 min, f32 max, char* fmt, ...);

function ui_interaction ui_slider(i32* value, i32 min, i32 max, str_t label);
function ui_interaction ui_sliderf(i32* value, i32 min, i32 max, char* fmt, ...);

function ui_interaction ui_text_edit(ui_text_point_t *cursor, ui_text_point_t* mark, u8* edit_buffer, u32 edit_buffer_size, u32 *edit_string_size_out, str_t pre_edit_value, str_t string);

function ui_interaction ui_float_edit(f32* value, f32 delta, f32 min, f32 max, str_t label);
function ui_interaction ui_float_editf(f32* value, f32 delta, f32 min, f32 max, char* fmt, ...);

function void ui_vec3_edit(vec3_t* value, f32 delta, str_t label);
function void ui_vec3_editf(vec3_t* value, f32 delta, char* fmt, ...);

function ui_node_t* ui_color_indicator(color_t col, vec2_t pos);
function ui_interaction ui_color_sv_quad(color_t* col, str_t label);
function ui_interaction ui_color_hue_bar(color_t* col, str_t label);
function ui_interaction ui_color_sat_bar(color_t* col, str_t label);
function ui_interaction ui_color_val_bar(color_t* col, str_t label);
function ui_interaction ui_color_alpha_bar(color_t* col, str_t label);

function ui_interaction ui_graph(void* data, u32 data_count, str_t label);


// misc

function void ui_tooltip_begin();
function void ui_tooltip_end();

function b8 ui_popup_begin(ui_key_t key);
function void ui_popup_end();

function void ui_popup_open(ui_key_t key, vec2_t pos);
function void ui_popup_close();

// dockspace

function ui_dockspace_t* ui_dockspace_create(ui_context_t* context);
function void ui_dockspace_release(ui_dockspace_t* dockspace);

function void ui_dockspace_begin(ui_dockspace_t* dockspace,  rect_t dockspace_rect);
function void ui_dockspace_end(ui_dockspace_t* dockspace);

// panel

function ui_panel_t* ui_panel_create(ui_dockspace_t* dockspace, f32 percent, ui_axis axis);
function void ui_panel_release(ui_dockspace_t* dockspace, ui_panel_t* panel);

function void ui_panel_insert(ui_panel_t* panel, ui_panel_t* parent, ui_panel_t* prev = nullptr);
function void ui_panel_remove(ui_panel_t* panel, ui_panel_t* parent);

function ui_panel_rec_t ui_panel_rec_depth_first(ui_panel_t* panel);

function rect_t _ui_rect_from_panel(ui_panel_t* panel, rect_t parent_rect);
function rect_t _ui_rect_from_panel_child(ui_panel_t* parent, ui_panel_t* panel, rect_t parent_rect);

// view

function ui_view_t* ui_view_create(ui_dockspace_t* dockspace, str_t name, ui_view_function* view_func);
function void ui_view_release(ui_dockspace_t* dockspace, ui_view_t* view);
function void ui_view_insert(ui_view_t* view, ui_panel_t* panel, ui_view_t* prev = nullptr);
function void ui_view_remove(ui_view_t* view, ui_panel_t* panel);


// layout

function ui_node_t* ui_row_begin();
function ui_interaction ui_row_end();

function ui_node_t* ui_column_begin();
function ui_interaction ui_column_end();

function ui_node_t* ui_padding_begin(f32 size);
function ui_interaction ui_padding_end();

function ui_node_t* ui_scroll_list_begin();
function ui_interaction ui_scroll_list_end();

function ui_node_t* ui_canvas_begin(str_t label);
function ui_interaction ui_canvas_end();

// basic widget draw 
function void ui_slider_draw(ui_node_t* node);
function void ui_text_edit_draw(ui_node_t* node);

// color widget draw
function void ui_color_indicator_draw(ui_node_t* node);
function void ui_color_sv_quad_draw(ui_node_t* node);
function void ui_color_hue_bar_draw(ui_node_t* node);
function void ui_color_sat_bar_draw(ui_node_t* node);
function void ui_color_val_bar_draw(ui_node_t* node);
function void ui_color_alpha_bar_draw(ui_node_t* node);

function void ui_canvas_draw(ui_node_t* node);
function void ui_graph_draw(ui_node_t* node);

function void ui_view_button_draw(ui_node_t* node);

// theme editor demo

function void ui_theme_editor();
function void ui_color_button_draw(ui_node_t* node);


function void ui_debug();


#endif // SORA_UI_WIDGETS_H