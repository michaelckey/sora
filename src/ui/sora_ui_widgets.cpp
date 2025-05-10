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
        ui_node_set_custom_draw(edit_string_node, ui_text_edit_draw_func, draw_data);
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

function ui_interaction 
ui_float_edit(str_t label, f32* value, f32 delta, f32 min, f32 max) {
    
    ui_context_t* context = ui_active_context;
    
    // keys
    ui_key_t slider_key = ui_key_from_string(ui_top_seed_key(), label);
    ui_key_t increment_key = ui_key_from_stringf(slider_key, "%s##increment", label.data);
    ui_key_t decrement_key = ui_key_from_stringf(slider_key, "%s##decrement", label.data);
    
    
    // create parent
    ui_node_flags parent_flags = 
        ui_flag_draw_background |
        ui_flag_draw_border;
    
    ui_set_next_layout_dir(ui_dir_right);
    ui_node_t* parent_node = ui_node_from_key(parent_flags, { 0 });
    
    // determine if we should show arrows
    b8 hovered = false;
    ui_key_t last_hovered_key = context->key_hovered_prev;
    if (ui_key_equals(last_hovered_key, decrement_key) || ui_key_is_active(decrement_key) ||
        ui_key_equals(last_hovered_key, increment_key) || ui_key_is_active(increment_key) ||
        ui_key_equals(last_hovered_key, slider_key) || ui_key_is_active(slider_key)) {
        hovered = true;
    }
    
    ui_push_parent(parent_node);
    
    b8 modifier_shift = os_get_modifiers() & os_modifier_shift;
    f32 adjusted_delta = delta * (modifier_shift ? 10.0f : 1.0f);
    
    // decrement and increment flags
    u32 inc_dec_flags = ui_flag_mouse_interactable;
    
    // hide if not hovered
    if (hovered) {
        inc_dec_flags |= 
            ui_flag_draw_background |
            ui_flag_draw_border |
            ui_flag_draw_hover_effects |
            ui_flag_draw_active_effects |
            ui_flag_draw_text;
    }
    
    // decrement
    ui_set_next_font(ui_font_icon);
    ui_set_next_text_alignment(ui_text_align_center);
    ui_set_next_size(ui_size_pixels(15.0f), ui_size_percent(1.0f));
    ui_set_next_rounding_00(0.0f);
    ui_set_next_rounding_01(0.0f);
    
    ui_node_t* decrement_node = ui_node_from_key(inc_dec_flags, decrement_key);
    decrement_node->label = str("N");
    ui_interaction decrement_interaction = ui_interaction_from_node(decrement_node);
    
    if (decrement_interaction & ui_left_clicked) {
        *value -= adjusted_delta;
    }
    
    // slider/text_edit
    ui_node_flags slider_flags = ui_flag_interactable | ui_flag_draw_text;
    
    if (hovered) {
        slider_flags |= 
            ui_flag_draw_background |
            ui_flag_draw_border |
            ui_flag_draw_hover_effects |
            ui_flag_draw_active_effects;
    }
    
    str_t number_text = str_format(ui_build_arena(), "%.2f", *value);
    ui_set_next_size(ui_size_percent(1.0f), ui_size_percent(1.0f));
    ui_set_next_text_alignment(ui_text_align_center);
    ui_set_next_hover_cursor(os_cursor_resize_EW);
    ui_set_next_rounding(vec4(0.0f));
    ui_node_t* slider_node = ui_node_from_key(slider_flags, slider_key);
    slider_node->label = number_text;
    ui_interaction slider_interaction = ui_interaction_from_node(slider_node);
    
    // slider interatcion
    if (slider_interaction & ui_left_dragging) {
        *value = *value + (adjusted_delta * context->mouse_delta.x);
        
        // don't clamp if everything equals 0.0f
        if (min != max != 0.0f) {
            *value = clamp(*value, min, max);
        }
    }
    
    // increment
    ui_set_next_font(ui_font_icon);
    ui_set_next_text_alignment(ui_text_align_center);
    ui_set_next_size(ui_size_pixels(15.0f), ui_size_percent(1.0f));
    ui_set_next_rounding_10(0.0f);
    ui_set_next_rounding_11(0.0f);
    
    ui_node_t* increment_node = ui_node_from_key(inc_dec_flags, increment_key);
    increment_node->label = str("O");
    ui_interaction increment_interaction = ui_interaction_from_node(increment_node);
    
    if (increment_interaction & ui_left_clicked) {
        *value += adjusted_delta;
    }
    
    ui_pop_parent();
    
    ui_interaction interaction = 0;
    return interaction;
}



function ui_interaction 
ui_color_sv_quad(f32 hue, f32* sat, f32* val, str_t label) {
    
    // build node
    ui_node_flags flags =
        ui_flag_interactable |
        ui_flag_draw_border |
        ui_flag_draw_custom;
    
    ui_set_next_hover_cursor(os_cursor_hand_point);
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), label);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    
    // set custom draw function and data
    vec3_t* hsv_data = (vec3_t*)arena_alloc(ui_build_arena(), sizeof(vec3_t));
    ui_node_set_custom_draw(node, ui_color_sv_draw_func, hsv_data);
    
    // do interaction
    ui_interaction interaction = ui_interaction_from_node(node);
    f32 target_sat = *sat;
    f32 target_val = *val;
    
    if (interaction & ui_left_dragging) {
        vec2_t mouse_pos = ui_active_context->mouse_pos;
        
        f32 node_width = rect_width(node->rect);
        f32 node_height = rect_height(node->rect);
        
        target_sat = (mouse_pos.x - node->rect.x0) / node_width;
        target_val = 1.0f - (mouse_pos.y - node->rect.y0) / node_height;
        target_sat = clamp_01(target_sat);
        target_val = clamp_01(target_val);
    }
    
    // animate
    *sat = ui_anim(ui_key_from_stringf(node_key, "anim_sat"), target_sat, target_sat);
    *val = ui_anim(ui_key_from_stringf(node_key, "anim_val"), target_val, target_val);
    
    // set draw data
    *hsv_data = vec3(hue, *sat, *val);
    
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

function ui_node_t*
ui_padding_begin(f32 size) {
    ui_set_next_padding(size);
    ui_node_t* node = ui_node_from_key(0, { 0 });
    ui_push_parent(node);
    return node;
}

function ui_interaction
ui_padding_end() {
    ui_node_t* node = ui_pop_parent();
    ui_interaction interaction = ui_interaction_from_node(node);
    return interaction;
}


function ui_node_t* 
ui_canvas_begin(str_t label) {
    
    ui_node_flags flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_custom |
        ui_flag_overflow |
        ui_flag_clip;
    
    ui_node_t* node = ui_node_from_string(flags, label);
    ui_node_set_custom_draw(node, ui_canvas_draw_func, nullptr);
    
    ui_push_seed_key(node->key);
    ui_push_parent(node);
    
    return node;
}

function ui_interaction
ui_canvas_end() {
    ui_pop_seed_key();
    ui_node_t* canvas_node = ui_pop_parent();
    ui_interaction interaction = ui_interaction_from_node(canvas_node);
    
    if (interaction & ui_middle_dragging) {
        vec2_t mouse_delta = ui_active_context->mouse_delta;
        canvas_node->view_offset_target.x -= mouse_delta.x;
        canvas_node->view_offset_target.y -= mouse_delta.y;
        canvas_node->hover_cursor = os_cursor_resize_all;
    }
    
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
    //draw_set_next_color(color(0x93E7FF80));
    //draw_set_next_rounding(1.0f);
    //draw_rect(select_rect);
    
    // draw cursor
    //draw_set_next_color(color(0x93E7FFff));
    //draw_set_next_rounding(1.0f);
    //draw_rect(cursor_rect);
    
}

function void
ui_color_sv_draw_func(ui_node_t* node) {
    
    // get data
    vec3_t* hsv_color = (vec3_t*)node->custom_draw_data;
    
    // convert colors
    color_t hue_color = color_hsv_to_rgb(color(hsv_color->x, 1.0f, 1.0f, 1.0f));
    color_t rgb_color = color_hsv_to_rgb(color(hsv_color->x, hsv_color->y, hsv_color->z, 1.0f));
    
    // node info
    f32 node_width = rect_width(node->rect);
    f32 node_height = rect_height(node->rect);
    
    // draw hue quad
    //draw_set_next_color0(color(0xffffffff));
    //draw_set_next_color1(color(0x000000ff));
    //draw_set_next_color2( hue_color);
    //draw_set_next_color3(color(0x000000ff));
    //draw_set_next_rounding(node->rounding);
    //draw_rect(node->rect);
    
    // draw indicator
    vec2_t indicator_pos = vec2(node->rect.x0 + (hsv_color->y * node_width), node->rect.y0 + ((1.0f - hsv_color->z) * node_height));
    
    f32 indicator_size = 6.0f;
    indicator_size = lerp(indicator_size, indicator_size + 2.0f, node->hover_t);
    indicator_size = lerp(indicator_size, indicator_size + 2.0f, node->active_t);
    
    // borders
    //draw_set_next_color(color(0x151515ff));
    //draw_circle(indicator_pos, indicator_size + 2.0f, 0.0f, 360.0f);
    
    //draw_set_next_color(color(0xe2e2e2ff));
    //draw_circle(indicator_pos, indicator_size + 1.0f, 0.0f, 360.0f);
    
    // color
    //draw_set_next_color(rgb_color);
    //draw_circle(indicator_pos, indicator_size, 0.0f, 360.0f);
    
    
}

function void 
ui_canvas_draw_func(ui_node_t* node) {
    
    //draw_set_next_rounding(node->rounding);
    //draw_set_next_color(color(0x09090aff));
    //draw_rect(node->rect);
    
    f32 offset_x = fmodf(node->view_offset_prev.x, 25.0f);
	f32 offset_y = fmodf(node->view_offset_prev.y, 25.0f);
	for (f32 x = node->rect.x0 - offset_x; x < node->rect.x1 - offset_x + 25.0f; x += 25.0f) {
		for (f32 y = node->rect.y0 - offset_y; y < node->rect.y1 - offset_y + 25.0f; y += 25.0f) {
            f32 radius = 1.25f;
            //draw_set_next_color(color(0x303031ff));
            //draw_circle(vec2(x, y), radius, 0.0f, 360.0f);
		}
	}
    
}


#endif // SORA_UI_WIDGETS_CPP