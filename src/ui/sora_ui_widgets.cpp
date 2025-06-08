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
    ui_node_set_display_string(node, string);
    
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
    ui_node_set_display_string(node, string);
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
ui_slider(f32* value, f32 min, f32 max, str_t string) {
    
    ui_node_flags flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_background |
        ui_flag_draw_border |
        ui_flag_draw_hover_effects |
        ui_flag_draw_active_effects |
        ui_flag_draw_custom;
    
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), string);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    ui_node_set_display_string(node, string);
    
    // set display text
    str_t text = str_format(ui_build_arena(), "%s: %.2f", string.data, *value);
    ui_node_set_display_string(node, text);
    
    // set custom draw function and data
    f32* percent = (f32*)arena_alloc(ui_build_arena(), sizeof(f32));
    ui_node_set_custom_draw(node, ui_slider_draw, percent);
    
    // do interaction
    ui_interaction interaction = ui_interaction_from_node(node);
    if (interaction & ui_left_dragging) {
        vec2_t mouse_pos = ui_active_context->mouse_pos;
        *value = remap(mouse_pos.x, node->rect.x0, node->rect.x1, min, max);
        *value = clamp(*value, min, max);
    }
    
    f32 percent_target = remap(*value, min, max, 0.0f, 1.0f);
    ui_key_t anim_key = ui_key_from_stringf(node_key, "anim_percent");
    f32 anim_percent = ui_anim_ex(anim_key, ui_anim_params_create(percent_target, percent_target));
    
    *percent = anim_percent;
    
    return interaction;
}

function ui_interaction
ui_sliderf(f32* value, f32 min, f32 max, char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_interaction result = ui_slider(value, min, max, string);
    
    return result;
}


function ui_interaction
ui_slider(i32* value, i32 min, i32 max, str_t string) {
    
    ui_node_flags flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_background |
        ui_flag_draw_border |
        ui_flag_draw_hover_effects |
        ui_flag_draw_active_effects |
        ui_flag_draw_custom;
    
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), string);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    ui_node_set_display_string(node, string);
    
    // set display text
    str_t text = str_format(ui_build_arena(), "%s: %i", string.data, *value);
    ui_node_set_display_string(node, text);
    
    // set custom draw function and data
    f32* percent = (f32*)arena_alloc(ui_build_arena(), sizeof(f32));
    ui_node_set_custom_draw(node, ui_slider_draw, percent);
    
    // do interaction
    ui_interaction interaction = ui_interaction_from_node(node);
    if (interaction & ui_left_dragging) {
        vec2_t mouse_pos = ui_active_context->mouse_pos;
        *value = remap(mouse_pos.x, node->rect.x0, node->rect.x1, min, max);
        *value = clamp(*value, min, max);
    }
    
    f32 percent_target = remap(*value, min, max, 0.0f, 1.0f);
    ui_key_t anim_key = ui_key_from_stringf(node_key, "anim_percent");
    f32 anim_percent = ui_anim_ex(anim_key, ui_anim_params_create(percent_target, percent_target));
    
    *percent = anim_percent;
    
    return interaction;
}

function ui_interaction
ui_sliderf(i32* value, i32 min, i32 max, char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_interaction result = ui_slider(value, min, max, string);
    
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
    
    b8 is_focused_active = ui_key_equals(ui_active_context->key_focused, key) || ui_key_equals(ui_active_context->key_active[os_mouse_button_left], key);
    
    b8 changes_made = false;
    if (is_focused_active) {
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
    
    ui_set_next_text_alignment(ui_text_align_left);
    ui_set_next_size(ui_size_percent(1.0f), ui_size_percent(1.0f));
    ui_key_t edit_string_key = ui_key_from_string(key, str("###edit_string"));
    ui_node_t* edit_string_node = ui_node_from_key(ui_flag_draw_text, edit_string_key);
    edit_string_node->label = edit_string;
    
    if (is_focused_active) {
        
        edit_string_node->flags |= ui_flag_draw_custom;
        ui_text_edit_draw_data_t* draw_data = (ui_text_edit_draw_data_t*)arena_alloc(ui_build_arena(), sizeof( ui_text_edit_draw_data_t));
        ui_node_set_custom_draw(edit_string_node, ui_text_edit_draw, draw_data);
        draw_data->edit_string = edit_string;
        
        // animate
        draw_data->cursor_pos = ui_anim(ui_key_from_stringf(key, "anim_cursor_rapid"), cursor_pos, cursor_pos, ui_active_context->anim_rapid_rate);
        draw_data->cursor_pos_delay = ui_anim(ui_key_from_stringf(key, "anim_cursor_fast"), cursor_pos, cursor_pos, ui_active_context->anim_fast_rate);
        draw_data->mark_pos = ui_anim(ui_key_from_stringf(key, "anim_mark_pos"), mark_pos, mark_pos, ui_active_context->anim_rapid_rate);
        
    }
    
    ui_pop_parent();
    
    // interact
    ui_interaction interaction = ui_interaction_from_node(node);
    
    if (interaction & ui_left_dragging) {
        
        // calculate mouse point
        vec2_t text_pos = ui_text_align(edit_string_node->font, edit_string_node->font_size, edit_string, edit_string_node->rect, edit_string_node->text_alignment);
        f32 mouse_pos = ui_active_context->mouse_pos.x - text_pos.x;
        i32 char_index = ui_text_index_from_pos(edit_string_node->font, edit_string_node->font_size, edit_string, mouse_pos);
        ui_text_point_t mouse_point = {1, 1 + char_index};
        
        *cursor = mouse_point;
        if (interaction & ui_left_pressed) {
            *mark = mouse_point;
        }
        
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
ui_float_edit(f32* value, f32 delta, f32 min, f32 max, str_t label) {
    
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
ui_float_editf(f32* value, f32 delta, f32 min, f32 max, char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_interaction result = ui_float_edit(value, delta, min, max, string);
    
    return result;
}

function void
ui_vec3_edit(vec3_t* value, f32 delta, str_t label) {
    
    ui_set_next_layout_dir(ui_dir_right);
    ui_node_t* container = ui_node_from_key(0, { 0 });
    ui_push_parent(container);
    
    ui_size_t spacer_size = ui_size_pixels(5.0f);
    
    ui_push_size(ui_size_percent(1.0f), ui_size_percent(1.0f));
    ui_spacer(spacer_size);
    ui_set_next_tagf("red");
    ui_float_editf(&value->x, delta, 0.0f, 0.0f, "%s_x_edit", label.data);
    ui_spacer(spacer_size);
    ui_set_next_tagf("green");
    ui_float_editf(&value->y, delta, 0.0f, 0.0f, "%s_y_edit", label.data);
    ui_spacer(spacer_size);
    ui_set_next_tagf("blue");
    ui_float_editf(&value->z, delta, 0.0f, 0.0f, "%s_z_edit", label.data);
    ui_spacer(spacer_size);
    ui_pop_size();
    
    ui_pop_parent();
    
    
}

function void 
ui_vec3_editf(vec3_t* value, f32 delta, char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_vec3_edit(value, delta, string);
    
}





function ui_node_t*
ui_color_indicator(color_t* col, vec2_t pos, f32 size) {
    
    ui_node_flags flags =
        ui_flag_draw_custom;
    
    f32 half_size = size * 0.5f;
    ui_set_next_fixed_pos(pos.x - half_size, pos.y - half_size);
    ui_set_next_size(ui_size_pixels(size), ui_size_pixels(size));
    ui_node_t* node = ui_node_from_string(flags, str(""));
    ui_node_set_custom_draw(node, ui_color_indicator_draw, col);
    
    node->hover_t = node->tree_parent->hover_t;
    node->active_t = node->tree_parent->active_t;
    
    return node;
}

function ui_interaction
ui_color_sv_quad(color_t* col, str_t label) {
    // this widget assumes the color is
    // in HSVA color space. 
    
    // build node
    ui_node_flags flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_border | 
        ui_flag_draw_custom;
    
    ui_set_next_hover_cursor(os_cursor_hand_point);
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), label);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    
    // widget interaction
    ui_interaction interaction = ui_interaction_from_node(node);
    if (interaction & ui_left_dragging) {
        vec2_t mouse_pos = ui_active_context->mouse_pos;
        
        f32 node_width = rect_width(node->rect);
        f32 node_height = rect_height(node->rect);
        
        col->s = (mouse_pos.x - node->rect.x0) / node_width;
        col->v = 1.0f - (mouse_pos.y - node->rect.y0) / node_height;
        col->s = clamp_01(col->s);
        col->v = clamp_01(col->v);
    }
    
    // animate
    color_t* animated_col = (color_t*)arena_alloc(ui_build_arena(), sizeof(color_t));
    animated_col->h = col->h;
    animated_col->s = ui_anim(ui_key_from_stringf(node_key, "anim_sat"), col->s, col->s);
    animated_col->v = ui_anim(ui_key_from_stringf(node_key, "anim_val"), col->v, col->v);
    animated_col->a = 1.0f;
    
    // set custom draw and data
    ui_node_set_custom_draw(node, ui_color_sv_quad_draw, animated_col);
    
    // color indicator
    vec2_t indicator_pos = vec2((animated_col->s * rect_width(node->rect)), ((1.0f - animated_col->v) * rect_height(node->rect)));
    color_t* rgb_col = (color_t*)arena_alloc(ui_build_arena(), sizeof(color_t));
    *rgb_col = color_rgb_from_hsv(*animated_col);
    ui_set_next_parent(node);
    ui_color_indicator(rgb_col, indicator_pos, 6.0f);
    
    return interaction;
}

function ui_interaction
ui_color_hue_bar(color_t* col, str_t label) {
    // this widget assumes the color is
    // in HSVA color space. 
    
    // build node
    ui_node_flags flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_border | 
        ui_flag_draw_custom;
    
    ui_set_next_hover_cursor(os_cursor_hand_point);
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), label);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    
    // interaction
    ui_interaction interaction = ui_interaction_from_node(node);
    if (interaction & ui_left_dragging) {
        vec2_t mouse_pos = ui_active_context->mouse_pos;
        col->h = remap(mouse_pos.x, node->rect.x0, node->rect.x1, 0.0f, 1.0f);
        col->h = clamp_01(col->h);
    }
    
    // animate
    color_t* animated_col = (color_t*)arena_alloc(ui_build_arena(), sizeof(color_t));
    animated_col->h = ui_anim(ui_key_from_stringf(node_key, "anim_hue"), col->h, col->h);
    animated_col->s = col->s;
    animated_col->v = col->v;
    
    // set custom draw and data
    ui_node_set_custom_draw(node, ui_color_hue_bar_draw, nullptr);
    
    // color indicator
    vec2_t indicator_pos = vec2((animated_col->h * rect_width(node->rect)), (rect_height(node->rect) * 0.5f));
    color_t* hue_col = (color_t*)arena_alloc(ui_build_arena(), sizeof(color_t));
    *hue_col = color_rgb_from_hsv(color(animated_col->h, 1.0f, 1.0f, 1.0f));
    ui_set_next_parent(node);
    ui_color_indicator(hue_col, indicator_pos, 6.0f);
    
    return interaction;
}


function ui_interaction
ui_color_alpha_bar(color_t* col, str_t label) {
    // this widget assumes the color is
    // in HSVA color space. 
    
    // build node
    ui_node_flags flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_border | 
        ui_flag_draw_custom;
    
    ui_set_next_hover_cursor(os_cursor_hand_point);
    ui_key_t node_key = ui_key_from_string(ui_top_seed_key(), label);
    ui_node_t* node = ui_node_from_key(flags, node_key);
    
    // interaction
    ui_interaction interaction = ui_interaction_from_node(node);
    if (interaction & ui_left_dragging) {
        vec2_t mouse_pos = ui_active_context->mouse_pos;
        col->a = remap(mouse_pos.x, node->rect.x0, node->rect.x1, 0.0f, 1.0f);
        col->a = clamp_01(col->a);
    }
    
    // animate
    color_t* animated_col = (color_t*)arena_alloc(ui_build_arena(), sizeof(color_t));
    animated_col->h = col->h;
    animated_col->s = col->s;
    animated_col->v = col->v;
    animated_col->a = ui_anim(ui_key_from_stringf(node_key, "anim_alpha"), col->a, col->a);
    
    // set custom draw and data
    ui_node_set_custom_draw(node, ui_color_alpha_bar_draw, animated_col);
    
    // color indicator
    vec2_t indicator_pos = vec2((animated_col->a * rect_width(node->rect)), (rect_height(node->rect) * 0.5f));
    color_t* alpha_col = (color_t*)arena_alloc(ui_build_arena(), sizeof(color_t));
    *alpha_col = color_rgb_from_hsv(* animated_col);
    ui_set_next_parent(node);
    ui_color_indicator(alpha_col, indicator_pos, 6.0f);
    
    return interaction;
}

function ui_interaction 
ui_graph(void* data, u32 data_count, str_t label) {
    
    ui_node_flags node_flags =
        ui_flag_mouse_interactable |
        ui_flag_draw_border |
        ui_flag_draw_background |
        ui_flag_draw_custom |
        ui_flag_clip;
    
    ui_node_t* node = ui_node_from_string(node_flags, label);
    
    struct data_t {
        void* data;
        u32 data_count;
    };
    
    data_t* draw_data = (data_t*)arena_alloc(ui_build_arena(), sizeof(data_t));
    draw_data->data = data;
    draw_data->data_count= data_count;
    ui_node_set_custom_draw(node, ui_graph_draw, draw_data);
    
    ui_interaction interaction = ui_interaction_from_node(node);
    
    return interaction;
}


//- misc 

// tooltips
function void
ui_tooltip_begin() {
    ui_active_context->node_tooltip_root->flags |= (ui_flag_draw_background | ui_flag_draw_border | ui_flag_draw_shadow);
    ui_push_parent(ui_active_context->node_tooltip_root);
}

function void
ui_tooltip_end() {
    ui_pop_parent();
}

// popups
function b8
ui_popup_begin(ui_key_t key) {
    
    b8 is_open = ui_key_equals(key, ui_active_context->key_popup);
    
    if (is_open) {
        ui_active_context->node_popup_root->flags |= ui_flag_draw_background | ui_flag_draw_border | ui_flag_draw_shadow;
        ui_active_context->popup_updated_this_frame = true;
        ui_push_parent(ui_active_context->node_popup_root);
    }
    
    return is_open;
}

function void
ui_popup_end() {
    ui_pop_parent();
}

function void
ui_popup_open(ui_key_t key, vec2_t pos) {
    ui_active_context->key_popup = key;
    ui_active_context->popup_pos = pos;
    ui_active_context->popup_is_open = true;
    ui_active_context->popup_updated_this_frame = true;
}

function void
ui_popup_close() {
    ui_active_context->key_popup = { 0 };
    ui_active_context->popup_pos = vec2(0.0f, 0.0f);
    ui_active_context->popup_is_open = false;
}

function ui_dockspace_t*
ui_dockspace_create(ui_context_t* context) {
    
    arena_t* arena = arena_create(megabytes(64));
    
    ui_dockspace_t* dockspace = (ui_dockspace_t*)arena_calloc(arena, sizeof(ui_dockspace_t));
    
    dockspace->arena = arena;
    dockspace->context = context;
    
    // create 1 root panel
    dockspace->panel_root = ui_panel_create(dockspace, 1.0f, ui_axis_x);
    
    return dockspace;
}

function void
ui_dockspace_release(ui_dockspace_t* dockspace) {
    arena_release(dockspace->arena);
}

function void 
ui_dockspace_begin(ui_dockspace_t* dockspace, rect_t dockspace_rect) {
    
    // TODO:
    //
    // [ ] - better closest view button calculations
    // [ ] - clamp the dragged view button to edges of panel.
    // [ ] - support other panels and adjusting size between them.
    // [ ] - drag off of panel to other panels.
    //
    //
    
    // create non leaf panel ui
    
    
    // create leaf panel ui
    for (ui_panel_t* panel = dockspace->panel_root, *next = nullptr; panel != nullptr; panel = next) {
        ui_panel_rec_t rec = ui_panel_rec_depth_first(panel);
        next = rec.next;
        
        // skip if not leaf panel
        if (panel->tree_first != nullptr) { continue; }
        
        // calculate rect
        rect_t panel_rect = _ui_rect_from_panel(panel, dockspace_rect);
        
        rect_t tab_bar_rect = rect_cut_top(panel_rect, 30.0f);
        rect_t content_rect = panel_rect;
        content_rect.y0 += 30.0f;
        
        
        // build panel node
        ui_node_flags panel_flags = 
            ui_flag_draw_background |
            ui_flag_draw_border | 
            ui_flag_clip;
        
        ui_set_next_rect(content_rect);
        panel->node = ui_node_from_stringf(panel_flags, "%p_panel_node", panel);
        
        // build panel contents
        ui_push_parent(panel->node);
        
        
        // view function
        if (panel->view_focus != nullptr) {
            if (panel->view_focus->view_func != nullptr) {
                panel->view_focus->view_func(panel->view_focus);
            }
        }
        
        // empty panel
        if (panel->view_first == nullptr) {
            
            ui_spacer(ui_size_percent(1.0f));
            ui_set_next_size(ui_size_by_children(1.0f), ui_size_by_children(1.0f));
            ui_row_begin();
            ui_spacer(ui_size_percent(1.0f));
            {
                
                ui_set_next_tagf("close_panel_button");
                ui_set_next_size(ui_size_by_text(1.0f), ui_size_by_text(1.0f));
                ui_buttonf("Close Panel###%p", panel);
                
            }
            ui_spacer(ui_size_percent(1.0f));
            ui_row_end();
            ui_spacer(ui_size_percent(1.0f));
            
        }
        
        ui_pop_parent();
        
        // build tab bar
        ui_set_next_rect(tab_bar_rect);
        ui_set_next_layout_dir(ui_dir_right);
        ui_node_t* tab_bar_node = ui_node_from_stringf(0, "%p_tab_bar", panel);
        
        // build tab contents
        ui_push_parent(tab_bar_node);
        
        ui_spacer(ui_size_pixels(8.0f));
        
        ui_push_size(ui_size_pixels(120.0f, 0.5f), ui_size_percent(1.0f));
        ui_push_rounding(vec4(0.0f, 4.0f, 0.0f, 4.0f));
        
        b8 dragged_this_frame = false;
        for (ui_view_t* view = panel->view_first, *next = nullptr; view != nullptr; view = next) {
            next = view->next;
            
            ui_node_flags view_button_flags =
                ui_flag_mouse_interactable |
                ui_flag_draw_background |
                ui_flag_draw_border |
                ui_flag_draw_hover_effects |
                ui_flag_draw_active_effects |
                ui_flag_draw_custom |
                ui_flag_draw_text |
                ui_flag_anim_pos_x;
            
            if (view == dockspace->view_focus) {
                ui_set_next_tagf("focused");
            }
            
            ui_set_next_text_alignment(ui_text_align_left);
            view->node = ui_node_from_stringf(view_button_flags, "%p_%p", panel, view);
            ui_node_set_display_string(view->node, view->name);
            ui_node_set_custom_draw(view->node, ui_view_button_draw, nullptr);
            
            // view button interaction
            ui_interaction interaction = ui_interaction_from_node(view->node);
            vec2_t mouse_pos = ui_active_context->mouse_pos;
            
            // begin drag
            if ((interaction & ui_left_pressed) && !ui_drag_is_active()) {
                dragged_this_frame = true;
                ui_drag_begin(view->node->key);
                
                dockspace->view_focus = view;
                
                // store drag data
                ui_view_drag_data_t data = { 0 };
                data.current = view;
                data.prev = view->prev;
                data.offset = ui_active_context->drag_start_pos.x - view->node->rect.x0;
                ui_drag_store_data(&data, sizeof(ui_view_drag_data_t));
                
                // move item to top of list
                dll_remove(panel->view_first, panel->view_last, view);
                dll_push_front(panel->view_first, panel->view_last, view);
            }
            
            // update drag data
            if (ui_drag_is_active()) {
                ui_view_drag_data_t* data = (ui_view_drag_data_t*)ui_drag_get_data();
                
                // set dragged view to mouse pos
                if (view == data->current) {
                    view->node->flags |= (ui_flag_fixed_pos_x | ui_flag_ignore_parent_offset);
                    view->node->pos_fixed.x = mouse_pos.x - data->offset;
                }
                
                // build blank node
                if ((dragged_this_frame && data->current == view) || (data->prev == view)) {
                    ui_spacer(ui_size_pixels(3.0f));
                    ui_set_next_size_x(data->current->node->size_wanted[0]);
                    ui_node_t* blank_node = ui_node_from_key(0, { 0 });
                }
                
                // drop drag
                if (interaction & ui_left_released) {
                    dll_remove(panel->view_first, panel->view_last, data->current);
                    dll_insert(panel->view_first, panel->view_last, data->prev == data->current ? nullptr : data->prev, data->current);
                }
                
            }
            
            if (ui_drag_is_active()) {
                if (view != panel->view_first) {
                    ui_spacer(ui_size_pixels(3.0f));
                }
            } else {
                ui_spacer(ui_size_pixels(3.0f));
            }
        }
        
        // update drag data
        if (ui_drag_is_active()) {
            ui_view_drag_data_t* data = (ui_view_drag_data_t*)ui_drag_get_data();
            
            // get closest item in list
            ui_node_t* dragged_node = data->current->node;
            f32 dragged_center = (dragged_node->rect.x0 + dragged_node->rect.x1) * 0.5f;
            
            ui_view_t* closest_item = panel->view_first;
            for (ui_view_t* view = panel->view_first->next; view != nullptr; view = view->next) {
                ui_node_t* current_node = view->node;
                f32 current_center = (current_node->rect.x0 + current_node->rect.x1) * 0.5f;
                
                if (dragged_center > current_center) {
                    closest_item = view;
                }
            }
            
            data->prev = closest_item;
        }
        
        ui_pop_rounding();
        ui_pop_size();
        ui_pop_parent();
        
    }
}

function void 
ui_dockspace_end(ui_dockspace_t* dockspace) {
    
}

//- panel functions

function ui_panel_t*
ui_panel_create(ui_dockspace_t* dockspace, f32 percent, ui_axis axis) {
    
    ui_panel_t* panel = dockspace->panel_free;
    if (panel != nullptr) {
        stack_pop_n(dockspace->panel_free, global_next);
    } else {
        panel = (ui_panel_t*)arena_alloc(dockspace->arena, sizeof(ui_panel_t));
    }
    memset(panel, 0, sizeof(ui_panel_t));
    dll_push_back_np(dockspace->panel_first, dockspace->panel_last, panel, global_next, global_prev);
    
    panel->percent_of_parent = percent;
    panel->split_axis = axis;
    
    return panel;
}

function void
ui_panel_release(ui_dockspace_t* dockspace, ui_panel_t* panel) {
    dll_remove_np(dockspace->panel_first, dockspace->panel_last, panel, global_next, global_prev);
    stack_push_n(dockspace->panel_free, panel, global_next);
}

function void
ui_panel_insert(ui_panel_t* panel, ui_panel_t* parent, ui_panel_t* prev) {
    dll_insert_np(parent->tree_first, parent->tree_last, prev, panel, tree_next, tree_prev);
    panel->tree_parent = parent;
}

function void
ui_panel_remove(ui_panel_t* panel, ui_panel_t* parent) {
    dll_remove_np(parent->tree_first, parent->tree_last, panel, tree_next, tree_prev);
    panel->tree_next = nullptr;
    panel->tree_prev = nullptr;
    panel->tree_parent = nullptr;
}

function ui_panel_rec_t 
ui_panel_rec_depth_first(ui_panel_t* panel) {
    ui_panel_rec_t rec = { 0 };
    
    if (panel->tree_first != nullptr) {
        rec.next = panel->tree_first;
        rec.push_count = 1;
    } else for (ui_panel_t* p = panel; p != 0; p = p->tree_parent) {
        if (p->tree_next != nullptr) {
            rec.next = p->tree_next;
            break;
        }
        rec.pop_count++;
    }
    
    return rec;
}

function rect_t
_ui_rect_from_panel(ui_panel_t* panel, rect_t parent_rect) {
    
    temp_t scratch = scratch_begin();
    
    // count ancestors
    u32 ancestor_count = 0;
    for (ui_panel_t* p = panel->tree_parent; p != nullptr; p = p->tree_parent) {
        ancestor_count++;
    }
    
    // gather a list of ancestors
    ui_panel_t** ancestors = (ui_panel_t**)arena_alloc(scratch.arena, sizeof(ui_panel_t*) * ancestor_count);
    u32 i = 0;
    for (ui_panel_t* p = panel->tree_parent; p != nullptr; p = p->tree_parent) {
        ancestors[i] = p;
        i++;
    }
    
    // calculate rect from highest to lowest ancestor
    rect_t rect = parent_rect;
    for (i32 i = ancestor_count - 1; i >= 0; i--) {
        ui_panel_t* ancestor = ancestors[i];
        
        if (ancestor->tree_parent != nullptr) {
            rect = _ui_rect_from_panel_child(ancestor->tree_parent, ancestor, rect);
        }
    }
    
    // calculate final rect
    rect_t result = _ui_rect_from_panel_child(panel->tree_parent, panel, rect);
    
    scratch_end(scratch);
    
    return result;
}

function rect_t
_ui_rect_from_panel_child(ui_panel_t* parent, ui_panel_t* panel, rect_t parent_rect) {
    
    rect_t result = parent_rect;
    
    if (parent != nullptr) {
        
        vec2_t parent_rect_size = rect_size(parent_rect);
        ui_axis axis = parent->split_axis;
        
        result.v1[axis] = result.v0[axis];
        
        for (ui_panel_t* child = parent->tree_first; child != nullptr; child = child->tree_next) {
            result.v1[axis] += parent_rect_size[axis] * child->percent_of_parent;
            if (child == panel) {
                break;
            }
            result.v0[axis] = result.v1[axis];
        }
        
        result = rect_round(result);
    }
    
    return result;
    
}

//- view functions 

function ui_view_t* 
ui_view_create(ui_dockspace_t* dockspace, str_t name, ui_view_function* view_func) {
    
    ui_view_t* view = dockspace->view_free;
    if (view != nullptr) {
        stack_pop_n(dockspace->view_free, global_next);
    } else {
        view = (ui_view_t*)arena_alloc(dockspace->arena, sizeof(ui_view_t));
    }
    memset(view, 0, sizeof(ui_view_t));
    dll_push_back_np(dockspace->view_first, dockspace->view_last, view, global_next, global_prev);
    
    // fill struct
    view->name = name;
    view->view_func = view_func;
    
    return view;
}

function void 
ui_view_release(ui_dockspace_t* dockspace, ui_view_t* view) {
    dll_remove_np(dockspace->view_first, dockspace->view_last, view, global_next, global_prev);
    stack_push_n(dockspace->view_free, view, global_next);
}

function void 
ui_view_insert(ui_view_t* view, ui_panel_t* panel, ui_view_t* prev) {
    dll_insert(panel->view_first, panel->view_last, prev, view);
    view->panel = panel;
}

function void 
ui_view_remove(ui_view_t* view, ui_panel_t* panel) {
    dll_remove(panel->view_first, panel->view_last, view);
    view->panel = nullptr;
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
    ui_node_set_custom_draw(node, ui_canvas_draw, nullptr);
    
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
ui_slider_draw(ui_node_t* node) {
    
    // get data
    f32* data = (f32*)node->custom_draw_data;
    
    // calculate percent bar
    rect_t bar_rect = node->rect;
    bar_rect.x1 = lerp(bar_rect.x0, bar_rect.x1, *data);
    
    // draw bar
    color_t bar_color = color(0x5667e260);
    ui_r_set_next_color(bar_color);
    ui_r_draw_rect(bar_rect, 0.0f, 0.25f, node->rounding);
    
    // draw text
    vec2_t text_pos = ui_text_align(node->font, node->font_size, node->label, node->rect, node->text_alignment);
    color_t color_text = ui_color_from_key_name(node->tags_key, str("text"));
    color_t color_shadow = ui_color_from_key_name(node->tags_key, str("shadow"));
    
    // text shadow
    ui_r_set_next_color(color_shadow);
    ui_r_draw_text(node->label, vec2_add(text_pos, 1.0f), node->font, node->font_size);
    
    // text
    ui_r_set_next_color(color_text);
    ui_r_draw_text(node->label, text_pos, node->font, node->font_size);
    
}


function void
ui_text_edit_draw(ui_node_t* node) {
    
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
    
    color_t color_accent = ui_color_from_key_name(node->tags_key, str("accent"));
    color_t color_select = color(color_accent.r, color_accent.g, color_accent.b, 0.4f);
    
    // draw select
    ui_r_set_next_color(color_select);
    ui_r_draw_rect(select_rect, 0.0f, 0.25f, vec4(1.0f));
    
    // draw cursor
    ui_r_set_next_color(color_accent);
    ui_r_draw_rect(cursor_rect, 0.0f, 0.25f, vec4(1.0f));
    
}

function void
ui_color_indicator_draw(ui_node_t* node) {
    
    color_t col = *(color_t*)node->custom_draw_data;
    
    vec2_t indicator_pos = rect_center(node->rect);
    f32 indicator_size = rect_width(node->rect);
    indicator_size = lerp(indicator_size, indicator_size + 2.0f, node->hover_t);
    indicator_size = lerp(indicator_size, indicator_size + 2.0f, node->active_t);
    
    // borders
    ui_r_set_next_color(color(0x151515ff));
    ui_r_draw_circle(indicator_pos, indicator_size + 2.0f, 0.0f, 0.5f);
    
    ui_r_set_next_color(color(0xe2e2e2ff));
    ui_r_draw_circle(indicator_pos, indicator_size + 1.0f, 0.0f, 0.5f);
    
    // color
    ui_r_set_next_color(col);
    ui_r_draw_circle(indicator_pos, indicator_size, 0.0f, 0.5f);
    
}

function void
ui_color_sv_quad_draw(ui_node_t* node) {
    
    // get data
    color_t* hsv_color = (color_t*)node->custom_draw_data;
    
    // convert colors
    color_t hue_color = color_rgb_from_hsv(color(hsv_color->h, 1.0f, 1.0f, 1.0f));
    color_t rgb_color = color_rgb_from_hsv(color(hsv_color->h, hsv_color->s, hsv_color->v, 1.0f));
    
    // node info
    f32 node_width = rect_width(node->rect);
    f32 node_height = rect_height(node->rect);
    
    // draw hue quad
    ui_r_set_next_color0(color(0xffffffff));
    ui_r_set_next_color1(color(0x000000ff));
    ui_r_set_next_color2( hue_color);
    ui_r_set_next_color3(color(0x000000ff));
    ui_r_draw_rect(node->rect, 0.0f, 0.25f, node->rounding);
    
}

function void
ui_color_hue_bar_draw(ui_node_t* node) {
    
    // node info
    f32 node_width = rect_width(node->rect);
    f32 node_height = rect_height(node->rect);
    
    // draw hue bar
    
    // calculate hue colors in array
    const f32 step = 1.0f / 6.0f;
    const color_t segments[] = {
        color_rgb_from_hsv({0 * step, 1.0f, 1.0f, 1.0f}),
        color_rgb_from_hsv({1 * step, 1.0f, 1.0f, 1.0f}),
        color_rgb_from_hsv({2 * step, 1.0f, 1.0f, 1.0f}),
        color_rgb_from_hsv({3 * step, 1.0f, 1.0f, 1.0f}),
        color_rgb_from_hsv({4 * step, 1.0f, 1.0f, 1.0f}),
        color_rgb_from_hsv({5 * step, 1.0f, 1.0f, 1.0f}),
        color_rgb_from_hsv({6 * step, 1.0f, 1.0f, 1.0f}),
    };
    
    for (i32 i = 0; i < 6; i++) {
        
        // set colors
        ui_r_set_next_color0(segments[i + 0]);
        ui_r_set_next_color1(segments[i + 0]);
        ui_r_set_next_color2(segments[i + 1]);
        ui_r_set_next_color3(segments[i + 1]);
        
        // calculate rounding and rect
        f32 fix = (i == 0 || i == 5) ? 0.0f : 1.0f;
        f32 x0 = roundf(node->rect.x0 + (step * (i + 0) * node_width) - fix);
        f32 x1 = roundf(node->rect.x0 + (step * (i + 1) * node_width) + fix);
        f32 rounding0 = (i == 5) ? node->rounding.x : 0.0f;
        f32 rounding1 = (i == 5) ? node->rounding.y : 0.0f;
        f32 rounding2 = (i == 0) ? node->rounding.z : 0.0f;
        f32 rounding3 = (i == 0) ? node->rounding.w : 0.0f;
        vec4_t rounding = vec4(rounding0, rounding1, rounding2, rounding3);
        
        // draw
        ui_r_draw_rect(rect(x0, node->rect.y0, x1, node->rect.y1), 0.0f, 0.25f, rounding);
        
    }
    
}


function void
ui_color_alpha_bar_draw(ui_node_t* node) {
    
    // get data
    color_t* hsv_color = (color_t*)node->custom_draw_data;
    
    // convert colors
    color_t rgb_color = color_rgb_from_hsv(*hsv_color);
    color_t hue_color_full_alpha = color(rgb_color.r, rgb_color.g, rgb_color.b, 1.0f);
    color_t hue_color_no_alpha = color(rgb_color.r, rgb_color.g, rgb_color.b, 0.0f);
    
    // node info
    f32 node_width = rect_width(node->rect);
    f32 node_height = rect_height(node->rect);
    
    // draw alpha texture
    ui_r_instance_t* instance = ui_r_draw_rect(node->rect, 0.0f, 0.25f, node->rounding);
    instance->uv0 = vec2(0.0f, 0.0f);
    instance->uv1 = vec2(1.5f * (node_width / node_height), 1.5f);
    instance->texture_index = ui_r_get_texture_index(ui_transparent_texture);
    
    // set colors
    ui_r_set_next_color0(hue_color_no_alpha);
    ui_r_set_next_color1(hue_color_no_alpha);
    ui_r_set_next_color2(hue_color_full_alpha);
    ui_r_set_next_color3(hue_color_full_alpha);
    ui_r_draw_rect(node->rect, 0.0f, 0.25f, node->rounding);
    
}


function void 
ui_canvas_draw(ui_node_t* node) {
    
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

function void
ui_graph_draw(ui_node_t* node) {
    
    // get data
    struct data_t {
        void* data;
        u32 count;
    };
    
    data_t* draw_data = (data_t*)node->custom_draw_data;
    vec2_t* data = (vec2_t*)draw_data->data;
    
    f32 padding = 15.0f; 
    f32 graph_width = rect_width(node->rect) - (padding * 2);
    f32 graph_height = rect_height(node->rect) - (padding * 2);
    
    // find data max and min
    f32 min_width = f32_max;
    f32 max_width = f32_min;
    f32 min_height = f32_max;
    f32 max_height = f32_min;
    for (i32 i = 0; i < draw_data->count; i++) {
        if (data[i].x > max_width) { max_width = data[i].x; }
        if (data[i].y > max_height) { max_height = data[i].y; }
        if (data[i].x < min_width) { min_width = data[i].x; }
        if (data[i].y < min_height) { min_height = data[i].y; }
    }
    
    min_height = 1.0f / (144.0f + 60.0f);
    max_height = 1.0f / (144.0f - 60.0f);
    
    // draw points
    for (i32 i = 0; i < draw_data->count - 1; i++) {
        
        vec2_t point0 = data[i + 0];
        vec2_t point1 = data[i + 1];
        
        f32 new_x0 = node->rect.x0 + padding + remap(point0.x, min_width, max_width, 0.0f, graph_width);
        f32 new_y0 = node->rect.y1 - padding - remap(point0.y, min_height, max_height, 0.0f, graph_height);
        
        f32 new_x1 = node->rect.x0 + padding + remap(point1.x, min_width, max_width, 0.0f, graph_width);
        f32 new_y1 = node->rect.y1 - padding - remap(point1.y, min_height, max_height, 0.0f, graph_height);
        
        ui_r_set_next_color(color(0xb3e856ff));
        ui_r_draw_line(vec2(new_x0, new_y0), vec2(new_x1, new_y1), 1.5f, 0.25f);
        //ui_r_draw_circle(vec2(new_x, new_y), 4.0f, 0.0f, 0.5f);
    }
    
}


function void
ui_view_button_draw(ui_node_t* node) {
    
    //f32 node_height = rect_height(node->rect);
    //rect_t rect_top = rect_cut_top(node->rect, node_height * 0.65f);
    //rect_t rect_bottom = rect_cut_bottom(node->rect, node_height * 0.35f);
    
    //color_t accent_color = ui_color_from_key_name(node->tags_key, str("accent"));
    //color_t color_bottom_alpha = color(accent_color.r, accent_color.g, accent_color.b, 0.5f); 
    //color_t color_top_alpha = color(accent_color.r, accent_color.g, accent_color.b, 0.2f); 
    //color_t color_no_alpha = color(accent_color.r, accent_color.g, accent_color.b, 0.0f); 
    
    //ui_r_set_next_color0(color_top_alpha);
    //ui_r_set_next_color1(color_bottom_alpha);
    //ui_r_set_next_color2(color_top_alpha);
    //ui_r_set_next_color3(color_bottom_alpha);
    //ui_r_draw_rect(rect_bottom, 0.0f, 0.0f, vec4(0.0f));
    
    //ui_r_set_next_color0(color_no_alpha);
    //ui_r_set_next_color1(color_top_alpha);
    //ui_r_set_next_color2(color_no_alpha);
    //ui_r_set_next_color3(color_top_alpha);
    //ui_r_draw_rect(rect_top, 0.0f, 0.0f, vec4(0.0f, node->rounding.y, 0.0f, node->rounding.w));
    
}


//- theme editor demo 


function void 
ui_theme_editor() {
    
    persist vec2_t pos = vec2(250.0f, 250.0f);
    persist color_t* edit_color = nullptr;
    
    const f32 color_picker_size = 300.0f;
    
    // build color edit popup
    if (ui_popup_begin(ui_key_from_string({0}, str("color_edit")))) {
        
        color_t hsv_col = color_hsv_from_rgb(*edit_color);
        
        ui_set_next_size(ui_size_pixels(color_picker_size), ui_size_by_children(1.0f));
        ui_padding_begin(4.0f);
        
        ui_push_size(ui_size_percent(1.0f), ui_size_pixels(25.0f));
        {
            
            // sv quad
            ui_set_next_size_y(ui_size_pixels(color_picker_size));
            ui_color_sv_quad(edit_color, str("color_edit_sv_quad"));
            
            ui_spacer(ui_size_pixels(4.0f));
            
            // hue bar
            ui_color_hue_bar(edit_color, str("color_edit_hue_bar"));
            
            ui_spacer(ui_size_pixels(4.0f));
            
            // alpha bar
            ui_color_alpha_bar(edit_color, str("color_edit_alpha_bar"));
            
            ui_set_next_size_y(ui_size_by_children(1.0f));
            ui_row_begin();
            {
                
                ui_set_next_size(ui_size_percent(0.5f), ui_size_by_children(1.0f));
                ui_column_begin(); 
                {
                    ui_labelf("hue: %.3f", hsv_col.h);
                    ui_labelf("sat: %.3f", hsv_col.s);
                    ui_labelf("val: %.3f", hsv_col.v);
                }
                ui_column_end();
                
                ui_set_next_size(ui_size_percent(0.5f), ui_size_by_children(1.0f));
                ui_column_begin();
                {
                    ui_labelf("red: %.3f", edit_color->r);
                    ui_labelf("green: %.3f", edit_color->g);
                    ui_labelf("blue: %.3f", edit_color->b);
                }
                ui_column_end();
                
            }
            ui_row_end();
            
            ui_labelf("alpha: %.3f", hsv_col.a);
            
            u32 hex_col = color_hex_from_rgb(*edit_color);
            ui_labelf("hex: %x", hex_col);
            
        }
        ui_pop_size(); // percent: 1.0f, pixels: 25.0f
        
        ui_padding_end();
        
        ui_popup_end();
    }
    
    // draggable container
    ui_node_flags container_flags = 
        ui_flag_mouse_interactable | 
        ui_flag_fixed_pos |
        ui_flag_draw_background | 
        ui_flag_draw_border |
        ui_flag_draw_shadow;
    
    ui_set_next_padding(8.0f);
    ui_set_next_fixed_pos(pos.x, pos.y);
    ui_set_next_size(ui_size_pixels(300.0f), ui_size_pixels(400.0f));
    ui_node_t* container = ui_node_from_string(container_flags, str("theme_editor"));
    
    // build contents
    ui_push_parent(container);
    {
        // title
        ui_set_next_size(ui_size_percent(1.0f), ui_size_pixels(25.0f));
        ui_labelf("Theme Editor");
        ui_spacer(ui_size_pixels(8.0f));
        
        // scrollview
        ui_node_flags scrollview_flags = 
            ui_flag_mouse_interactable | 
            ui_flag_view_scroll_y | 
            ui_flag_view_clamp_y |
            ui_flag_draw_background | 
            ui_flag_draw_border |
            ui_flag_clip;
        
        ui_set_next_tagf("alt");
        ui_set_next_padding(8.0f);
        ui_set_next_size(ui_size_percent(1.0f), ui_size_percent(1.0f));
        ui_node_t* scrollview_node = ui_node_from_string(scrollview_flags, str("theme_edit_scrollview"));
        ui_push_parent(scrollview_node);
        {
            
            for (ui_theme_pattern_t* pattern = ui_active_context->theme->first; pattern != nullptr; pattern = pattern->next) {
                
                // build pattern container
                ui_set_next_layout_dir(ui_dir_right);
                ui_set_next_size(ui_size_percent(1.0f), ui_size_pixels(25.0f));
                ui_node_t* pattern_container = ui_node_from_stringf(0, "%p_container", pattern);
                
                
                ui_push_parent(pattern_container);
                {
                    
                    // build pattern label
                    ui_push_size(ui_size_by_text(0.0f), ui_size_percent(1.0f));
                    for (i32 i = 0; i < pattern->tag_count; i++) {
                        ui_label(pattern->tags[i]);
                    }
                    ui_pop_size();
                    
                    ui_spacer(ui_size_percent(1.0f));
                    
                    // build color button
                    ui_set_next_size(ui_size_pixels(25.0f), ui_size_percent(1.0f));
                    ui_node_t* color_button = ui_node_from_stringf(ui_flag_mouse_interactable | ui_flag_draw_border | ui_flag_draw_custom, "%p_col", pattern);
                    ui_node_set_custom_draw(color_button, ui_color_button_draw, &pattern->col);
                    ui_interaction color_button_interaction = ui_interaction_from_node(color_button);
                    // open popup
                    if (color_button_interaction & ui_left_clicked) {
                        edit_color = &pattern->col;
                        ui_popup_open(ui_key_from_string({ 0 }, str("color_edit")), ui_active_context->mouse_pos);
                    }
                }
                ui_pop_parent();
                
                ui_spacer(ui_size_pixels(8.0f));
            }
            
            ui_spacer(ui_size_pixels(8.0f));
            
        } // scrollview end
        ui_pop_parent();
        ui_interaction scrollview_interaction = ui_interaction_from_node(scrollview_node);
        
    } // draggable container end
    ui_pop_parent();
    
    // drag container
    ui_interaction container_interaction = ui_interaction_from_node(container);
    if (container_interaction & ui_left_dragging) {
        vec2_t mouse_delta = ui_active_context->mouse_delta;
        pos.x += mouse_delta.x;
        pos.y += mouse_delta.y;
    }
    
}

function void
ui_color_button_draw(ui_node_t* node) {
    
    // draw background
    color_t col = *(color_t*)node->custom_draw_data;
    
    color_t col_full_alpha = color(col.r, col.g, col.b, 1.0f);
    color_t col_alpha = color(col.r, col.g, col.b, col.a);
    
    // draw alpha texture
    ui_r_instance_t* instance = ui_r_draw_rect(node->rect, 0.0f, 0.25f, node->rounding);
    instance->uv0 = vec2(0.0f, 0.0f);
    instance->uv1 = vec2(1.5f, 1.5f);
    instance->texture_index = ui_r_get_texture_index(ui_transparent_texture);
    
    f32 node_height = rect_height(node->rect);
    f32 weight = 0.35f;
    rect_t top_rect = rect_cut_top(node->rect, (node_height * weight) + 1.0f);
    rect_t bot_rect = rect_cut_bottom(node->rect, node_height * (1.0f - weight));
    
    ui_r_set_next_color(col_full_alpha);
    ui_r_draw_rect(top_rect, 0.0f, 0.25f, vec4(0.0f, node->rounding.y, 0.0f, node->rounding.w));
    
    ui_r_set_next_color0(col_full_alpha);
    ui_r_set_next_color1(col_alpha);
    ui_r_set_next_color2(col_full_alpha);
    ui_r_set_next_color3(col_alpha);
    ui_r_draw_rect(bot_rect, 0.0f, 0.25f, vec4(node->rounding.x, 0.0f, node->rounding.z, 0.0f));
    
}


function void 
ui_debug() {
    
    if (!ui_key_equals(ui_active_context->key_hovered, { 0 })) {
        
        ui_node_t* debug_node = ui_node_find(ui_active_context->key_hovered);
        
        ui_tooltip_begin();
        
        ui_push_size(ui_size_pixels(150.0f), ui_size_pixels(25.0f));
        
        ui_labelf("key: %x", debug_node->key);
        
        ui_pop_size();
        
        ui_tooltip_end();
        
    }
    
}


#endif // SORA_UI_WIDGETS_CPP