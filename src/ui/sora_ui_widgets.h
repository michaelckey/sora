// sora_ui_widgets.h

#ifndef SORA_UI_WIDGETS_H
#define SORA_UI_WIDGETS_H

//- functions 

function void ui_spacer(ui_size_t size = ui_size_pixels(2.0f));

function ui_interaction ui_label(str_t string);
function ui_interaction ui_labelf(char* fmt, ...);

function ui_interaction ui_button(str_t string);
function ui_interaction ui_buttonf(char* fmt, ...);

// layout

function ui_node_t* ui_row_begin();
function ui_interaction ui_row_end();

function ui_node_t* ui_column_begin();
function ui_interaction ui_column_end();

#endif // SORA_UI_WIDGETS_H