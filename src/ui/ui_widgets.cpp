// ui_widgets.cpp

#ifndef UI_WIDGETS_CPP
#define UI_WIDGETS_CPP

//- implementation 

function void
ui_spacer(ui_size_t size) {
    ui_node_t* parent = ui_top_parent();
    ui_axis axis = ui_axis_from_dir(parent->layout_dir);
    (axis == ui_axis_x) ? ui_set_next_size_x(size) : ui_set_next_size_y(size);
    ui_node_t* frame = ui_node_from_key(0, { 0 });
}

function ui_interaction
ui_label(str_t string) {
    
    ui_node_flags flags = 
        ui_flag_draw_text;
    
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), string);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    node->label = string;
    
    ui_interaction result = ui_interaction_from_node(node);
    
    return result;
}

function ui_interaction
ui_labelf(char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_interaction result = ui_label(string);
    
    return result;
}


function ui_interaction
ui_button(str_t string) {
    
    ui_node_flags flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_background |
        ui_flag_draw_border |
        ui_flag_draw_hover_effects |
        ui_flag_draw_active_effects;
    
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), string);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    ui_interaction interaction = ui_interaction_from_node(node);
    
    return interaction;
}

function ui_interaction
ui_buttonf(char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_interaction result = ui_button(string);
    
    return result;
}


#endif // UI_WIDGETS_CPP