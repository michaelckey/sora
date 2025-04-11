// sora_ui_widgets.cpp

#ifndef SORA_UI_WIDGETS_CPP
#define SORA_UI_WIDGETS_CPP

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
        ui_flag_draw_active_effects |
        ui_flag_draw_text;
    
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), string);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    node->label = string;
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

function ui_interaction 
ui_text_edit(ui_text_point_t *cursor, ui_text_point_t* mark, u8* edit_buffer, u32 edit_buffer_size, u32 *edit_string_size_out, str_t pre_edit_value, str_t string) {
    
    ui_node_flags flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_background |
        ui_flag_draw_border |
        ui_flag_draw_hover_effects |
        ui_flag_clip |
        ui_flag_overflow_x |
        ui_flag_view_clamp;
    
    ui_set_next_hover_cursor(os_cursor_I_beam);
    ui_key_t key = ui_key_from_string(ui_top_seed_key(), string);
    ui_node_t* node = ui_node_from_key(flags, key);
    
    b8 is_focused = ui_key_equals(ui_active_context->key_focused, key);
    
    b8 changes_made = false;
    if (is_focused) {
        temp_t scratch = scratch_begin();
        
        for (ui_event_t* event = ui_active_context->event_list.first; event != nullptr; event = event->next) {
            str_t edit_string = str((char*)edit_buffer, *edit_string_size_out);
            
            if ((event->type != ui_event_type_edit && event->type != ui_event_type_navigate && event->type != ui_event_type_text) || event->delta.y != 0) {
                continue;
            }
            
            ui_text_op_t text_op = ui_single_line_text_op_from_event(scratch.arena, event, edit_string, *cursor, *mark);
            
            // replace 
            if(!ui_text_point_equals(text_op.range.min, text_op.range.max) || text_op.replace.size != 0) {
                ivec2_t range = ivec2(text_op.range.min.column, text_op.range.max.column);
                str_t new_string = str_replace_range(scratch.arena, edit_string, range, text_op.replace);
                new_string.size = min(edit_buffer_size, new_string.size);
                memcpy(edit_buffer, new_string.data, new_string.size);
                *edit_string_size_out = new_string.size;
            }
            
            *cursor = text_op.cursor;
            *mark = text_op.mark;
            
            ui_event_pop(event);
            changes_made = true;
            
        }
        
        scratch_end(scratch);
    }
    
    str_t edit_string = str((char*)edit_buffer, *edit_string_size_out);
    f32 cursor_pos = font_text_get_width(node->font, node->font_size, str_prefix(edit_string, cursor->column-1));
    f32 mark_pos = font_text_get_width(node->font, node->font_size, str_prefix(edit_string, mark->column-1));
    
    ui_push_parent(node);
    
    ui_set_next_size(ui_size_percent(1.0f), ui_size_percent(1.0f));
    ui_key_t edit_string_key = ui_key_from_string(key, str("###edit_string"));
    ui_node_t* edit_string_node = ui_node_from_key(ui_flag_draw_text, edit_string_key);
    edit_string_node->label = edit_string;
    
    if (is_focused) {
        edit_string_node->flags |= ui_flag_draw_custom;
        ui_text_edit_draw_data_t* draw_data = (ui_text_edit_draw_data_t*)arena_alloc(ui_build_arena(), sizeof( ui_text_edit_draw_data_t));
        ui_node_set_custom_draw_func(edit_string_node, ui_text_edit_draw_func, draw_data);
        draw_data->edit_string = edit_string;
        
        // animate
        draw_data->cursor_pos = ui_anim(ui_key_from_stringf(key, "anim_cursor_rapid"), cursor_pos, cursor_pos, ui_active_context->anim_rapid_rate);
        draw_data->cursor_pos_delay = ui_anim(ui_key_from_stringf(key, "anim_cursor_fast"), cursor_pos, cursor_pos, ui_active_context->anim_slow_rate);
        draw_data->mark_pos = ui_anim(ui_key_from_stringf(key, "anim_mark_pos"), mark_pos, mark_pos, ui_active_context->anim_rapid_rate);
        
    }
    
    ui_pop_parent();
    
    // interact
    ui_interaction interaction = ui_interaction_from_node(node);
    
    if (interaction & ui_left_pressed) {
        
    }
    
    // keep cursor in view
    
    vec2_t cursor_range = vec2(cursor_pos - 20.0f, cursor_pos + 20.0f);
    vec2_t visible_range = vec2(node->view_offset_target.x, node->view_offset_target.x + rect_width(node->rect));
    
    cursor_range.x = max(0, cursor_range.x);
    cursor_range.y = max(0, cursor_range.y);
    
    f32 min_delta = cursor_range.x - visible_range.x;
    f32 max_delta = cursor_range.y - visible_range.y;
    min_delta = min(0, min_delta);
    max_delta = max(0, max_delta);
    
    node->view_offset_target.x += min_delta;
    node->view_offset_target.x += max_delta;
    
    return interaction;
}


//- layout functions

function ui_node_t* 
ui_row_begin() {
    ui_set_next_layout_dir(ui_dir_right);
    ui_node_t* node = ui_node_from_key(0, { 0 });
    ui_push_parent(node);
    return node;
}

function ui_interaction
ui_row_end() {
    ui_node_t* node = ui_pop_parent();
    ui_interaction interaction = ui_interaction_from_node(node);
    return interaction;
}

function ui_node_t* 
ui_column_begin() {
    ui_set_next_layout_dir(ui_dir_down);
    ui_node_t* node = ui_node_from_key(0, { 0 });
    ui_push_parent(node);
    return node;
}

function ui_interaction
ui_column_end() {
    ui_node_t* node = ui_pop_parent();
    ui_interaction interaction = ui_interaction_from_node(node);
    return interaction;
}


//- draw functions 

function void
ui_text_edit_draw_func(ui_node_t* node) {
    
    ui_text_edit_draw_data_t* data = (ui_text_edit_draw_data_t*)node->custom_draw_data;
    
    vec2_t text_pos = ui_text_align(node->font, node->font_size, data->edit_string, node->rect, node->text_alignment);
    
    f32 cursor_left = data->cursor_pos_delay;
    f32 cursor_right = data->cursor_pos;
    if (data->cursor_pos < data->cursor_pos_delay) {
        cursor_left = data->cursor_pos;
        cursor_right = data->cursor_pos_delay;
    }
    
    rect_t cursor_rect = rect(text_pos.x + cursor_left + 0.0f, node->rect.y0 + 3.0f,
                              text_pos.x + cursor_right + 2.0f, node->rect.y1 - 3.0f);
    rect_t select_rect = rect(text_pos.x + cursor_right + 0.0f, node->rect.y0 + 4.0f,
                              text_pos.x + data->mark_pos + 0.0f, node->rect.y1 - 4.0f);
    
    // draw select
    draw_set_next_color(color(0x93E7FF80));
    draw_set_next_rounding(1.0f);
    draw_rect(select_rect);
    
    // draw cursor
    draw_set_next_color(color(0x93E7FFff));
    draw_set_next_rounding(1.0f);
    draw_rect(cursor_rect);
    
}


#endif // SORA_UI_WIDGETS_CPP