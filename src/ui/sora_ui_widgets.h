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

// layout

function ui_node_t* ui_row_begin();
function ui_interaction ui_row_end();

function ui_node_t* ui_column_begin();
function ui_interaction ui_column_end();

// draw 

function void ui_text_edit_draw_func(ui_node_t* node);

#endif // SORA_UI_WIDGETS_H