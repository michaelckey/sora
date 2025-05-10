// sora_ui_core.cpp

#ifndef SORA_UI_CORE_CPP
#define SORA_UI_CORE_CPP

//~ implementation

//- state 

function void
ui_init() {
    
    // load icon font
    ui_font_icon = font_open(str("res/fonts/icons.ttf"));
    ui_font_default = font_open(str("res/fonts/consola.ttf"));
    
}

function void
ui_release() {
    font_close(ui_font_icon);
    font_close(ui_font_default);
}

function void 
ui_begin(ui_context_t* context) {
    prof_begin("ui_begin");
    
    ui_context_set_active(context);
    
    // reset stacks
#define ui_stack(name, type)\
context->name##_stack.top = &context->name##_default_node;\
context->name##_stack.free = nullptr;\
context->name##_stack.auto_pop = false;
    ui_stack_list
#undef ui_stack
    
#define ui_r_stack(name, type)\
context->name##_r_stack.top = &context->name##_r_default_node;\
context->name##_r_stack.free = nullptr;\
context->name##_r_stack.auto_pop = false;
    ui_r_stack_list
#undef ui_r_stack
    
    // reset texture list
    memset(context->texture_list, 0, sizeof(gfx_handle_t) * ui_r_max_textures);
    context->texture_count = 0;
    
    memset(context->constants.clip_masks, 0, sizeof(rect_t) * 128);
    context->clip_mask_count = 0;
    
    // reset build state
    context->tags_hash_list_count = 512;
    context->tags_hash_list = (ui_tags_hash_list_t*)arena_calloc(ui_build_arena(), sizeof(ui_tags_hash_list_t) * context->tags_hash_list_count);
    
    // copy to prev key state
    context->key_hovered_prev = context->key_hovered;
    for (i32 i = 0; i < os_mouse_button_count; i++) {
        context->key_active_prev[i] = context->key_active[i];
    }
    
    // reset keys
    context->key_hovered = { 0 };
    for (i32 button = 0; button < os_mouse_button_count; button++) {
        ui_node_t* node = ui_node_find(context->key_active[button]);
        if (node == nullptr) {
            context->key_active[button] = { 0 };
        }
    }
    
    // gather events
    
    vec2_t last_mouse_pos = context->mouse_pos;
    context->mouse_pos = os_window_get_cursor_pos(context->window);
    context->mouse_delta = vec2_sub(context->mouse_pos, last_mouse_pos);
    
    for (os_event_t* os_event = os_state.event_list.first; os_event != nullptr; os_event = os_event->next) {
        if (os_handle_equals(os_event->window, context->window)) {
            
            // allocate event
            ui_event_t* ui_event = (ui_event_t*)arena_calloc(context->event_arena, sizeof(ui_event_t));
            
            // fill event data
            ui_event->os_event = os_event;
            switch (os_event->type) {
                
                case os_event_type_key_press: {
                    ui_event->type = ui_event_type_key_press;
                    
                    ui_key_binding_t* binding = ui_key_binding_find(os_event->key, os_event->modifiers);
                    if (binding != nullptr) {
                        ui_event->type = binding->result_type;
                        ui_event->flags = binding->result_flags;
                        ui_event->delta_unit = binding->result_delta_unit;
                        ui_event->delta = binding->result_delta;
                    }
                    
                    break;
                }
                
                case os_event_type_key_release: {
                    ui_event->type = ui_event_type_key_release;
                    break;
                }
                
                case os_event_type_mouse_press: {
                    ui_event->type = ui_event_type_mouse_press;
                    context->click_counter[os_event->mouse]++;
                    context->last_click_time[os_event->mouse] = (os_time_microseconds() / 1000);
                    break;
                }
                
                case os_event_type_mouse_release: {
                    ui_event->type = ui_event_type_mouse_release;
                    break;
                }
                
                case os_event_type_mouse_move: {
                    ui_event->type = ui_event_type_mouse_move;
                    break;
                }
                
                case os_event_type_mouse_scroll: {
                    ui_event->type = ui_event_type_mouse_scroll;
                    break;
                }
                
                case os_event_type_text: {
                    ui_event->type = ui_event_type_text;
                    break;
                }
            }
            
            // push to event list
            ui_event_push(ui_event);
        }
        
    }
    
    // drag state
    for (ui_event_t* event = context->event_list.first; event != nullptr; event = event->next) {
        if (event->type == ui_event_type_mouse_release && ui_drag_is_active()) {
            context->drag_state = ui_drag_state_dropping;
        }
    }
    
    // prune unused animation nodes
    for (ui_anim_node_t* node = context->anim_node_lru, *next = nullptr; node != nullptr; node = next) {
        next = node->lru_next;
        
        if (node->last_build_index + 2 < context->build_index) {
            u32 index = node->key.data[0] % context->anim_hash_list_count;
            dll_remove_np(context->anim_hash_list[index].first, context->anim_hash_list[index].last, node, list_next, list_prev);
            dll_remove_np(context->anim_node_lru, context->anim_node_mru, node, lru_next, lru_prev);
            stack_push_n(context->anim_node_free, node, list_next);
        }
    }
    
    // build root nodes
    uvec2_t renderer_size = gfx_renderer_get_size(context->renderer);
    ui_set_next_fixed_size((f32)renderer_size.x, (f32)renderer_size.y);
    context->node_root = ui_node_from_stringf(0, "root_%u", context->window.data[0]);
    ui_push_parent(context->node_root);
    
    // push rendering stacks
    ui_r_push_clip_mask(rect(0.0f, 0.0f, (f32)renderer_size.x, (f32)renderer_size.y));
    ui_r_push_texture(context->texture);
    
    prof_end();
}

function void 
ui_end(ui_context_t* context) {
    prof_begin("ui_end");
    
    // pop root parent
    ui_pop_parent();
    
    // drag state
    if (context->drag_state == ui_drag_state_dropping) {
        context->drag_state = ui_drag_state_null;
        context->key_drag = { 0 };
    }
    
    // unused events
    for (ui_event_t* event = context->event_list.first; event != nullptr; event = event->next) {
        
        // reset focused key
        if (event->type == ui_event_type_mouse_release) {
            context->key_focused = { 0 };
        }
    }
    
    // prune unused and transient nodes
    for (u32 index = 0; index < context->node_hash_list_count; index++) {
        for (ui_node_t* node = context->node_hash_list[index].first, *next = nullptr; node != nullptr; node = next) {
            next = node->list_next;
            if (node->last_build_index < context->build_index || ui_key_equals(node->key, { 0 })) {
                dll_remove_np(context->node_hash_list[index].first, context->node_hash_list[index].last, node, list_next, list_prev);
                stack_push_n(context->node_free, node, list_next);
            }
        }
    }
    
    // layout
    {
        prof_begin("layout");
        for (ui_axis axis = 0; axis < ui_axis_count; axis++) {
            ui_layout_solve_independent(context->node_root, axis);
            ui_layout_solve_upward_dependent(context->node_root, axis);
            ui_layout_solve_downward_dependent(context->node_root, axis);
            ui_layout_solve_violations(context->node_root, axis);
            ui_layout_set_positions(context->node_root, axis);
        }
        prof_end();
    }
    
    // animate
    {
        prof_begin("animate");
        
        // animation rates
        f32 dt = os_window_get_delta_time(context->window);
        context->anim_rapid_rate = 1.0f - powf(2.0f, -75.0f * dt);
        context->anim_fast_rate = 1.0f - powf(2.0f, -50.0f * dt);
        context->anim_slow_rate = 1.0f - powf(2.0f, -25.0f * dt);
        
        // animate cache
        for (u32 index =  0; index < context->anim_hash_list_count; index++) {
            for (ui_anim_node_t* node = context->anim_hash_list[index].first; node != nullptr; node = node->list_next) {
                node->current += (node->params.target - node->current) * node->params.rate;
            }
        }
        
        // animate nodes
        for (u32 index = 0; index < context->node_hash_list_count; index++) {
            for (ui_node_t* node = context->node_hash_list[index].first, *next = nullptr; node != nullptr; node = next) {
                
                b8 is_hovered = ui_key_equals(context->key_hovered, node->key);
                b8 is_active = ui_key_equals(context->key_active[os_mouse_button_left], node->key);
                
                node->hover_t += context->anim_slow_rate * ((f32)is_hovered - node->hover_t);
                node->active_t += context->anim_slow_rate * ((f32)is_active - node->active_t);
                
                // animate pos
                if (node->flags & ui_flag_anim_pos_x) {
                    node->pos.x += context->anim_fast_rate * (node->pos_target.x - node->pos.x);
                    if (fabsf(node->pos_target.x - node->pos.x) < 1.0f) { node->pos.x = node->pos_target.x; }
                }
                if (node->flags & ui_flag_anim_pos_y) {
                    node->pos.y += context->anim_fast_rate * (node->pos_target.y - node->pos.y);
                    if (fabsf(node->pos_target.y - node->pos.y) < 1.0f) { node->pos.y = node->pos_target.y; }
                }
                
                // animate view offset
                node->view_offset_prev = node->view_offset;
                node->view_offset.x += context->anim_fast_rate * (node->view_offset_target.x - node->view_offset.x);
                if (fabsf(node->view_offset_target.x - node->view_offset.x) < 1.0f) { node->view_offset.x = node->view_offset_target.x; }
                
                node->view_offset.y += context->anim_fast_rate * (node->view_offset_target.y - node->view_offset.y);
                if (fabsf(node->view_offset_target.y - node->view_offset.y) < 1.0f) { node->view_offset.y = node->view_offset_target.y; }
                
            }
        }
        
        prof_end();
    }
    
    
    // hover cursor
    ui_node_t* hovered_node = ui_node_find(context->key_hovered);
    ui_node_t* active_node = ui_node_find(context->key_active[os_mouse_button_left]);
    ui_node_t* node = active_node == nullptr ? hovered_node : active_node; 
    
    if (node != nullptr) {
        os_cursor cursor = node->hover_cursor;
        if (cursor != os_cursor_null) {
            os_set_cursor(cursor);
        }
        
    }
    
    
    // draw ui 
    
    for (ui_node_t* node = context->node_root; node != nullptr;) {
        ui_node_rec_t rec = ui_node_rec_depth_first(node);
        
        // clipping
        if (node->flags & ui_flag_clip) {
            rect_t top_clip = ui_r_top_clip_mask();
            rect_t new_clip = node->rect;
            if (top_clip.x1 != 0.0f || top_clip.y1 != 0.0f) {
                new_clip = rect_intersection(new_clip, top_clip);
            }
            rect_validate(new_clip);
            ui_r_push_clip_mask(new_clip);
        }
        
        // shadow
        if (node->flags & ui_flag_draw_shadow) {
            ui_r_set_next_color(color(0x00000090));
            ui_r_set_next_softness(6.0f);
            ui_r_draw_rect(rect_translate(rect_grow(node->rect, 4.0f), 4.0f));
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
            ui_r_set_next_color(color_background);
            ui_r_set_next_rounding(node->rounding);
            ui_r_draw_rect(node->rect);
            
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
                
                ui_r_set_next_color0(color_transparent);
                ui_r_set_next_color1(color_effect);
                ui_r_set_next_color2(color_transparent);
                ui_r_set_next_color3(color_effect);
                ui_r_set_next_rounding(vec4(node->rounding.x, 0.0f, node->rounding.z, 0.0f));
                ui_r_draw_rect(top_rect);
                
                ui_r_set_next_color0(color_effect);
                ui_r_set_next_color1(color_transparent);
                ui_r_set_next_color2(color_effect);
                ui_r_set_next_color3(color_transparent);
                ui_r_set_next_rounding(vec4(0.0f, node->rounding.y, 0.0f, node->rounding.w));
                ui_r_draw_rect(bottom_rect);
            }
            
        }
        
        // border
        if (node->flags & ui_flag_draw_border) {
            
            color_t border_main_color = ui_color_from_key(ui_key_from_string(node->tags_key, str("border")));
            color_t border_highlight_color = color_blend(border_main_color, color(0xffffff35));
            color_t border_lowlight_color = color_blend(border_main_color, color(0x00000035));
            
            ui_r_set_next_color0(border_main_color);
            ui_r_set_next_color1(border_main_color);
            ui_r_set_next_color2(border_highlight_color);
            ui_r_set_next_color3(border_lowlight_color);
            ui_r_set_next_rounding(node->rounding);
            ui_r_set_next_thickness(1.0f);
            ui_r_draw_rect(rect_shrink(node->rect, 1.0f));
            
            color_t border_dark_color = ui_color_from_key(ui_key_from_string(node->tags_key, str("background")));
            border_dark_color = color(0x131313ff);
            
            ui_r_set_next_color(border_dark_color);
            ui_r_set_next_rounding(node->rounding);
            ui_r_set_next_thickness(1.0f);
            ui_r_draw_rect(node->rect);
        }
        
        // text
        if (node->flags & ui_flag_draw_text) {
            color_t shadow_color = ui_color_from_key(ui_key_from_string(node->tags_key, str("shadow")));
            color_t text_color = ui_color_from_key(ui_key_from_string(node->tags_key, str("text")));
            vec2_t text_pos = ui_text_align(node->font, node->font_size, node->label, node->rect, node->text_alignment);
            
            ui_r_push_font(node->font);
            ui_r_push_font_size(node->font_size);
            
            ui_r_set_next_color(shadow_color);
            ui_r_draw_text(node->label, vec2_add(text_pos, 1.0f));
            
            ui_r_set_next_color(text_color);
            ui_r_draw_text(node->label, text_pos);
            
            ui_r_pop_font();
            ui_r_pop_font_size();
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
            if (n->flags & ui_flag_clip) { ui_r_pop_clip_mask(); }
        }
        
        node = rec.next;
    }
    
    
    // render ui
    
    // update constant buffer
    uvec2_t renderer_size = gfx_renderer_get_size(context->renderer);
    rect_t viewport = rect(0.0f, 0.0f, (f32)renderer_size.x, (f32)renderer_size.y);
    context->constants.window_size = vec2((f32)renderer_size.x, (f32)renderer_size.y);
    
    gfx_buffer_fill(context->constant_buffer, &context->constants, sizeof(ui_r_constants_t));
    
    // set state
    gfx_set_viewport(viewport);
    gfx_set_scissor(viewport);
    gfx_set_rasterizer(gfx_fill_solid, gfx_cull_back);
    gfx_set_topology(gfx_topology_tri_strip);
    gfx_set_sampler(gfx_filter_linear, gfx_wrap_repeat, 0);
    gfx_set_depth_mode(gfx_depth_none);
    gfx_set_shader(context->vertex_shader);
    gfx_set_shader(context->pixel_shader);
    gfx_set_buffer(context->constant_buffer);
    gfx_set_texture_array(context->texture_list, context->texture_count, 0);
    
    for (ui_r_batch_t* batch = context->batch_first; batch != 0; batch = batch->next) {
        
        // fill instance buffer
        gfx_buffer_fill(context->instance_buffer, batch->instances, batch->instance_count * sizeof(ui_r_instance_t));
        gfx_set_buffer(context->instance_buffer, 0, sizeof(ui_r_instance_t));
        
        gfx_draw_instanced(4, batch->instance_count);
    }
    
    
    // clear batches
    arena_clear(context->batch_arena);
    context->batch_first = nullptr;
    context->batch_last = nullptr;
    
    context->build_index++;
    arena_clear(ui_build_arena());
    arena_clear(context->event_arena);
    context->event_list.first = nullptr;
    context->event_list.last = nullptr;
    ui_context_set_active(nullptr);
    
    prof_end();
}

function arena_t*
ui_build_arena() {
    arena_t* arena = ui_active_context->build_arenas[ui_active_context->build_index % 2];
    return arena;
}

//- context functions 

function ui_context_t* 
ui_context_create(os_handle_t window, gfx_handle_t renderer) {
    
    arena_t* arena = arena_create(megabytes(256));
    ui_context_t* context = (ui_context_t*)arena_alloc(arena, sizeof(ui_context_t));
    memset(context, 0, sizeof(ui_context_t));
    
    context->arena = arena;
    context->build_arenas[0] = arena_create(megabytes(64));
    context->build_arenas[1] = arena_create(megabytes(64));
    context->drag_state_arena = arena_create(megabytes(64));
    context->event_arena = arena_create(kilobytes(8));
    
    context->window = window;
    context->renderer = renderer;
    
    context->node_hash_list_count = 4096;
    context->node_hash_list = (ui_node_hash_list_t*)arena_alloc(arena, sizeof(ui_node_hash_list_t) * context->node_hash_list_count);
    
    context->anim_hash_list_count = 4096;
    context->anim_hash_list = (ui_anim_hash_list_t*)arena_alloc(arena, sizeof(ui_anim_hash_list_t) * context->anim_hash_list_count);
    
    context->theme_pattern_hash_list_count = 512;
    context->theme_pattern_hash_list = (ui_theme_pattern_hash_list_t*)arena_alloc(arena, sizeof(ui_theme_pattern_hash_list_t) * context->theme_pattern_hash_list_count);
    
    // rendering
    context->batch_arena = arena_create(megabytes(64));
    
    context->vertex_shader = gfx_shader_load(str("res/shaders/shader_ui.hlsl"), gfx_shader_flag_vertex | gfx_shader_flag_per_instance);
    context->pixel_shader = gfx_shader_load(str("res/shaders/shader_ui.hlsl"), gfx_shader_flag_pixel);
    
    context->instance_buffer = gfx_buffer_create(gfx_buffer_type_vertex, kilobytes(256));
    context->constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, kilobytes(4));
    
    u32 data = 0xffffffff;
    context->texture = gfx_texture_create(uvec2(1, 1), gfx_texture_format_rgba8, &data);
    
    // theme
    ui_context_add_color(context, str("background"), color(0x252629ff));
    ui_context_add_color(context, str("shadow"), color(0x00000080));
    ui_context_add_color(context, str("text"), color(0xe2e2e3ff));
    ui_context_add_color(context, str("border"), color(0xffffff35));
    ui_context_add_color(context, str("effect"), color(0x393a3fff));
    
    // default stack values
#define ui_stack(name, type, initial)\
context->name##_default_node.v = initial;
    ui_stack_list
#undef ui_stack
    
#define ui_r_stack(name, type, initial)\
context->name##_r_default_node.v = initial;
    ui_r_stack_list
#undef ui_r_stack
    
    // key bindings
    ui_context_set_active(context);
    
    ui_key_binding_add(os_key_left,  0, ui_event_type_navigate, 0, ui_event_delta_unit_char, { -1, 0 } );
    ui_key_binding_add(os_key_right, 0, ui_event_type_navigate, 0, ui_event_delta_unit_char, { +1, 0 } );
    ui_key_binding_add(os_key_up,    0, ui_event_type_navigate, 0, ui_event_delta_unit_char, { 0, -1 } );
    ui_key_binding_add(os_key_down,  0, ui_event_type_navigate, 0, ui_event_delta_unit_char, { 0, +1 } );
    
    ui_key_binding_add(os_key_left,  os_modifier_shift, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_char, { -1, 0 } );
    ui_key_binding_add(os_key_right, os_modifier_shift, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_char, { +1, 0 } );
    ui_key_binding_add(os_key_up,    os_modifier_shift, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_char, { 0, -1 } );
    ui_key_binding_add(os_key_down,  os_modifier_shift, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_char, { 0, +1 } );
    
    ui_key_binding_add(os_key_left,  os_modifier_ctrl, ui_event_type_navigate, 0, ui_event_delta_unit_word, { -1, 0 } );
    ui_key_binding_add(os_key_right, os_modifier_ctrl, ui_event_type_navigate, 0, ui_event_delta_unit_word, { +1, 0 } );
    ui_key_binding_add(os_key_up,    os_modifier_ctrl, ui_event_type_navigate, 0, ui_event_delta_unit_word, { 0, -1 } );
    ui_key_binding_add(os_key_down,  os_modifier_ctrl, ui_event_type_navigate, 0, ui_event_delta_unit_word, { 0, +1 } );
    
    ui_key_binding_add(os_key_left,  os_modifier_shift | os_modifier_ctrl, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_word, { -1, 0 } );
    ui_key_binding_add(os_key_right, os_modifier_shift | os_modifier_ctrl, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_word, { +1, 0 } );
    ui_key_binding_add(os_key_up,    os_modifier_shift | os_modifier_ctrl, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_word, { 0, -1 } );
    ui_key_binding_add(os_key_down,  os_modifier_shift | os_modifier_ctrl, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_word, { 0, +1 } );
    
    ui_key_binding_add(os_key_home,  0, ui_event_type_navigate, 0, ui_event_delta_unit_line, { -1, 0 } );
    ui_key_binding_add(os_key_end,   0, ui_event_type_navigate, 0, ui_event_delta_unit_line, { +1, 0 } );
    ui_key_binding_add(os_key_home,  os_modifier_shift, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_line, { -1, 0 } );
    ui_key_binding_add(os_key_end,   os_modifier_shift, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_line, { +1, 0 } );
    
    ui_key_binding_add(os_key_home,  os_modifier_ctrl, ui_event_type_navigate, 0, ui_event_delta_unit_whole, { -1, 0 } );
    ui_key_binding_add(os_key_end,   os_modifier_ctrl, ui_event_type_navigate, 0, ui_event_delta_unit_whole, { +1, 0 } );
    ui_key_binding_add(os_key_home,  os_modifier_shift | os_modifier_ctrl, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_whole, { -1, 0 } );
    ui_key_binding_add(os_key_end,   os_modifier_shift | os_modifier_ctrl, ui_event_type_navigate, ui_event_flag_keep_mark, ui_event_delta_unit_whole, { +1, 0 } );
    
    ui_key_binding_add(os_key_delete,    0, ui_event_type_edit, ui_event_flag_delete | ui_event_flag_zero_delta, ui_event_delta_unit_char, { +1, 0 } );
    ui_key_binding_add(os_key_backspace, 0, ui_event_type_edit, ui_event_flag_delete | ui_event_flag_zero_delta, ui_event_delta_unit_char, { -1, 0 } );
    ui_key_binding_add(os_key_delete,    os_modifier_shift, ui_event_type_edit, ui_event_flag_delete | ui_event_flag_zero_delta, ui_event_delta_unit_char, { +1, 0 } );
    ui_key_binding_add(os_key_backspace, os_modifier_shift, ui_event_type_edit, ui_event_flag_delete | ui_event_flag_zero_delta, ui_event_delta_unit_char, { -1, 0 } );
    ui_key_binding_add(os_key_delete,    os_modifier_ctrl, ui_event_type_edit, ui_event_flag_delete | ui_event_flag_zero_delta, ui_event_delta_unit_word, { +1, 0 } );
    ui_key_binding_add(os_key_backspace, os_modifier_ctrl, ui_event_type_edit, ui_event_flag_delete | ui_event_flag_zero_delta, ui_event_delta_unit_word, { -1, 0 } );
    
    ui_context_set_active(nullptr);
    
    return context;
}

function void 
ui_context_release(ui_context_t* context) {
    
    gfx_texture_release(context->texture);
    
    gfx_buffer_release(context->instance_buffer);
    gfx_buffer_release(context->constant_buffer);
    
    arena_release(context->batch_arena);
    arena_release(context->event_arena);
    arena_release(context->drag_state_arena);
    arena_release(context->build_arenas[0]);
    arena_release(context->build_arenas[1]);
    arena_release(context->arena);
}

function void
ui_context_add_color(ui_context_t* context, str_t tags, color_t color) {
    
    temp_t scratch = scratch_begin();
    
    // break up string by spaces.
    str_t delim = str(" ");
    str_list_t str_list = str_split(scratch.arena, tags, delim.data, 1);
    
    // make key based on tags
    ui_key_t prev_key = { 0 };
    ui_key_t current_key = { 0 };
    for (str_node_t* node = str_list.first; node != nullptr; node = node->next) {
        current_key = ui_key_from_string(prev_key, node->string); 
        prev_key = current_key;
    }
    
    // add theme pattern
    ui_theme_pattern_t* theme_pattern = (ui_theme_pattern_t*)arena_alloc(context->arena, sizeof(ui_theme_pattern_t));
    theme_pattern->key = current_key;
    theme_pattern->color = color;
    
    u32 index = current_key.data[0] % context->theme_pattern_hash_list_count;
    ui_theme_pattern_hash_list_t* hash_list = &context->theme_pattern_hash_list[index];
    dll_push_back(hash_list->first, hash_list->last, theme_pattern);
    
    scratch_end(scratch);
}

function void
ui_context_set_active(ui_context_t* context) {
    ui_active_context = context;
}

//- key functions  

function ui_key_t 
ui_key_from_string(ui_key_t seed, str_t string) {
    
    ui_key_t result = { 0 };
    
    if (string.size != 0) {
        
        // find hash portion of the string string
        str_t hash_string = string;
        u32 pos = str_find_substr(string, str("###"));
        if (pos < string.size) {
            hash_string = str_skip(string, pos);
        }
        result.data[0] = str_hash(seed.data[0], hash_string);
        
    }
    
    return result;
}

function ui_key_t
ui_key_from_stringf(ui_key_t seed, char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_key_t result = ui_key_from_string(seed, string);
    
    return result;
}

function b8
ui_key_equals(ui_key_t a, ui_key_t b) {
    b8 result = (a.data[0] == b.data[0]);
    return result;
}

function b8
ui_key_is_hovered(ui_key_t key) {
    return ui_key_equals(ui_active_context->key_hovered, key);
}

function b8
ui_key_is_active(ui_key_t key) {
    b8 result =
        ui_key_equals(ui_active_context->key_active[0], key) ||
        ui_key_equals(ui_active_context->key_active[1], key) ||
        ui_key_equals(ui_active_context->key_active[2], key);
    return result;
}

function b8
ui_key_is_focused(ui_key_t key) {
    return ui_key_equals(ui_active_context->key_focused, key);
}


//- size functions 

inlnfunc ui_size_t 
ui_size(ui_size_type type, f32 value, f32 strictness) {
    return { type, value, strictness };
}

inlnfunc ui_size_t 
ui_size_pixels(f32 pixels, f32 strictness) {
    return { ui_size_type_pixel, pixels, strictness };
}

inlnfunc ui_size_t 
ui_size_percent(f32 percent) {
    return { ui_size_type_percent, percent, 0.0f };
}

inlnfunc ui_size_t 
ui_size_by_children(f32 strictness) {
    return { ui_size_type_by_children, 0.0f, strictness };
}

inlnfunc ui_size_t 
ui_size_by_text(f32 padding) {
    return { ui_size_type_by_text, padding, 1.0f };
}

//- axis/side/dir functions  

function ui_axis 
ui_axis_from_dir(ui_dir dir) {
    // ui_dir_left -> ui_axis_x;
    // ui_dir_up -> ui_axis_y;
    // ui_dir_right -> ui_axis_x;
    // ui_dir_down -> ui_axis_y;
    return ((dir & 1) ? ui_axis_y : ui_axis_x);
}

function ui_side 
ui_side_from_dir(ui_dir dir) {
    // ui_dir_left -> ui_side_min;
    // ui_dir_up -> ui_side_min;
    // ui_dir_right -> ui_side_max;
    // ui_dir_down -> ui_side_max;
    return ((dir < 2) ? ui_side_min : ui_side_max);
}

function ui_dir
ui_dir_from_axis_side(ui_axis axis, ui_side side) {
    ui_dir result = (axis == ui_axis_x) ? 
    (side == ui_side_min) ? ui_dir_left : ui_dir_right :
    (side == ui_side_min) ? ui_dir_up : ui_dir_down;
    return result;
}

//- text alignment functions 

function vec2_t 
ui_text_size(font_handle_t font, f32 font_size, str_t text) {
    vec2_t result = { 0 };
    result.x = font_text_get_width(font, font_size, text);
    result.y = font_text_get_height(font,font_size, text);
    return result;
}

function vec2_t 
ui_text_align(font_handle_t font, f32 font_size, str_t text, rect_t rect, ui_text_alignment alignment) {
    
    vec2_t result = { 0 };
    
    font_metrics_t font_metrics = font_get_metrics(font, font_size);
    f32 text_height = font_text_get_height(font, font_size, text);
    result.y = roundf(rect.y0 + (rect.y1 - rect.y0 - (text_height)) / 2.0f);
    
    switch (alignment) {
        default:
        case ui_text_align_left: {
            result.x = rect.x0 + 4.0f; // TODO: make this a text padding param for frames.
            break;
        }
        
        case ui_text_align_center: {
            f32 text_width = font_text_get_width(font, font_size, text);
            result.x = roundf((rect.x0 + rect.x1 - text_width) * 0.5f);
            result.x = max(result.x, rect.x0 + 4.0f);
            break;
        }
        
        case ui_text_align_right: {
            f32 text_width = font_text_get_width(font, font_size, text);
            result.x = roundf(rect.x1 - text_width - 4.0f);
            result.x = max(result.x, rect.x0 + 4.0f);
            break;
        }
        
    }
    result.x = floorf(result.x);
    return result;
}

//- text point functions 

function ui_text_point_t
ui_text_point(i32 line, i32 column) {
    ui_text_point_t result;
    result.line = line;
    result.column = column;
    return result;
}

function b8
ui_text_point_equals(ui_text_point_t a, ui_text_point_t b) {
    return (a.line == b.line && a.column == b.column);
}

function b8
ui_text_point_less_than(ui_text_point_t a, ui_text_point_t b) {
    b8 result = false;
    if(a.line < b.line) {
        result = true;
    } else if(a.line == b.line) {
        result = a.column < b.column;
    }
    return result;
}

function ui_text_point_t
ui_text_point_min(ui_text_point_t a, ui_text_point_t b) {
    ui_text_point_t result = b;
    if(ui_text_point_less_than(a, b)) {
        result = a;
    }
    return result;
}

function ui_text_point_t
ui_text_point_max(ui_text_point_t a, ui_text_point_t b) {
    ui_text_point_t result = a;
    if(ui_text_point_less_than(a, b)) {
        result = b;
    }
    return result;
}

//- text range functions 

function ui_text_range_t
ui_text_range(ui_text_point_t min, ui_text_point_t max) {
    ui_text_range_t range = { 0 };
    if (ui_text_point_less_than(min, max)) {
        range.min = min;
        range.max = max;
    } else {
        range.min = max;
        range.max = min;
    }
    return range;
}

function ui_text_range_t
ui_text_range_intersects(ui_text_range_t a, ui_text_range_t b) {
    ui_text_range_t result = { 0 };
    result.min = ui_text_point_max(a.min, b.min);
    result.max = ui_text_point_min(a.max, b.max);
    if (ui_text_point_less_than(result.max, result.min)) {
        memset(&result, 0, sizeof(ui_text_range_t));
    }
    return result;
}

function ui_text_range_t
ui_text_range_union(ui_text_range_t a, ui_text_range_t b) {
    ui_text_range_t result = { 0 };
    result.min = ui_text_point_min(a.min, b.min);
    result.max = ui_text_point_max(a.max, b.max);
    return result;
}

function b8
ui_text_range_contains(ui_text_range_t r, ui_text_point_t p) {
    b8 result = ((ui_text_point_less_than(r.min, p) || ui_text_point_equals(r.min, p)) &&
                 ui_text_point_less_than(p, r.max));
    return result;
}

//- text op functions

function ui_text_op_t 
ui_single_line_text_op_from_event(arena_t *arena, ui_event_t* event, str_t string, ui_text_point_t cursor, ui_text_point_t mark) {
    
    ui_text_point_t next_cursor = cursor;
    ui_text_point_t next_mark = mark;
    ui_text_range_t range = { 0 };
    str_t replace = { 0 };
    str_t copy = { 0 };
    ui_text_op_flags flags = 0;
    ivec2_t delta = event->delta;
    ivec2_t original_delta = delta;
    
    switch(event->delta_unit) {
        default:{ break; }
        
        case ui_event_delta_unit_char: {
            break;
        }
        
        case ui_event_delta_unit_word: {
            
            ui_side side = (delta.x > 0) ? ui_side_max : ui_side_min;
            
            b8 found_text = false;
            b8 found_non_space = false;
            
            i32 new_column = cursor.column;
            i32 inc_delta = (!!side)*2 - 1;
            i32 start_off = inc_delta < 0 ? inc_delta : 0;
            
            // start at the cursor and inc or dec depending on delta
            for(i32 col = cursor.column + start_off; 1 <= col && col <= string.size + 1; col += inc_delta) {
                
                u8 byte = (col <= string.size) ? string.data[col-1] : 0;
                
                b8 is_non_space = !char_is_space(byte);
                b8 is_name = (char_is_alpha(byte) || char_is_digit(byte) || byte == '_');
                
                if(((side == ui_side_min) && (col == 1)) || 
                   ((side == ui_side_max) && (col == string.size + 1)) ||
                   (found_non_space && !is_non_space) || 
                   (found_text && !is_name)) {
                    new_column = col + (!side && col != 1);  
                    break;
                } else if (!found_text && is_name) {
                    found_text = true;
                } else if (!found_non_space && is_non_space) {
                    found_non_space = true;
                }
            }
            
            delta.x = new_column - cursor.column;
            
            break;
        }
        
        case ui_event_delta_unit_line: 
        case ui_event_delta_unit_whole: 
        case ui_event_delta_unit_page: {
            i32 first_nonwhitespace_column = 1;
            for(i32 idx = 0; idx < string.size; idx += 1) {
                if(!char_is_space(string.data[idx])) {
                    first_nonwhitespace_column = (i32)idx + 1;
                    break;
                }
            }
            i32 home_dest_column = (cursor.column == first_nonwhitespace_column) ? 1 : first_nonwhitespace_column;
            delta.x = (delta.x > 0) ? ((i32)string.size+1 - cursor.column) : (home_dest_column - cursor.column);
            
            break;
        }
        
    }
    
    if(!ui_text_point_equals(cursor, mark) && event->flags & ui_event_flag_zero_delta) {
        delta = ivec2(0, 0);
    }
    
    if(ui_text_point_equals(cursor, mark) || !(event->flags & ui_event_flag_zero_delta)) {
        next_cursor.column += delta.x;
    }
    
    if(!ui_text_point_equals(cursor, mark) && event->flags & ui_event_flag_pick_side) {
        if(original_delta.x < 0 || original_delta.y < 0) {
            next_cursor = next_mark = ui_text_point_min(cursor, mark);
        } else if(original_delta.x > 0 || original_delta.y > 0) {
            next_cursor = next_mark = ui_text_point_max(cursor, mark);
        }
    }
    
    // deletion
    if(event->flags & ui_event_flag_delete) {
        ui_text_point_t new_pos = ui_text_point_min(next_cursor, next_mark);
        range = ui_text_range(next_cursor, next_mark);
        replace = str("");
        next_cursor = next_mark = new_pos;
    }
    
    // keep mark
    if(!(event->flags & ui_event_flag_keep_mark)) {
        next_mark = next_cursor;
    }
    
    // insertion
    if(event->os_event->string.size != 0) {
        range = ui_text_range(cursor, mark);
        replace = str_copy(arena, event->os_event->string);
        next_cursor = next_mark = ui_text_point(range.min.line, range.min.column + event->os_event->string.size);
    }
    
    // should event be taken
    if(next_cursor.column > string.size + 1 || 1 > next_cursor.column || event->delta.y != 0) {
        flags |= ui_text_op_flag_invalid;
    }
    
    next_cursor.column = clamp(next_cursor.column, 1, string.size + replace.size + 1);
    next_mark.column = clamp(next_mark.column, 1, string.size + replace.size + 1);
    
    
    ui_text_op_t result = { 0 };
    
    result.flags = flags;
    result.replace = replace;
    result.copy = copy;
    result.range = range;
    result.cursor = next_cursor;
    result.mark = next_mark;
    
    return result;
}

//- events functions 

function void 
ui_event_push(ui_event_t* event) {
    dll_push_back(ui_active_context->event_list.first, ui_active_context->event_list.last, event);
}

function void 
ui_event_pop(ui_event_t* event) {
    dll_remove(ui_active_context->event_list.first, ui_active_context->event_list.last, event);
}

//- key binding functions 

function void 
ui_key_binding_add(os_key key, os_modifiers modifiers, ui_event_type result_type, ui_event_flags result_flags, ui_event_delta_unit result_delta_unit, ivec2_t result_delta) {
    
    ui_context_t* context = ui_active_context;
    
    ui_key_binding_t* binding = (ui_key_binding_t*)arena_alloc(context->arena, sizeof(ui_key_binding_t));
    
    binding->key = key;
    binding->modifiers = modifiers;
    binding->result_type = result_type;
    binding->result_flags = result_flags;
    binding->result_delta_unit = result_delta_unit;
    binding->result_delta = result_delta;
    
    dll_push_back(context->key_binding_list.first, context->key_binding_list.last, binding);
}

function ui_key_binding_t* 
ui_key_binding_find(os_key key, os_modifiers modifiers) {
    // TODO: key binding lookup might be slow in future
    //if the binding are large. we can probably do a hash
    // lookup for speed. but this works for now.
    
    ui_key_binding_t* binding = nullptr;
    
    for (ui_key_binding_t* b = ui_active_context->key_binding_list.first; b != nullptr; b = b->next) {
        if (key == b->key && modifiers == b->modifiers) {
            binding = b;
            break;
        }
    }
    
    return binding;
}

//- theme functions 

function color_t 
ui_color_from_key(ui_key_t key) {
    
    color_t result = { 0 };
    
    u32 index = key.data[0] % ui_active_context->theme_pattern_hash_list_count;
    for (ui_theme_pattern_t* pattern = ui_active_context->theme_pattern_hash_list[index].first; pattern != nullptr; pattern = pattern->next) {
        if (ui_key_equals(pattern->key, key)) {
            result = pattern->color;
            break;
        }
    }
    
    return result;
}

//- animation functions 


function ui_anim_params_t 
ui_anim_params_create(f32 initial, f32 target, f32 rate) {
    return { initial, target, rate };
}

function f32 
ui_anim_ex(ui_key_t key, ui_anim_params_t params) {
    
    // get context
    ui_context_t* context = ui_active_context;
    i32 index = key.data[0] % context->anim_hash_list_count;
    
    // search for in list
    ui_anim_node_t* node = nullptr;
    if (!ui_key_equals(key, { 0 })) {
        for (ui_anim_node_t* n = context->anim_hash_list[index].first; n != nullptr; n = n->list_next) {
            if (ui_key_equals(key, n->key)) {
                node = n;
                break;
            }
        }
    }
    
    // if we didn't find one, allocate it
    if (node == nullptr) {
        node = context->anim_node_free;
        if (node != nullptr) {
            stack_pop_n(context->anim_node_free, list_next);
        } else {
            node = (ui_anim_node_t*)arena_alloc(context->arena, sizeof(ui_anim_node_t));
        }
        memset(node, 0, sizeof(ui_anim_node_t));
        
        // fill struct
        node->first_build_index = context->build_index;
        node->key = key;
        node->params = params;
        node->current = params.initial;
        
        // add to list
        dll_push_back_np(context->anim_hash_list[index].first, context->anim_hash_list[index].last, node, list_next, list_prev);
        
    } else {
        // remove from lru list
        dll_remove_np(context->anim_node_lru, context->anim_node_mru, node, lru_next, lru_prev);
    }
    
    // update node
    node->last_build_index = context->build_index;
    dll_push_back_np(context->anim_node_lru, context->anim_node_mru, node, lru_next, lru_prev);
    node->params = params;
    
    if (node->params.rate == 1.0f) {
        node->current = node->params.target;
    }
    
    return node->current;
}

function f32 
ui_anim(ui_key_t key, f32 initial, f32 target, f32 rate) {
    ui_anim_params_t params = { 0 };
	params.initial = initial;
	params.target = target;
	params.rate = rate;
	return ui_anim_ex(key, params);
}

//- drag state functions 

function void 
ui_drag_store_data(void* data, u32 size) {
    arena_clear(ui_active_context->drag_state_arena);
    ui_active_context->drag_state_data = arena_alloc(ui_active_context->drag_state_arena, size);
    ui_active_context->drag_state_size = size;
    memcpy(ui_active_context->drag_state_data, data, size);
}

function void*
ui_drag_get_data() {
    return ui_active_context->drag_state_data;
}

function void
ui_drag_clear_data() {
    arena_clear(ui_active_context->drag_state_arena);
    ui_active_context->drag_state_size = 0;
}

function b8
ui_drag_is_active() {
    return (ui_active_context->drag_state == ui_drag_state_dragging || ui_active_context->drag_state == ui_drag_state_dropping);
}

function void
ui_drag_begin(ui_key_t key ) {
    ui_drag_clear_data();
    if (!ui_drag_is_active()) {
        ui_active_context->drag_state = ui_drag_state_dragging;
        ui_active_context->key_drag = key;
        ui_active_context->drag_start_pos = ui_active_context->mouse_pos;
    }
}

function b8
ui_drag_drop() {
    b8 result = false;
    if (ui_active_context->drag_state == ui_drag_state_dropping) {
        result = true;
        ui_active_context->drag_state = ui_drag_state_null;
        ui_active_context->key_drag = { 0 }; // TODO: not sure if this is where we should clear the key.
    }
    return result;
}

function void
ui_drag_kill() {
    ui_active_context->drag_state = ui_drag_state_null;
    ui_active_context->key_drag = { 0 };
}

function vec2_t 
ui_drag_delta() {
    return vec2_sub(ui_active_context->mouse_pos, ui_active_context->drag_start_pos);
}


//- node functions 

function ui_node_t* 
ui_node_find(ui_key_t key) {
    
    ui_node_t* result = nullptr;
    
    if (!ui_key_equals(key, { 0 })) {
        u32 index = key.data[0] % ui_active_context->node_hash_list_count;
        for (ui_node_t* node = ui_active_context->node_hash_list[index].first; node != nullptr; node = node->list_next) {
            if (ui_key_equals(node->key, key)) {
                result = node;
                break;
            }
        }
    }
    
    return result;
}

function ui_node_t* 
ui_node_from_key(ui_node_flags flags, ui_key_t key) {
    
    ui_context_t* context = ui_active_context;
    ui_node_t* node = nullptr;
    
    // try to find existing node.
    node = ui_node_find(key);
    
    // filter out duplicates
    b8 node_is_new = (node == nullptr);
    if (!node_is_new && (node->last_build_index == context->build_index)) {
        node = nullptr;
        key = { 0 };
        node_is_new = true;
    }
    
    b8 node_is_transient = ui_key_equals(key, { 0 });
    
    // grab from free list or allocate one
    if (node_is_new) {
        node = node_is_transient ? nullptr : context->node_free;
        if (node != nullptr) {
            stack_pop_n(context->node_free, list_next);
        } else {
            node = (ui_node_t*)arena_alloc(node_is_transient ? ui_build_arena() : context->arena, sizeof(ui_node_t));
        }
        memset(node, 0, sizeof(ui_node_t));
        node->first_build_index = context->build_index;
    }
    node->last_build_index = context->build_index;
    
    // clear node
    node->tree_next = nullptr;
    node->tree_prev = nullptr;
    node->tree_parent = nullptr;
    node->tree_first = nullptr;
    node->tree_last = nullptr;
    node->flags = 0;
    
    // add to node list if needed
    if (node_is_new && !node_is_transient) {
        u32 index = key.data[0] % context->node_hash_list_count;
        dll_push_back_np(context->node_hash_list[index].first, context->node_hash_list[index].last, node, list_next, list_prev);
    }
    
    // add to node tree if needed
    ui_node_t* parent = ui_top_parent();
    if (parent != nullptr) {
        dll_push_back_np(parent->tree_first, parent->tree_last, node, tree_next, tree_prev);
        node->tree_parent = parent;
    }
    
    // fill node members
    node->key = key;
    node->group_key = ui_top_group_key();
    node->flags = (flags | ui_top_flags()) & ~ui_top_omit_flags();
    node->pos_fixed.x = ui_top_fixed_pos_x();
    node->pos_fixed.y = ui_top_fixed_pos_y();
    node->size_fixed.y = ui_top_fixed_size_y();
    node->size_fixed.x = ui_top_fixed_size_x();
    node->size_wanted[0] = ui_top_size_x();
    node->size_wanted[1] = ui_top_size_y();
    node->padding.x = ui_top_padding_x();
    node->padding.y = ui_top_padding_y();
    node->layout_dir = ui_top_layout_dir();
    node->text_alignment = ui_top_text_alignment();
    node->hover_cursor = ui_top_hover_cursor();
    node->rounding.x = ui_top_rounding_00();
    node->rounding.y = ui_top_rounding_01();
    node->rounding.z = ui_top_rounding_10();
    node->rounding.w = ui_top_rounding_11();
    node->border_size = ui_top_border_size();
    node->shadow_size = ui_top_shadow_size();
    node->texture = ui_top_texture();
    node->font = ui_top_font();
    node->font_size = ui_top_font_size();
    
    if (node->pos_fixed.x != 0.0f) {
        node->flags |= ui_flag_fixed_pos_x;
    }
    
    if (node->pos_fixed.y != 0.0f) {
        node->flags |= ui_flag_fixed_pos_y;
    }
    
    if (node->size_fixed.x != 0.0f) {
        node->flags |= ui_flag_fixed_size_x;
    }
    
    if (node->size_fixed.y != 0.0f) {
        node->flags |= ui_flag_fixed_size_y;
    }
    
    // tags
    node->tags_key = { 0 };
    if (ui_active_context->tags_stack_top != nullptr) {
        node->tags_key = ui_active_context->tags_stack_top->key;
    }
    
    // auto pop stacks
    ui_auto_pop_stacks();
    
    return node;
}

function ui_node_t*
ui_node_from_string(ui_node_flags flags, str_t string) {
    
    ui_key_t seed_key = ui_top_seed_key();
    ui_key_t key = ui_key_from_string(seed_key, string);
    ui_node_t* node = ui_node_from_key(flags, key);
    node->label = string;
    return node;
}

function ui_node_t*
ui_node_from_stringf(ui_node_flags flags, char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_node_t* node = ui_node_from_string(flags, string);
    return node;
}

function ui_node_rec_t 
ui_node_rec_depth_first(ui_node_t* node) {
    ui_node_rec_t rec = { 0 };
    
    if (node->tree_last != nullptr) {
        rec.next = node->tree_last;
        rec.push_count = 1;
    } else for (ui_node_t* parent = node; parent != nullptr; parent = parent->tree_parent) {
        if (parent->tree_prev != nullptr) {
            rec.next = parent->tree_prev;
            break;
        }
        rec.pop_count++;
    }
    
    return rec;
}

function void 
ui_node_set_custom_draw(ui_node_t* node, ui_node_custom_draw_func* func, void* data) {
    node->custom_draw_func = func;
    node->custom_draw_data = data;
}

//- layout functions 


function void 
ui_layout_solve_independent(ui_node_t* node, ui_axis axis) {
    
    // NOTE: this sets the size of nodes that are 
    // independent of other nodes. (ie: pixel, text).
    
    switch (node->size_wanted[axis].type) {
        
        case ui_size_type_pixel: {
            node->size[axis] = node->size_wanted[axis].value;
            break;
        }
        
        case ui_size_type_by_text: {
            f32 padding = node->size_wanted[axis].value;
            vec2_t text_size = ui_text_size(node->font, node->font_size, node->label);
            node->size[axis] = padding + text_size[axis] + 8.0f;
            break;
        }
        
    }
    
    if (node->flags & (ui_flag_fixed_pos_x << axis)) {
        node->pos[axis] = node->pos_fixed[axis];
    }
    
    if (node->flags & (ui_flag_fixed_size_x << axis)) {
        node->size[axis] = node->size_fixed[axis];
    }
    
    // recurse through children nodes
    for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
        ui_layout_solve_independent(child, axis);
    }
    
}

function void 
ui_layout_solve_upward_dependent(ui_node_t* node, ui_axis axis) {
    
    // NOTE: this sets the size of nodes that inherit 
    // their size from it's parent. (ie: percent)
    
    switch (node->size_wanted[axis].type) {
        case ui_size_type_percent: {
            
            // find a uitable parent
            ui_node_t* fixed_parent = nullptr;
            for (ui_node_t* parent = node->tree_parent; parent != nullptr; parent = parent->tree_parent) {
                if (parent->flags & (ui_flag_fixed_size_x << axis) ||
                    parent->size_wanted[axis].type == ui_size_type_pixel ||
                    parent->size_wanted[axis].type == ui_size_type_by_text ||
                    parent->size_wanted[axis].type == ui_size_type_percent) {
                    fixed_parent = parent;
                    break;
                }
            }
            
            if (fixed_parent != nullptr) {
                f32 parent_padding = fixed_parent->padding[axis] * 2.0f;
                f32 size = (fixed_parent->size[axis] - parent_padding) * node->size_wanted[axis].value;
                node->size[axis] = size;
            }
            
            break;
        }
    }
    
    // recurse through children nodes
    for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
        ui_layout_solve_upward_dependent(child, axis);
    }
    
}

function void 
ui_layout_solve_downward_dependent(ui_node_t* node, ui_axis axis) {
    
    // NOTE: this sets the size of nodes that inherit 
    // their size from it's children. (ie: by_children)
    
    // recurse through children nodes
    for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
        ui_layout_solve_downward_dependent(child, axis);
    }
    
    ui_axis layout_axis = ui_axis_from_dir(node->layout_dir);
    
    switch (node->size_wanted[axis].type) {
        case ui_size_type_by_children: {
            
            // add up children's sizes
            f32 sum = 0.0f;
            for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
                if (!(child->flags & (ui_flag_fixed_pos_x << axis))) {
                    if (axis == layout_axis) {
                        sum += child->size[axis];
                    } else {
                        sum = max(sum, child->size[axis]);
                    }
                }
            }
            
            f32 padding = node->padding[axis] * 2.0f;
            node->size[axis] = sum + padding;
            
            break;
        }
    }
    
}

function void 
ui_layout_solve_violations(ui_node_t* node, ui_axis axis) {
    
    temp_t scratch = scratch_begin();
    
    ui_axis layout_axis = ui_axis_from_dir(node->layout_dir);
    ui_side layout_side = ui_side_from_dir(node->layout_dir);
    
    f32 node_size = node->size[axis] - (node->padding[axis] * 2.0f);
    
    if (axis != layout_axis && !(node->flags & (ui_flag_overflow_x << axis))) {
        for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
            if (!(child->flags & (ui_flag_fixed_pos_x << axis))) {
                f32 child_size = child->size[axis];
                f32 violation = child_size - node_size;
                f32 fixup = clamp(violation, 0.0f, child_size);
                if (fixup > 0.0f) {
                    child->size[axis] -= fixup;
                }
            }
        }
    }
    
    if (axis == layout_axis && !(node->flags & (ui_flag_overflow_x << axis))) {
        
        f32 total_allowed_size = node_size;
        f32 total_size = 0.0f;
        f32 total_weighted_size = 0.0f;
        u32 child_count = 0;
        
        for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
            if (!(child->flags & (ui_flag_fixed_pos_x << axis))) {
                total_size += child->size[axis];
                total_weighted_size += child->size[axis] * (1.0f - child->size_wanted[axis].strictness);
            }
            child_count++;
        }
        
        f32 violation = total_size - total_allowed_size;
        if (violation > 0.0) {
            
            f32 child_fixup_sum = 0.0f;
            f32* child_fixups = (f32*)arena_alloc(scratch.arena, sizeof(f32) * child_count);
            u32 child_index = 0;
            
            for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
                if (!(child->flags & (ui_flag_fixed_pos_x << axis))) {
                    f32 fixup = child->size[axis] * (1.0f - child->size_wanted[axis].strictness);
                    fixup = max(fixup, 0.0f);
                    child_fixups[child_index] = fixup;
                    child_fixup_sum += fixup;
                }
                child_index++;
            }
            
            child_index = 0;
            for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
                if (!(child->flags & (ui_flag_fixed_pos_x << axis))) {
                    f32 fixup_percent = (violation / total_weighted_size);
                    fixup_percent = clamp(fixup_percent, 0.0f, 1.0f);
                    child->size[axis] -= child_fixups[child_index] * fixup_percent;
                }
                child_index++;
            }
        }
    }
    
    if (node->flags & (ui_flag_overflow_x << axis)) {
        for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
            if (child->size_wanted[axis].type == ui_size_type_percent) {
                child->size[axis] = node->size[axis] * child->size_wanted[axis].value;
            }
        }
    }
    
    // recurse through children nodes
    for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
        ui_layout_solve_violations(child, axis);
    }
    
    scratch_end(scratch);
    
}

function void 
ui_layout_set_positions(ui_node_t* node, ui_axis axis) {
    
    ui_axis layout_axis = ui_axis_from_dir(node->layout_dir);
    ui_side layout_side = ui_side_from_dir(node->layout_dir);
    
    f32 bounds = 0.0f;
    
    // starting layout position
    f32 layout_pos = node->padding[axis];
    if (layout_side == ui_side_min) {
        layout_pos = node->size[axis] - node->padding[axis];
    }
    
    f32 parent_pos = node->rect.v0[axis];
    
    for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
        
        // calculate pos
        if (!(child->flags & (ui_flag_fixed_pos_x << axis))) {
            
            child->pos_target[axis] = parent_pos + layout_pos;
            
            if (layout_axis == axis) {
                
                if (layout_side == ui_side_max) {
                    layout_pos += child->size[axis];
                } else {
                    layout_pos -= child->size[axis];
                }
                
                bounds += child->size[axis];
            } else {
                bounds = max(bounds, child->size[axis]);
            }
            
        } else {
            child->pos_target[axis] = parent_pos + child->pos_fixed[axis];
        }
        
        if (!(child->flags & (ui_flag_anim_pos_x << axis)) ||
            child->first_build_index == child->last_build_index) {
            child->pos[axis] = child->pos_target[axis];
        }
        
        b8 ignore_view_offset = (child->flags & (ui_flag_ignore_view_offset_x << axis));
        f32 view_offset = !ignore_view_offset * node->view_offset[axis];
        
        // set rect
        child->rect.v0[axis] = floorf(child->pos[axis] - view_offset);
        child->rect.v1[axis] = floorf(child->pos[axis] + child->size[axis] - view_offset);
        
    }
    
    // set view bounds
    node->view_bounds[axis] = bounds;
    
    // recurse through children nodes
    for (ui_node_t* child = node->tree_first; child != nullptr; child = child->tree_next) {
        ui_layout_set_positions(child, axis);
    }
    
}

//- interaction functions 

function ui_interaction 
ui_interaction_from_node(ui_node_t* node) {
    
    if (node == nullptr) { return 0; }
    
    ui_context_t* context = ui_active_context;
    ui_interaction result = ui_null;
    
    rect_t clipped_rect = node->rect;
    for (ui_node_t* parent = node->tree_parent; parent != nullptr; parent = parent->tree_parent) {
        if (parent->flags & ui_flag_clip) {
            clipped_rect = rect_intersection(clipped_rect, parent->rect);
        }
    }
    
    b8 mouse_in_bounds = rect_contains(clipped_rect, context->mouse_pos);
    
    for (ui_event_t* event = context->event_list.first, *next = nullptr; event != nullptr; event = next) {
        next = event->next;
        os_event_t* os_event = event->os_event;
        b8 taken = false;
        
        // mouse interactable
        if (node->flags & ui_flag_mouse_interactable) {
            
            // mouse press event
            if (event->type == ui_event_type_mouse_press && mouse_in_bounds) {
                
                // single click
                context->key_active[os_event->mouse] = node->key; 
                context->key_focused = node->key; 
                result |= (ui_left_pressed << os_event->mouse);
                
                // double and triple clicks
                if (context->click_counter[os_event->mouse] >= 2) {
                    if (context->click_counter[os_event->mouse] % 2 == 0) {
                        result |= ui_left_double_clicked << os_event->mouse;
                    } else {
                        result |= ui_left_triple_clicked << os_event->mouse;
                    }
                }
                
                taken = true;
            }
            
            
            // mouse release event
            if (event->type == ui_event_type_mouse_release) {
                
                // if we release on the frame but frame was not active
                if (mouse_in_bounds) {
                    result |= (ui_left_released << os_event->mouse);
                }
                
                // if frame was active
                if (ui_key_equals(context->key_active[os_event->mouse], node->key)) {
                    
                    // release if we were active
                    result |= (ui_left_released << os_event->mouse);
                    
                    // if we released on the frame and frame is active
                    if (mouse_in_bounds) {
                        result |= (ui_left_clicked << os_event->mouse);
                        taken = true;
                    }
                    
                    // if was active, reset active key
                    context->key_active[os_event->mouse] = { 0 };
                }
            }
            
            // mouse scroll event
            if (event->type == ui_event_type_mouse_scroll  && mouse_in_bounds) {
                
                // scrollable
                if (node->flags & ui_flag_scrollable) {
                    vec2_t scroll = os_event->scroll;
                    
                    if (os_event->modifiers & os_modifier_shift) {
                        scroll.x = os_event->scroll.y;
                        scroll.y = os_event->scroll.x;
                    }
                    
                    taken = true;
                }
                
                // view scroll
                if (node->flags & ui_flag_view_scroll && !(os_event->modifiers & os_modifier_ctrl) ) {
                    
                    vec2_t scroll = os_event->scroll;
                    
                    if (os_event->modifiers & os_modifier_shift) {
                        scroll.x = os_event->scroll.y;
                        scroll.y = os_event->scroll.x;
                    }
                    
                    scroll = vec2_mul(scroll, -17.0f);
                    
                    if (!(node->flags & ui_flag_view_scroll_x)) {
                        scroll.x = 0.0f;
                    }
                    
                    if (!(node->flags & ui_flag_view_scroll_y)) {
                        scroll.y = 0.0f;
                    }
                    
                    node->view_offset_target = vec2_add(node->view_offset_target, scroll);
                    
                    // clamp view scroll
                    if (node->flags & ui_flag_view_clamp_x) {
                        node->view_offset_target.x = clamp(node->view_offset_target.x, 0.0f, max(0.0f, node->view_bounds.x - node->size.x));
                    }
                    
                    if (node->flags & ui_flag_view_clamp_y) {
                        node->view_offset_target.y = clamp(node->view_offset_target.y, 0.0f, max(0.0f, node->view_bounds.y - node->size.y));
                    }
                    
                    taken = true;
                }
                
                
            }
            
        }
        
        if (taken) {
            dll_remove(context->event_list.first, context->event_list.last, event);
            os_event_pop(os_event);
        }
    }
    
    // mouse dragging
    if (node->flags & ui_flag_mouse_interactable) {
        
        for (i32 mouse_button = 0; mouse_button < os_mouse_button_count; mouse_button++) {
            
            // single dragging
            if (ui_key_equals(context->key_active[mouse_button], node->key)) {
                result |= ui_left_dragging << mouse_button;
            }
            
            // double and triple dragging
            if (result & (ui_left_dragging << mouse_button) &&
                context->click_counter[mouse_button] >= 2) {
                if (context->click_counter[mouse_button] % 2 == 0) {
                    result |= ui_left_double_dragging << mouse_button;
                } else {
                    result |= ui_left_triple_dragging << mouse_button;
                }
            }
            
        }
        
    }
    
    
    // mouse hovering
    if ((node->flags & ui_flag_mouse_interactable) && mouse_in_bounds) {
        
        // if we are dragging
        if (ui_drag_is_active() && ui_key_equals(context->key_hovered, { 0 })) {
            result |= ui_hovered;
            context->key_hovered = node->key;
        }else if ((ui_key_equals(context->key_hovered, {0}) || ui_key_equals(context->key_hovered, node->key)) &&
                  (ui_key_equals(context->key_active[os_mouse_button_left], {0}) || ui_key_equals(context->key_active[os_mouse_button_left], node->key)) &&
                  (ui_key_equals(context->key_active[os_mouse_button_middle], {0}) || ui_key_equals(context->key_active[os_mouse_button_middle], node->key)) &&
                  (ui_key_equals(context->key_active[os_mouse_button_right], {0}) || ui_key_equals(context->key_active[os_mouse_button_right], node->key))) {
            context->key_hovered = node->key;
            result |= ui_hovered;
        }
        
        // mouse over even if not active
        result |= ui_mouse_over;
    }
    
    return result;
    
}


//- renderer functions


function ui_r_instance_t*
ui_r_get_instance() {
    
	// find a batch
	ui_r_batch_t* batch = nullptr;
    
	// search batch list
	for (ui_r_batch_t* b = ui_active_context->batch_first; b != 0; b = b->next) {
        
		// if batch has space, and texture matches
		if (((b->instance_count + 1) * sizeof(ui_r_instance_t)) < (kilobytes(256))) {
			batch = b;
			break;
		}
	}
    
	// else create one
	if (batch == nullptr) {
        
		batch = (ui_r_batch_t*)arena_alloc(ui_active_context->batch_arena, sizeof(ui_r_batch_t));
		
		batch->instances = (ui_r_instance_t*)arena_alloc(ui_active_context->batch_arena, kilobytes(256));
		batch->instance_count = 0;
		
		// add to batch list
		dll_push_back(ui_active_context->batch_first, ui_active_context->batch_last, batch);
        
	}
    
	// get instance
	ui_r_instance_t* instance = &batch->instances[batch->instance_count++];
	memset(instance, 0, sizeof(ui_r_instance_t));
    
	return instance;
}

function i32
ui_r_get_texture_index(gfx_handle_t texture) {
    
	// find index if in list
	i32 index = 0;
	for (; index < ui_active_context->texture_count; index++) {
		if (gfx_handle_equals(texture, ui_active_context->texture_list[index])) {
			break;
		}
	}
    
	// we didn't find one, add to list
	if (index == ui_active_context->texture_count) {
		ui_active_context->texture_list[ui_active_context->texture_count] = texture;
		ui_active_context->texture_count++;
	}
    
	return index;
    
}

function i32
ui_r_get_clip_mask_index(rect_t rect) {
    
	// find index if in list
	i32 index = 0;
	for (; index < ui_active_context->clip_mask_count; index++) {
		if (rect_equals(rect, ui_active_context->constants.clip_masks[index])) {
			break;
		}
	}
    
	// we didn't find one, add to list
	if (index == ui_active_context->clip_mask_count) {
		ui_active_context->constants.clip_masks[ui_active_context->clip_mask_count] = rect;
		ui_active_context->clip_mask_count++;
	}
    
	return index;
}

inlnfunc u32
ui_r_pack_indices(u32 shape, u32 texture, u32 clip) {
	return (shape << 24) | (texture << 16) | (clip << 8);
}


function void 
ui_r_draw_rect(rect_t rect) {
    
	ui_r_instance_t* instance = ui_r_get_instance();
    
	rect_validate(rect);
    
	instance->bbox = rect;
	instance->tex = {0.0f, 0.0f, 1.0f, 1.0f};
	
	instance->color0 = ui_r_top_color0();
	instance->color1 = ui_r_top_color1();
	instance->color2 = ui_r_top_color2();
	instance->color3 = ui_r_top_color3();
    
	instance->radii = ui_r_top_rounding();
	instance->thickness = ui_r_top_thickness();
	instance->softness = ui_r_top_softness();
    
	instance->indices = ui_r_pack_indices(ui_r_shape_rect,
                                          ui_r_get_texture_index(ui_r_top_texture()),
                                          ui_r_get_clip_mask_index(ui_r_top_clip_mask()));
    
	ui_r_auto_pop_stacks();
}

function void
ui_r_draw_image(rect_t rect) {
    
	ui_r_instance_t* instance = ui_r_get_instance();
    
	rect_validate(rect);
    
	instance->bbox = rect;
	instance->tex = { 0.0f, 0.0f, 1.0f, 1.0f };
    
	instance->color0 = ui_r_top_color0();
	instance->color1 = ui_r_top_color1();
	instance->color2 = ui_r_top_color2();
	instance->color3 = ui_r_top_color3();
    
	instance->radii = ui_r_top_rounding();
	instance->thickness = ui_r_top_thickness();
	instance->softness = ui_r_top_softness();
    
	instance->indices = ui_r_pack_indices(ui_r_shape_rect,
                                          ui_r_get_texture_index(ui_r_top_texture()),
                                          ui_r_get_clip_mask_index(ui_r_top_clip_mask()));
    
	ui_r_auto_pop_stacks();
    
}

function void
ui_r_draw_quad(vec2_t p0, vec2_t p1, vec2_t p2, vec2_t p3) {
    
	// order: (0, 0), (0, 1), (1, 1), (1, 0);
    
	ui_r_instance_t* instance = ui_r_get_instance();
    
	f32 softness = ui_r_top_softness();
    
	f32 min_x = min(min(min(p0.x, p1.x), p2.x), p3.x);
	f32 min_y = min(min(min(p0.y, p1.y), p2.y), p3.y);
	f32 max_x = max(max(max(p0.x, p1.x), p2.x), p3.x);
	f32 max_y = max(max(max(p0.y, p1.y), p2.y), p3.y);
    
	rect_t bbox = rect_grow(rect(min_x, min_y, max_x, max_y), softness);
    
	vec2_t c = rect_center(bbox);
	vec2_t c_p0 = vec2_sub(p0, c);
	vec2_t c_p1 = vec2_sub(p1, c);
	vec2_t c_p2 = vec2_sub(p2, c);
	vec2_t c_p3 = vec2_sub(p3, c);
    
	instance->bbox = bbox;
    
	instance->color0 = ui_r_top_color0();
	instance->color1 = ui_r_top_color1();
	instance->color2 = ui_r_top_color2();
	instance->color3 = ui_r_top_color3();
    
	instance->point0 = c_p0;
	instance->point1 = c_p1;
	instance->point2 = c_p2;
	instance->point3 = c_p3;
    
	instance->thickness = ui_r_top_thickness();
	instance->softness = softness;
    
	instance->indices = ui_r_pack_indices(ui_r_shape_quad,
                                          ui_r_get_texture_index(ui_r_top_texture()),
                                          ui_r_get_clip_mask_index(ui_r_top_clip_mask()));
    
	ui_r_auto_pop_stacks();
}

function void
ui_r_draw_line(vec2_t p0, vec2_t p1) {
    
	ui_r_instance_t* instance = ui_r_get_instance();
    
	f32 thickness = ui_r_top_thickness();
	f32 softness = ui_r_top_softness();
    
	f32 min_x = min(p0.x, p1.x);
	f32 min_y = min(p0.y, p1.y);
	f32 max_x = max(p0.x, p1.x);
	f32 max_y = max(p0.y, p1.y);
    
	rect_t bbox = rect_grow(rect(min_x, min_y, max_x, max_y), softness + thickness + 2.0f);
    
	vec2_t c = rect_center(bbox);
	vec2_t c_p0 = vec2_sub(c, p0);
	vec2_t c_p1 = vec2_sub(c, p1);
    
	instance->bbox = bbox;
    
	instance->color0 = ui_r_top_color0();
	instance->color1 = ui_r_top_color1();
    
	instance->point0 = c_p0;
	instance->point1 = c_p1;
    
	instance->thickness = thickness;
	instance->softness = softness;
    
	instance->indices = ui_r_pack_indices(ui_r_shape_line,
                                          ui_r_get_texture_index(ui_r_top_texture()),
                                          ui_r_get_clip_mask_index(ui_r_top_clip_mask()));
    
	ui_r_auto_pop_stacks();
}

function void 
ui_r_draw_circle(vec2_t pos, f32 radius, f32 start_angle, f32 end_angle) {
	
	ui_r_instance_t* instance = ui_r_get_instance();
	
	f32 softness = ui_r_top_softness();
    
	instance->bbox = rect_grow(rect(pos.x - radius, pos.y - radius, pos.x + radius, pos.y + radius), softness);
    
	instance->color0 = ui_r_top_color0();
	instance->color1 = ui_r_top_color1();
	instance->color2 = ui_r_top_color2();
	instance->color3 = ui_r_top_color3();
    
	instance->point0 = vec2(radians(start_angle), radians(end_angle));
	
	instance->thickness = ui_r_top_thickness();
	instance->softness = softness;
    
	instance->indices = ui_r_pack_indices(ui_r_shape_circle,
                                          ui_r_get_texture_index(ui_r_top_texture()),
                                          ui_r_get_clip_mask_index(ui_r_top_clip_mask()));
    
    
	ui_r_auto_pop_stacks();
} 

function void
ui_r_draw_tri(vec2_t p0, vec2_t p1, vec2_t p2) {
    
	ui_r_instance_t* instance = ui_r_get_instance();
    
	f32 softness = ui_r_top_softness();
    
	f32 min_x = min(min(p0.x, p1.x), p2.x);
	f32 min_y = min(min(p0.y, p1.y), p2.y);
	f32 max_x = max(max(p0.x, p1.x), p2.x);
	f32 max_y = max(max(p0.y, p1.y), p2.y);
    
	rect_t bbox = rect_grow(rect(min_x, min_y, max_x, max_y), softness * 5.0f);
    
	vec2_t c = rect_center(bbox);
	vec2_t c_p0 = vec2_sub(p0, c);
	vec2_t c_p1 = vec2_sub(p1, c);
	vec2_t c_p2 = vec2_sub(p2, c);
    
	instance->bbox = bbox;
    
	instance->color0 = ui_r_top_color0();
	instance->color1 = ui_r_top_color1();
	instance->color2 = ui_r_top_color2();
    
	instance->point0 = c_p0;
	instance->point1 = c_p1;
	instance->point2 = c_p2;
    
	instance->radii = ui_r_top_rounding();
    
	instance->thickness = ui_r_top_thickness();
	instance->softness = softness;
    
	instance->indices = ui_r_pack_indices(ui_r_shape_tri,
                                          ui_r_get_texture_index(ui_r_top_texture()),
                                          ui_r_get_clip_mask_index(ui_r_top_clip_mask()));
    
    
	ui_r_auto_pop_stacks();
}

function void
ui_r_draw_text(str_t text, vec2_t pos) {
    
	f32 font_size = ui_r_top_font_size();
	font_handle_t font = ui_r_top_font();
    
	for (u32 i = 0; i < text.size; i++) {
        
		ui_r_instance_t* instance = ui_r_get_instance();
		
		u8 codepoint = *(text.data + i);
		font_glyph_t* glyph = font_get_glyph(font, font_size, codepoint);
        
		instance->bbox = rect(pos.x, pos.y, pos.x + glyph->pos.x1, pos.y + glyph->pos.y1);
		instance->tex = glyph->uv;
        
		instance->color0 = ui_r_top_color0();
		instance->color1 = ui_r_top_color1();
		instance->color2 = ui_r_top_color2();
		instance->color3 = ui_r_top_color3();
        
		instance->indices = ui_r_pack_indices(ui_r_shape_rect,
                                              ui_r_get_texture_index(font_state.atlas_texture),
                                              ui_r_get_clip_mask_index(ui_r_top_clip_mask()));
        
		pos.x += glyph->advance;
	}
    
	ui_r_auto_pop_stacks();
}



//- stack functions 

function void 
ui_auto_pop_stacks() {
#define ui_stack(name, type) if (ui_active_context->name##_stack.auto_pop) { ui_pop_##name(); ui_active_context->name##_stack.auto_pop = false; };
    ui_stack_list
#undef ui_stack
}

#define ui_stack_top_impl(name, type)\
return ui_active_context->name##_stack.top->v;

#define ui_stack_push_impl(name, type)\
ui_##name##_node_t* node = ui_active_context->name##_stack.free;\
if (node != nullptr) {\
stack_pop(ui_active_context->name##_stack.free);\
} else {\
node = (ui_##name##_node_t*)arena_alloc(ui_build_arena(), sizeof(ui_##name##_node_t));\
}\
type old_value = ui_active_context->name##_stack.top->v; node->v = v;\
stack_push(ui_active_context->name##_stack.top, node);\
ui_active_context->name##_stack.auto_pop = false;\
return old_value;


#define ui_stack_pop_impl(name, type, default)\
ui_##name##_node_t* popped = ui_active_context->name##_stack.top;\
type result = default;\
if (popped != nullptr) {\
result = popped->v;\
stack_pop(ui_active_context->name##_stack.top);\
stack_push(ui_active_context->name##_stack.free, popped);\
ui_active_context->name##_stack.auto_pop = false;\
}\
return result;


#define ui_stack_set_next_impl(name, type)\
ui_##name##_node_t* node = ui_active_context->name##_stack.free;\
if (node != nullptr) {\
stack_pop(ui_active_context->name##_stack.free);\
} else {\
node = (ui_##name##_node_t*)arena_alloc(ui_build_arena(), sizeof(ui_##name##_node_t));\
}\
type old_value = ui_active_context->name##_stack.top->v;\
node->v = v;\
stack_push(ui_active_context->name##_stack.top, node);\
ui_active_context->name##_stack.auto_pop = true;\
return old_value;

function ui_node_t* ui_top_parent() { ui_stack_top_impl(parent, ui_node_t*) }
function ui_node_t* ui_push_parent(ui_node_t* v) { ui_stack_push_impl(parent, ui_node_t*) }
function ui_node_t* ui_pop_parent() { ui_stack_pop_impl(parent, ui_node_t*, nullptr) }
function ui_node_t* ui_set_next_parent(ui_node_t* v) { ui_stack_set_next_impl(parent, ui_node_t*) }

function ui_node_flags ui_top_flags() { ui_stack_top_impl(flags, ui_node_flags) }
function ui_node_flags ui_push_flags(ui_node_flags v) { ui_stack_push_impl(flags, ui_node_flags) }
function ui_node_flags ui_pop_flags() { ui_stack_pop_impl(flags, ui_node_flags, 0) }
function ui_node_flags ui_set_next_flags(ui_node_flags v) { ui_stack_set_next_impl(flags, ui_node_flags) }

function ui_node_flags ui_top_omit_flags() { ui_stack_top_impl(omit_flags, ui_node_flags) }
function ui_node_flags ui_push_omit_flags(ui_node_flags v) { ui_stack_push_impl(omit_flags, ui_node_flags) }
function ui_node_flags ui_pop_omit_flags() { ui_stack_pop_impl(omit_flags, ui_node_flags, 0) }
function ui_node_flags ui_set_next_omit_flags(ui_node_flags v) { ui_stack_set_next_impl(omit_flags, ui_node_flags) }

function ui_key_t ui_top_seed_key() { ui_stack_top_impl(seed_key, ui_key_t) }
function ui_key_t ui_push_seed_key(ui_key_t v) { ui_stack_push_impl(seed_key, ui_key_t) }
function ui_key_t ui_pop_seed_key() { ui_stack_pop_impl(seed_key, ui_key_t, { 0 }) }
function ui_key_t ui_set_next_seed_key(ui_key_t v) { ui_stack_set_next_impl(seed_key, ui_key_t) }

function ui_key_t ui_top_group_key() { ui_stack_top_impl(group_key, ui_key_t) }
function ui_key_t ui_push_group_key(ui_key_t v) { ui_stack_push_impl(group_key, ui_key_t) }
function ui_key_t ui_pop_group_key() { ui_stack_pop_impl(group_key, ui_key_t, { 0 }) }
function ui_key_t ui_set_next_group_key(ui_key_t v) { ui_stack_set_next_impl(group_key, ui_key_t) }

function f32 ui_top_fixed_size_x() { ui_stack_top_impl(fixed_size_x, f32) }
function f32 ui_push_fixed_size_x(f32 v) { ui_stack_push_impl(fixed_size_x, f32) }
function f32 ui_pop_fixed_size_x() { ui_stack_pop_impl(fixed_size_x, f32, 0.0f) }
function f32 ui_set_next_fixed_size_x(f32 v) { ui_stack_set_next_impl(fixed_size_x, f32) }

function f32 ui_top_fixed_size_y() { ui_stack_top_impl(fixed_size_y, f32) }
function f32 ui_push_fixed_size_y(f32 v) { ui_stack_push_impl(fixed_size_y, f32) }
function f32 ui_pop_fixed_size_y() { ui_stack_pop_impl(fixed_size_y, f32, 0.0f) }
function f32 ui_set_next_fixed_size_y(f32 v) { ui_stack_set_next_impl(fixed_size_y, f32) }

function f32 ui_top_fixed_pos_x() { ui_stack_top_impl(fixed_pos_x, f32) }
function f32 ui_push_fixed_pos_x(f32 v) { ui_stack_push_impl(fixed_pos_x, f32) }
function f32 ui_pop_fixed_pos_x() { ui_stack_pop_impl(fixed_pos_x, f32, 0.0f) }
function f32 ui_set_next_fixed_pos_x(f32 v) { ui_stack_set_next_impl(fixed_pos_x, f32) }

function f32 ui_top_fixed_pos_y() { ui_stack_top_impl(fixed_pos_y, f32) }
function f32 ui_push_fixed_pos_y(f32 v) { ui_stack_push_impl(fixed_pos_y, f32) }
function f32 ui_pop_fixed_pos_y() { ui_stack_pop_impl(fixed_pos_y, f32, 0.0f) }
function f32 ui_set_next_fixed_pos_y(f32 v) { ui_stack_set_next_impl(fixed_pos_y, f32) }

function ui_size_t ui_top_size_x() { ui_stack_top_impl(size_x, ui_size_t) }
function ui_size_t ui_push_size_x(ui_size_t v) { ui_stack_push_impl(size_x, ui_size_t) }
function ui_size_t ui_pop_size_x() { ui_stack_pop_impl(size_x, ui_size_t, { 0 }) }
function ui_size_t ui_set_next_size_x(ui_size_t v) { ui_stack_set_next_impl(size_x, ui_size_t) }

function ui_size_t ui_top_size_y() { ui_stack_top_impl(size_y, ui_size_t) }
function ui_size_t ui_push_size_y(ui_size_t v) { ui_stack_push_impl(size_y, ui_size_t) }
function ui_size_t ui_pop_size_y() { ui_stack_pop_impl(size_y, ui_size_t, { 0 }) }
function ui_size_t ui_set_next_size_y(ui_size_t v) { ui_stack_set_next_impl(size_y, ui_size_t) }

function f32 ui_top_padding_x() { ui_stack_top_impl(padding_x, f32) }
function f32 ui_push_padding_x(f32 v) { ui_stack_push_impl(padding_x, f32) }
function f32 ui_pop_padding_x() { ui_stack_pop_impl(padding_x, f32, 0.0f) }
function f32 ui_set_next_padding_x(f32 v) { ui_stack_set_next_impl(padding_x, f32) }

function f32 ui_top_padding_y() { ui_stack_top_impl(padding_y, f32) }
function f32 ui_push_padding_y(f32 v) { ui_stack_push_impl(padding_y, f32) }
function f32 ui_pop_padding_y() { ui_stack_pop_impl(padding_y, f32, 0.0f) }
function f32 ui_set_next_padding_y(f32 v) { ui_stack_set_next_impl(padding_y, f32) }

function ui_dir ui_top_layout_dir() { ui_stack_top_impl(layout_dir, ui_dir) }
function ui_dir ui_push_layout_dir(ui_dir v) { ui_stack_push_impl(layout_dir, ui_dir) }
function ui_dir ui_pop_layout_dir() { ui_stack_pop_impl(layout_dir, ui_dir, 0) }
function ui_dir ui_set_next_layout_dir(ui_dir v) { ui_stack_set_next_impl(layout_dir, ui_dir) }

function ui_text_alignment ui_top_text_alignment() { ui_stack_top_impl(text_alignment, ui_text_alignment) }
function ui_text_alignment ui_push_text_alignment(ui_text_alignment v) { ui_stack_push_impl(text_alignment, ui_text_alignment) }
function ui_text_alignment ui_pop_text_alignment() { ui_stack_pop_impl(text_alignment, ui_text_alignment, 0) }
function ui_text_alignment ui_set_next_text_alignment(ui_text_alignment v) { ui_stack_set_next_impl(text_alignment, ui_text_alignment) }

function os_cursor ui_top_hover_cursor() { ui_stack_top_impl(hover_cursor, os_cursor) }
function os_cursor ui_push_hover_cursor(os_cursor v) { ui_stack_push_impl(hover_cursor, os_cursor) }
function os_cursor ui_pop_hover_cursor() { ui_stack_pop_impl(hover_cursor, os_cursor, (os_cursor)0) }
function os_cursor ui_set_next_hover_cursor(os_cursor v) { ui_stack_set_next_impl(hover_cursor, os_cursor) }

function f32 ui_top_rounding_00() { ui_stack_top_impl(rounding_00, f32) }
function f32 ui_push_rounding_00(f32 v) { ui_stack_push_impl(rounding_00, f32) }
function f32 ui_pop_rounding_00() { ui_stack_pop_impl(rounding_00, f32, 0.0f) }
function f32 ui_set_next_rounding_00(f32 v) { ui_stack_set_next_impl(rounding_00, f32) }

function f32 ui_top_rounding_01() { ui_stack_top_impl(rounding_01, f32) }
function f32 ui_push_rounding_01(f32 v) { ui_stack_push_impl(rounding_01, f32) }
function f32 ui_pop_rounding_01() { ui_stack_pop_impl(rounding_01, f32, 0.0f) }
function f32 ui_set_next_rounding_01(f32 v) { ui_stack_set_next_impl(rounding_01, f32) }

function f32 ui_top_rounding_10() { ui_stack_top_impl(rounding_10, f32) }
function f32 ui_push_rounding_10(f32 v) { ui_stack_push_impl(rounding_10, f32) }
function f32 ui_pop_rounding_10() { ui_stack_pop_impl(rounding_10, f32, 0.0f) }
function f32 ui_set_next_rounding_10(f32 v) { ui_stack_set_next_impl(rounding_10, f32) }

function f32 ui_top_rounding_11() { ui_stack_top_impl(rounding_11, f32) }
function f32 ui_push_rounding_11(f32 v) { ui_stack_push_impl(rounding_11, f32) }
function f32 ui_pop_rounding_11() { ui_stack_pop_impl(rounding_11, f32, 0.0f) }
function f32 ui_set_next_rounding_11(f32 v) { ui_stack_set_next_impl(rounding_11, f32) }

function f32 ui_top_border_size() { ui_stack_top_impl(border_size, f32) }
function f32 ui_push_border_size(f32 v) { ui_stack_push_impl(border_size, f32) }
function f32 ui_pop_border_size() { ui_stack_pop_impl(border_size, f32, 0.0f) }
function f32 ui_set_next_border_size(f32 v) { ui_stack_set_next_impl(border_size, f32) }

function f32 ui_top_shadow_size() { ui_stack_top_impl(shadow_size, f32) }
function f32 ui_push_shadow_size(f32 v) { ui_stack_push_impl(shadow_size, f32) }
function f32 ui_pop_shadow_size() { ui_stack_pop_impl(shadow_size, f32, 0.0f) }
function f32 ui_set_next_shadow_size(f32 v) { ui_stack_set_next_impl(shadow_size, f32) }

function gfx_handle_t ui_top_texture() { ui_stack_top_impl(texture, gfx_handle_t) }
function gfx_handle_t ui_push_texture(gfx_handle_t v) { ui_stack_push_impl(texture, gfx_handle_t) }
function gfx_handle_t ui_pop_texture() { ui_stack_pop_impl(texture, gfx_handle_t, { 0 }) }
function gfx_handle_t ui_set_next_texture(gfx_handle_t v) { ui_stack_set_next_impl(texture, gfx_handle_t) }

function font_handle_t ui_top_font() { ui_stack_top_impl(font, font_handle_t) }
function font_handle_t ui_push_font(font_handle_t v) { ui_stack_push_impl(font, font_handle_t) }
function font_handle_t ui_pop_font() { ui_stack_pop_impl(font, font_handle_t, { 0 }) }
function font_handle_t ui_set_next_font(font_handle_t v) { ui_stack_set_next_impl(font, font_handle_t) }

function f32 ui_top_font_size() { ui_stack_top_impl(font_size, f32) }
function f32 ui_push_font_size(f32 v) { ui_stack_push_impl(font_size, f32) }
function f32 ui_pop_font_size() { ui_stack_pop_impl(font_size, f32, 0.0f) }
function f32 ui_set_next_font_size(f32 v) { ui_stack_set_next_impl(font_size, f32) }

function ui_key_t 
ui_top_tags_key() {
    ui_key_t result = { 0 };
    if (ui_active_context->tags_stack_top != nullptr) {
        result = ui_active_context->tags_stack_top->key;
    }
    return result;
}

function str_t ui_top_tag() { ui_stack_top_impl(tag, str_t) }

// tags

function str_t 
ui_push_tag(str_t v) {
    
    // get seed key
    ui_key_t seed_key = { 0 };
    if (ui_active_context->tags_stack_top != nullptr) {
        seed_key = ui_active_context->tags_stack_top->key;
    }
    
    // create key
    ui_key_t key = seed_key;
    if (v.size > 0) {
        key = ui_key_from_string(seed_key, v);
    }
    
    // get or allocate stack node
    ui_tags_stack_node_t* tags_stack_node = ui_active_context->tags_stack_free;
    if (tags_stack_node != nullptr) {
        stack_pop(ui_active_context->tags_stack_free);
    } else {
        tags_stack_node = (ui_tags_stack_node_t*)arena_alloc(ui_build_arena(), sizeof(ui_tags_stack_node_t)); 
    }
    stack_push(ui_active_context->tags_stack_top, tags_stack_node);
    tags_stack_node->key = key;
    
    // store tag in cache
    u32 index = key.data[0] % ui_active_context->tags_hash_list_count;
    ui_tags_hash_list_t* hash_list = &ui_active_context->tags_hash_list[index];
    
    // try to find tag in cache first
    ui_tags_node_t* tag_node = nullptr;
    for (ui_tags_node_t* n = hash_list->first; n != nullptr; n = n->next) {
        if (ui_key_equals(n->key, key)) {
            tag_node = n;
            break;
        }
    }
    
    // create if we didn't find one
    if (tag_node == nullptr) {
        tag_node = (ui_tags_node_t*)arena_alloc(ui_build_arena(), sizeof(ui_tags_node_t));
        stack_push(hash_list->first, tag_node);
        tag_node->key = key;
    }
    
    ui_stack_push_impl(tag, str_t)
}

function str_t
ui_push_tagf(char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_push_tag(string);
    
    return string;
}

function str_t 
ui_pop_tag() {
    if(ui_active_context->tags_stack_top != nullptr) {
        ui_tags_stack_node_t* popped = ui_active_context->tags_stack_top;
        stack_pop(ui_active_context->tags_stack_top);
        stack_push(ui_active_context->tags_stack_free, popped);
    }
    ui_stack_pop_impl(tag, str_t, { 0 })
}

function str_t 
ui_set_next_tag(str_t v) {
    
    // get seed key
    ui_key_t seed_key = { 0 };
    if (ui_active_context->tags_stack_top != nullptr) {
        seed_key = ui_active_context->tags_stack_top->key;
    }
    
    // create key
    ui_key_t key = seed_key;
    if (v.size > 0) {
        key = ui_key_from_string(seed_key, v);
    }
    
    // get or allocate stack node
    ui_tags_stack_node_t* tags_stack_node = ui_active_context->tags_stack_free;
    if (tags_stack_node != nullptr) {
        stack_pop(ui_active_context->tags_stack_free);
    } else {
        tags_stack_node = (ui_tags_stack_node_t*)arena_alloc(ui_build_arena(), sizeof(ui_tags_stack_node_t)); 
    }
    stack_push(ui_active_context->tags_stack_top, tags_stack_node);
    tags_stack_node->key = key;
    
    // store tag in cache
    u32 index = key.data[0] % ui_active_context->tags_hash_list_count;
    ui_tags_hash_list_t* hash_list = &ui_active_context->tags_hash_list[index];
    
    // try to find tag in cache first
    ui_tags_node_t* tag_node = nullptr;
    for (ui_tags_node_t* n = hash_list->first; n != nullptr; n = n->next) {
        if (ui_key_equals(n->key, key)) {
            tag_node = n;
            break;
        }
    }
    
    // create if we didn't find one
    if (tag_node == nullptr) {
        tag_node = (ui_tags_node_t*)arena_alloc(ui_build_arena(), sizeof(ui_tags_node_t));
        stack_push(hash_list->first, tag_node);
        tag_node->key = key;
    }
    
    ui_stack_set_next_impl(tag, str_t)
}

function str_t
ui_set_next_tagf(char* fmt, ...) {
    
    va_list args;
    va_start(args, fmt);
    str_t string = str_formatv(ui_build_arena(), fmt, args);
    va_end(args);
    
    ui_set_next_tag(string);
    
    return string;
}

// group stacks

function void 
ui_push_size(ui_size_t size_x, ui_size_t size_y) {
    ui_push_size_x(size_x);
    ui_push_size_y(size_y);
}

function void 
ui_pop_size() {
    ui_pop_size_x();
    ui_pop_size_y();
}

function void 
ui_set_next_size(ui_size_t size_x, ui_size_t size_y) {
    ui_set_next_size_x(size_x);
    ui_set_next_size_y(size_y);
}

function void 
ui_push_fixed_size(f32 x, f32 y) {
    ui_push_fixed_size_x(x);
    ui_push_fixed_size_y(y);
}

function void 
ui_pop_fixed_size() {
    ui_pop_fixed_size_x();
    ui_pop_fixed_size_y();
}

function void 
ui_set_next_fixed_size(f32 x, f32 y) {
    ui_set_next_fixed_size_x(x);
    ui_set_next_fixed_size_y(y);
}

function void 
ui_push_fixed_pos(f32 x, f32 y) {
    ui_push_fixed_pos_x(x);
    ui_push_fixed_pos_y(y);
}

function void 
ui_pop_fixed_pos() {
    ui_pop_fixed_pos_x();
    ui_pop_fixed_pos_y();
}

function void 
ui_set_next_fixed_pos(f32 x, f32 y) {
    ui_set_next_fixed_pos_x(x);
    ui_set_next_fixed_pos_y(y);
}


function void 
ui_push_rect(rect_t rect) {
    ui_push_fixed_pos_x(rect.x0);
    ui_push_fixed_pos_y(rect.y0);
    ui_push_fixed_size_x(rect.x1 - rect.x0);
    ui_push_fixed_size_y(rect.y1 - rect.y0);
}

function void
ui_pop_rect() {
    ui_pop_fixed_pos_x();
    ui_pop_fixed_pos_y();
    ui_pop_fixed_size_x();
    ui_pop_fixed_size_y();
}

function void 
ui_set_next_rect(rect_t rect) {
    ui_set_next_fixed_pos_x(rect.x0);
    ui_set_next_fixed_pos_y(rect.y0);
    ui_set_next_fixed_size_x(rect.x1 - rect.x0);
    ui_set_next_fixed_size_y(rect.y1 - rect.y0);
}


function void 
ui_push_rounding(vec4_t rounding) {
    ui_push_rounding_00(rounding.x);
    ui_push_rounding_01(rounding.y);
    ui_push_rounding_10(rounding.z);
    ui_push_rounding_11(rounding.w);
}

function void 
ui_pop_rounding() {
    ui_pop_rounding_00();
    ui_pop_rounding_01();
    ui_pop_rounding_10();
    ui_pop_rounding_11();
}

function void 
ui_set_next_rounding(vec4_t rounding) {
    ui_set_next_rounding_00(rounding.x);
    ui_set_next_rounding_01(rounding.y);
    ui_set_next_rounding_10(rounding.z);
    ui_set_next_rounding_11(rounding.w);
}

function void 
ui_push_padding(f32 padding) {
    ui_push_padding_x(padding);
    ui_push_padding_y(padding);
}

function void 
ui_pop_padding() {
    ui_pop_padding_x();
    ui_pop_padding_y();
}

function void 
ui_set_next_padding(f32 padding) {
    ui_set_next_padding_x(padding);
    ui_set_next_padding_y(padding);
}

//- renderer stacks


function void 
ui_r_auto_pop_stacks() {
#define ui_r_stack(name, type) if (ui_active_context->name##_r_stack.auto_pop) { ui_r_pop_##name(); ui_active_context->name##_r_stack.auto_pop = false; };
    ui_r_stack_list
#undef ui_r_stack
}

#define ui_r_stack_top_impl(name, type)\
return ui_active_context->name##_r_stack.top->v;

#define ui_r_stack_push_impl(name, type)\
ui_r_##name##_node_t* node = ui_active_context->name##_r_stack.free;\
if (node != nullptr) {\
stack_pop(ui_active_context->name##_r_stack.free);\
} else {\
node = (ui_r_##name##_node_t*)arena_alloc(ui_build_arena(), sizeof(ui_r_##name##_node_t));\
}\
type old_value = ui_active_context->name##_r_stack.top->v; node->v = v;\
stack_push(ui_active_context->name##_r_stack.top, node);\
ui_active_context->name##_r_stack.auto_pop = false;\
return old_value;


#define ui_r_stack_pop_impl(name, type, default)\
ui_r_##name##_node_t* popped = ui_active_context->name##_r_stack.top;\
type result = default;\
if (popped != nullptr) {\
result = popped->v;\
stack_pop(ui_active_context->name##_r_stack.top);\
stack_push(ui_active_context->name##_r_stack.free, popped);\
ui_active_context->name##_r_stack.auto_pop = false;\
}\
return result;


#define ui_r_stack_set_next_impl(name, type)\
ui_r_##name##_node_t* node = ui_active_context->name##_r_stack.free;\
if (node != nullptr) {\
stack_pop(ui_active_context->name##_r_stack.free);\
} else {\
node = (ui_r_##name##_node_t*)arena_alloc(ui_build_arena(), sizeof(ui_r_##name##_node_t));\
}\
type old_value = ui_active_context->name##_r_stack.top->v;\
node->v = v;\
stack_push(ui_active_context->name##_r_stack.top, node);\
ui_active_context->name##_r_stack.auto_pop = true;\
return old_value;

function color_t ui_r_top_color0() { ui_r_stack_top_impl(color0, color_t) }
function color_t ui_r_push_color0(color_t v) { ui_r_stack_push_impl(color0, color_t) }
function color_t ui_r_pop_color0() { ui_r_stack_pop_impl(color0, color_t, { 0 }) }
function color_t ui_r_set_next_color0(color_t v) { ui_r_stack_set_next_impl(color0, color_t) }

function color_t ui_r_top_color1() { ui_r_stack_top_impl(color1, color_t) }
function color_t ui_r_push_color1(color_t v) { ui_r_stack_push_impl(color1, color_t) }
function color_t ui_r_pop_color1() { ui_r_stack_pop_impl(color1, color_t, { 0 }) }
function color_t ui_r_set_next_color1(color_t v) { ui_r_stack_set_next_impl(color1, color_t) }

function color_t ui_r_top_color2() { ui_r_stack_top_impl(color2, color_t) }
function color_t ui_r_push_color2(color_t v) { ui_r_stack_push_impl(color2, color_t) }
function color_t ui_r_pop_color2() { ui_r_stack_pop_impl(color2, color_t, { 0 }) }
function color_t ui_r_set_next_color2(color_t v) { ui_r_stack_set_next_impl(color2, color_t) }

function color_t ui_r_top_color3() { ui_r_stack_top_impl(color3, color_t) }
function color_t ui_r_push_color3(color_t v) { ui_r_stack_push_impl(color3, color_t) }
function color_t ui_r_pop_color3() { ui_r_stack_pop_impl(color3, color_t, { 0 }) }
function color_t ui_r_set_next_color3(color_t v) { ui_r_stack_set_next_impl(color3, color_t) }

function f32 ui_r_top_radius0() { ui_r_stack_top_impl(radius0, f32) }
function f32 ui_r_push_radius0(f32 v) { ui_r_stack_push_impl(radius0, f32) }
function f32 ui_r_pop_radius0() { ui_r_stack_pop_impl(radius0, f32, 0.0f) }
function f32 ui_r_set_next_radius0(f32 v) { ui_r_stack_set_next_impl(radius0, f32) }

function f32 ui_r_top_radius1() { ui_r_stack_top_impl(radius1, f32) }
function f32 ui_r_push_radius1(f32 v) { ui_r_stack_push_impl(radius1, f32) }
function f32 ui_r_pop_radius1() { ui_r_stack_pop_impl(radius1, f32, 0.0f) }
function f32 ui_r_set_next_radius1(f32 v) { ui_r_stack_set_next_impl(radius1, f32) }

function f32 ui_r_top_radius2() { ui_r_stack_top_impl(radius2, f32) }
function f32 ui_r_push_radius2(f32 v) { ui_r_stack_push_impl(radius2, f32) }
function f32 ui_r_pop_radius2() { ui_r_stack_pop_impl(radius2, f32, 0.0f) }
function f32 ui_r_set_next_radius2(f32 v) { ui_r_stack_set_next_impl(radius2, f32) }

function f32 ui_r_top_radius3() { ui_r_stack_top_impl(radius3, f32) }
function f32 ui_r_push_radius3(f32 v) { ui_r_stack_push_impl(radius3, f32) }
function f32 ui_r_pop_radius3() { ui_r_stack_pop_impl(radius3, f32, 0.0f) }
function f32 ui_r_set_next_radius3(f32 v) { ui_r_stack_set_next_impl(radius3, f32) }

function f32 ui_r_top_thickness() { ui_r_stack_top_impl(thickness, f32) }
function f32 ui_r_push_thickness(f32 v) { ui_r_stack_push_impl(thickness, f32) }
function f32 ui_r_pop_thickness() { ui_r_stack_pop_impl(thickness, f32, 0.0f) }
function f32 ui_r_set_next_thickness(f32 v) { ui_r_stack_set_next_impl(thickness, f32) }

function f32 ui_r_top_softness() { ui_r_stack_top_impl(softness, f32) }
function f32 ui_r_push_softness(f32 v) { ui_r_stack_push_impl(softness, f32) }
function f32 ui_r_pop_softness() { ui_r_stack_pop_impl(softness, f32, 0.0f) }
function f32 ui_r_set_next_softness(f32 v) { ui_r_stack_set_next_impl(softness, f32) }

function font_handle_t ui_r_top_font() { ui_r_stack_top_impl(font, font_handle_t) }
function font_handle_t ui_r_push_font(font_handle_t v) { ui_r_stack_push_impl(font, font_handle_t) }
function font_handle_t ui_r_pop_font() { ui_r_stack_pop_impl(font, font_handle_t, { 0 }) }
function font_handle_t ui_r_set_next_font(font_handle_t v) { ui_r_stack_set_next_impl(font, font_handle_t) }

function f32 ui_r_top_font_size() { ui_r_stack_top_impl(font_size, f32) }
function f32 ui_r_push_font_size(f32 v) { ui_r_stack_push_impl(font_size, f32) }
function f32 ui_r_pop_font_size() { ui_r_stack_pop_impl(font_size, f32, 12.0f) }
function f32 ui_r_set_next_font_size(f32 v) { ui_r_stack_set_next_impl(font_size, f32) }

function rect_t ui_r_top_clip_mask() { ui_r_stack_top_impl(clip_mask, rect_t) }
function rect_t ui_r_push_clip_mask(rect_t v) { ui_r_stack_push_impl(clip_mask, rect_t) }
function rect_t ui_r_pop_clip_mask() { ui_r_stack_pop_impl(clip_mask, rect_t, { 0 }) }
function rect_t ui_r_set_next_clip_mask(rect_t v) { ui_r_stack_set_next_impl(clip_mask, rect_t) }

function gfx_handle_t ui_r_top_texture() { ui_r_stack_top_impl(texture, gfx_handle_t) }
function gfx_handle_t ui_r_push_texture(gfx_handle_t v) { ui_r_stack_push_impl(texture, gfx_handle_t) }
function gfx_handle_t ui_r_pop_texture() { ui_r_stack_pop_impl(texture, gfx_handle_t, { 0 }) }
function gfx_handle_t ui_r_set_next_texture(gfx_handle_t v) { ui_r_stack_set_next_impl(texture, gfx_handle_t) }

// group stacks

// colors
function void
ui_r_push_color(color_t color) {
	ui_r_push_color0(color);
	ui_r_push_color1(color);
	ui_r_push_color2(color);
	ui_r_push_color3(color);
}

function void
ui_r_set_next_color(color_t color) {
	ui_r_set_next_color0(color);
	ui_r_set_next_color1(color);
	ui_r_set_next_color2(color);
	ui_r_set_next_color3(color);
}

function void
ui_r_pop_color() {
	ui_r_pop_color0();
	ui_r_pop_color1();
	ui_r_pop_color2();
	ui_r_pop_color3();
}

// rounding
function vec4_t 
ui_r_top_rounding() {
	f32 x = ui_r_top_radius0();
	f32 y = ui_r_top_radius1();
	f32 z = ui_r_top_radius2();
	f32 w = ui_r_top_radius3();
	return vec4(x, y, z, w);
}

function void
ui_r_push_rounding(f32 radius) {
	ui_r_push_radius0(radius);
	ui_r_push_radius1(radius);
	ui_r_push_radius2(radius);
	ui_r_push_radius3(radius);
}

function void
ui_r_push_rounding(vec4_t radii) {
	ui_r_push_radius0(radii.x);
	ui_r_push_radius1(radii.y);
	ui_r_push_radius2(radii.z);
	ui_r_push_radius3(radii.w);
}

function void
ui_r_set_next_rounding(f32 radius) {
	ui_r_set_next_radius0(radius);
	ui_r_set_next_radius1(radius);
	ui_r_set_next_radius2(radius);
	ui_r_set_next_radius3(radius);
}

function void
ui_r_set_next_rounding(vec4_t radii) {
	ui_r_set_next_radius0(radii.x);
	ui_r_set_next_radius1(radii.y);
	ui_r_set_next_radius2(radii.z);
	ui_r_set_next_radius3(radii.w);
}

function void
ui_r_pop_rounding() {
	ui_r_pop_radius0();
	ui_r_pop_radius1();
	ui_r_pop_radius2();
	ui_r_pop_radius3();
}

#endif // SORA_UI_CORE_CPP