// profile.cpp

#ifndef PROFILE_CPP
#define PROFILE_CPP

//- implementation 

function void
profile_init() {
    profiler.arena = arena_create(megabytes(512));
    
    profiler.slots = (profile_slot_t*)arena_alloc(profiler.arena, sizeof(profile_slot_t) * 1024 * 1024);
    
    profiler.cpu_freq = os_get_cpu_freq();
}

function void
profile_release() {
    arena_release(profiler.arena);
}

function void
profile_begin(cstr name) {
    
    // get index
    u32 index = profiler.slot_count;
    
    profile_slot_t* slot = &profiler.slots[index];
    profiler.slot_count++;
    
    // push this index to the stack
    profiler.slot_indices[++profiler.top_slot_index] = index;
    
    slot->name = name;
    slot->depth = profiler.current_depth;
    slot->start =  os_get_cpu_time();
    
    profiler.current_depth++;
    
}

function void
profile_end() {
    
    u64 end_time = os_get_cpu_time();
    
    // pop index from stack
    u32 index = profiler.slot_indices[profiler.top_slot_index--];
    
    // get slot
    profile_slot_t* slot = &profiler.slots[index];
    profiler.current_depth--;
    slot->end = end_time;
    
}

//- ui functions 

function void 
ui_profile_graph(str_t label) {
    
    // calculate total time
    // TODO: don't really need to do this everytime, just once
    f64 total_ms = 0.0f;
    for (i32 i = 0; i < profiler.slot_count; i++) {
        profile_slot_t* slot = &profiler.slots[i];
        if (slot->depth == 0) {
            total_ms += (f64)(slot->end - slot->start) * 1000.0 / (f64)profiler.cpu_freq;
        }
    }
    
    // start and end time
    f64 entry_start_ms = ((f64)profiler.slots[0].start * 1000.0) / (f64)profiler.cpu_freq;
    persist f64 start_ms = entry_start_ms;
    persist f64 end_ms = start_ms + total_ms;
    f64 anim_start_ms = ui_anim(ui_key_from_stringf({0}, "profile_start_ms"), start_ms, start_ms);
    f64 anim_end_ms = ui_anim(ui_key_from_stringf({0}, "profile_end_ms"), end_ms, end_ms);
    f64 anim_range_ms = (anim_end_ms - anim_start_ms);
    
    profile_graph_draw_data_t* data = (profile_graph_draw_data_t*)arena_alloc(ui_build_arena(), sizeof(profile_graph_draw_data_t));
    data->start_ms = entry_start_ms;
    data->current_start_ms = anim_start_ms;
    data->current_end_ms = anim_end_ms;
    
    
    ui_node_flags graph_flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_custom;
    
    ui_set_next_padding(4.0f);
    ui_node_t* graph_node = ui_node_from_string(graph_flags, label);
    ui_node_set_custom_draw(graph_node, ui_profile_graph_draw, data);
    
    ui_push_parent(graph_node);
    
    ui_spacer(ui_size_pixels(30.0f));
    
    ui_set_next_size(ui_size_percent(1.0f), ui_size_percent(1.0f));
    ui_node_t* container_node = ui_node_from_stringf(ui_flag_clip, "%p_container", graph_node);
    ui_push_parent(container_node);
    
    f32 graph_width = rect_width(container_node->rect);
    f32 graph_height = rect_height(container_node->rect);
    
    for (i32 i = 0; i < profiler.slot_count; i++) {
        profile_slot_t* slot = &profiler.slots[i];
        
        // calculate rect
        f64 min_ms =  (f64)(slot->start) * 1000.0f / (f64)(profiler.cpu_freq);
        f64 max_ms =  (f64)(slot->end) * 1000.0f / (f64)(profiler.cpu_freq);
        f32 min_pos_x = (min_ms - anim_start_ms) / anim_range_ms * graph_width; 
        f32 max_pos_x = (max_ms - anim_start_ms) / anim_range_ms * graph_width; 
        
        min_pos_x = clamp(min_pos_x, 0.0f, graph_width);
        max_pos_x = clamp(max_pos_x, 0.0f, graph_width);
        
        if (min_pos_x != max_pos_x) {
            
            rect_t entry_rect = rect(min_pos_x, 25.0f * slot->depth, max_pos_x, 25.0f * (slot->depth + 1));
            
            f32 slot_ms = max_ms - min_ms;
            
            // build ui node
            ui_set_next_rect(entry_rect);
            ui_set_next_text_alignment(ui_text_align_left);
            ui_node_t* slot_node = ui_node_from_stringf(ui_flag_mouse_interactable | ui_flag_draw_border | ui_flag_draw_custom, "%s_%p", label.data, slot);
            ui_node_set_display_string(slot_node, str_format(ui_build_arena(), "%s : %.2f ms", slot->name, slot_ms));
            ui_node_set_custom_draw(slot_node, ui_profile_slot_draw, nullptr);
            
            ui_interaction interaction = ui_interaction_from_node(slot_node);
            
            if (interaction & ui_hovered) {
                ui_tooltip_begin();
                
                ui_set_next_size(ui_size_by_text(2.0f), ui_size_by_text(2.0f));
                ui_labelf("%s : %.2f ms", slot->name, slot_ms);
                
                ui_tooltip_end();
            }
        }
    }
    
    ui_pop_parent();
    ui_pop_parent();
    
    // interaction
    ui_interaction interaction = ui_interaction_from_node(graph_node);
    
    f32 factor = 0.001f * anim_range_ms;
    
    if (interaction & ui_left_dragging) {
        vec2_t mouse_delta = ui_active_context->mouse_delta;
        start_ms -= mouse_delta.x * factor;
        end_ms -= mouse_delta.x * factor;
    }
    
    if (interaction & ui_mouse_scrolled) {
        
        vec2_t mouse_pos = ui_active_context->mouse_pos;
        f32 scroll = ui_mouse_scroll();
        
        f32 zoom_factor = scroll > 0.0f ? factor * 60.0f : -factor * 60.0f;
        f32 mouse_ratio = (mouse_pos.x - graph_node->rect.x0) / graph_width;
        
        start_ms += zoom_factor * mouse_ratio;
        end_ms -= zoom_factor * (1.0f - mouse_ratio);
    }
    
    
}

function void
ui_profile_graph_draw(ui_node_t* node) {
    
    profile_graph_draw_data_t* data = (profile_graph_draw_data_t*)node->custom_draw_data;
    
    ui_r_set_next_color(ui_color_from_key_name(node->tags_key, str("background alt")));
    ui_r_draw_rect(node->rect, 0.0f, 0.25f, node->rounding);
    
    f64 start_ms = data->current_start_ms - data->start_ms;
    f64 end_ms = data->current_end_ms - data->start_ms;
    f64 range_ms =  (end_ms - start_ms);
    
    f64 graph_width = rect_width(node->rect) - (node->padding.x * 2.0f);
    f64 pixels_per_ms = graph_width / range_ms;
    f64 ideal_interval = 75.0f / pixels_per_ms;
    
    f64 magnitude = pow(10, floor(log10(ideal_interval)));
    f64 steps[] = {1, 2, 5, 10};
    f64 interval_ms = steps[3] * magnitude;
    for (i32 i = 0; i < 4; i++) {
        if (steps[i] * magnitude >= ideal_interval) {
            interval_ms = steps[i] * magnitude;
            break;
        }
    }
    
    f64 first_ms = ceil(start_ms / interval_ms) * interval_ms;
    f32 start_pos =  node->rect.x0 + (node->padding.x);
    
    ui_r_push_clip_mask(node->rect);
    
    for (f64 t = first_ms - interval_ms; t <= end_ms; t += interval_ms) {
        
        f32 x = start_pos + (t - start_ms) * pixels_per_ms;
        vec2_t p0 = vec2(x, node->rect.y0);
        vec2_t p1 = vec2(x, node->rect.y1);
        ui_r_set_next_color(color(0x26272aff));
        ui_r_draw_line(p0, p1, 1.5f, 0.0f);
        
        
        f64 delta_time = t;
        str_t time_text;
        if (interval_ms >= 1000.0) {
            time_text = str_format(ui_build_arena(), "%.1f s", delta_time / 1000.0);
        } else if (interval_ms >= 1.0) {
            time_text = str_format(ui_build_arena(), "%.1f ms", delta_time);
        } else if (interval_ms >= 0.001) {
            time_text = str_format(ui_build_arena(), "%.1f us", delta_time * 1000.0);
        } else {
            time_text = str_format(ui_build_arena(), "%.1f ns", delta_time * 1e6);
        }
        
        ui_r_set_next_color(color(0x676869ff));
        ui_r_draw_text(time_text, vec2(x + 2.0f, node->rect.y0 + 8.0f), node->font, node->font_size);
    }
    
    ui_r_pop_clip_mask();
    
}

function void
ui_profile_slot_draw(ui_node_t* node) {
    
    f32 hue = (f32)((u32)node % 360) / 360.0f;
    
    color_t col = color_rgb_from_hsv(color(hue, 0.4f, 0.5f, 1.0f));
    
    // draw background
    ui_r_set_next_color(col );
    ui_r_draw_rect(node->rect, 0.0f, 0.33f, node->rounding);
    
    // draw text
    ui_r_push_clip_mask(node->rect);
    
    color_t text_color = ui_color_from_key_name(node->tags_key, str("text"));
    color_t shadow_color = ui_color_from_key_name(node->tags_key, str("text shadow"));
    vec2_t text_pos = ui_text_align(node->font, node->font_size, node->label, node->rect, node->text_alignment);
    
    ui_r_set_next_color(shadow_color);
    ui_r_draw_text(node->label, vec2_add(text_pos, 1.0f), node->font, node->font_size);
    
    ui_r_set_next_color(text_color);
    ui_r_draw_text(node->label, text_pos, node->font, node->font_size);
    
    ui_r_pop_clip_mask();
}


#endif // PROFILE_CPP