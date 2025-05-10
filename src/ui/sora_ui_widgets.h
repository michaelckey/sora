// sora_ui_widgets.h

#ifndef SORA_UI_WIDGETS_H
#define SORA_UI_WIDGETS_H

//- structs 

struct ui_text_edit_draw_data_t {
    str_t edit_string;
    f32 cursor_pos;
    f32 cursor_pos_delay;
    f32 mark_pos;
};

//- functions 

function void ui_spacer(ui_size_t size = ui_size_pixels(2.0f));

function ui_interaction ui_label(str_t string);
function ui_interaction ui_labelf(char* fmt, ...);

function ui_interaction ui_button(str_t string);
function ui_interaction ui_buttonf(char* fmt, ...);

function ui_interaction ui_text_edit(ui_text_point_t *cursor, ui_text_point_t* mark, u8* edit_buffer, u32 edit_buffer_size, u32 *edit_string_size_out, str_t pre_edit_value, str_t string);
function ui_interaction ui_float_edit(str_t label, f32* value, f32 delta = 0.01f, f32 min = 0.0f, f32 max = 0.0f);

function ui_interaction ui_color_sv_quad(f32 hue, f32* sat, f32* val, str_t label);

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

// draw 

function void ui_text_edit_draw_func(ui_node_t* node);

function void ui_color_sv_draw_func(ui_node_t* node);

function void ui_canvas_draw_func(ui_node_t* node);

#endif // SORA_UI_WIDGETS_H