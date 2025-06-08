// scene.cpp

#ifndef SCENE_CPP
#define SCENE_CPP

//~ implementation 

//- scene functions

function scene_t*
scene_create() {
    
    arena_t* arena = arena_create(gigabytes(1));
    scene_t* scene = (scene_t*)arena_calloc(arena, sizeof(scene_t));
    
    scene->arena = arena;
    
    // create root node
    scene->node_root = _scene_node_alloc(scene);
    scene->node_root->id = _scene_get_id(scene);
    scene->node_root->position = vec3(0.0f, 0.0f, 0.0f);
    scene->node_root->rotation = vec3(0.0f, 0.0f, 0.0f);
    scene->node_root->scale = vec3(1.0f, 1.0f, 1.0f);
    scene->node_root->transform = mat4(1.0f);
    
    // set name
    scene->node_root->name_length = cstr_length("root");
    memcpy(scene->node_root->name, "root", sizeof(char) * scene->node_root->name_length);
    
    // ui 
    scene->ui_hierarchy = (ui_scene_hierarchy_t*)arena_calloc(arena, sizeof(ui_scene_hierarchy_t));
    scene->ui_hierarchy->arena = arena;
    
    // allocate ui hash list
    scene->ui_hierarchy->hash_list_count = 512;
    scene->ui_hierarchy->hash_list = (ui_scene_node_hash_list_t*)arena_calloc(arena, sizeof(ui_scene_node_hash_list_t) * scene->ui_hierarchy->hash_list_count);
    
    return scene;
}

function void
scene_release(scene_t* scene) {
    
    
    arena_release(scene->arena);
}

function void
scene_update(scene_t* scene) {
    
    // update the transforms
    for (scene_node_t* node = scene->node_root, *next = nullptr; node != nullptr; node = next) {
        scene_node_rec_t rec = scene_node_rec_depth_first(node, false);
        next = rec.next;
        
        // TODO: we should try and cache this 
        // and only calculate this when it
        // is changed.
        
        // calculate this nodes world transform
        mat4_t translation = mat4_translate(node->position);
        mat4_t rotation = mat4_from_quat(quat_from_euler_angle(node->rotation));
        mat4_t scale = mat4_scale(node->scale);
        node->transform = mat4_mul(translation, mat4_mul(rotation, scale));
        
        if (node->parent != nullptr) {
            node->transform = mat4_mul(node->parent->transform, node->transform);
        }
        
    }
    
    
}

//- scene node functions 


function scene_node_t*
scene_node_create(scene_t* scene, str_t name) {
    
    // get a node
    scene_node_t* node = _scene_node_alloc(scene);
    
    // fill data
    node->id = _scene_get_id(scene);
    node->name_length = name.size;
    memcpy(node->name, name.data, sizeof(char) * node->name_length);
    node->position = vec3(0.0f, 0.0f, 0.0f);
    node->rotation = vec3(0.0f, 0.0f, 0.0f);
    node->scale = vec3(1.0f, 1.0f, 1.0f);
    node->transform = mat4(1.0f);
    
    // default to root
    dll_push_back(scene->node_root->child_first, scene->node_root->child_last, node);
    node->parent = scene->node_root;
    
    return node;
}

function void
scene_node_release(scene_t* scene, scene_node_t* node) {
    
    // TODO: unhook from graph
    
    _scene_node_release(scene, node);
}

function void 
scene_node_add_child(scene_node_t* parent, scene_node_t* child) {
    
    // remove from old parent if needed
    if (child->parent != nullptr) {
        scene_node_t* child_parent = child->parent;
        dll_remove(child_parent->child_first, child_parent->child_last, child);
    }
    
    // push to new parent
    dll_push_back(parent->child_first, parent->child_last, child);
    child->parent = parent;
}

function void
scene_node_remove_child(scene_node_t* parent, scene_node_t* child) {
    dll_remove(parent->child_first, parent->child_last, child);
    child->parent = nullptr;
}

function scene_node_rec_t 
scene_node_rec_depth_first(scene_node_t* node, b8 skip_child) {
    scene_node_rec_t rec = { 0 };
    
    if (!skip_child && node->child_first != nullptr) {
        rec.next = node->child_first;
        rec.push_count = 1;
    } else for (scene_node_t* parent = node; parent != nullptr; parent = parent->parent) {
        if (parent->next != nullptr) {
            rec.next = parent->next;
            break;
        }
        rec.pop_count++;
    }
    
    return rec;
}

//- scene internal functions 

function u64
_scene_get_id(scene_t* scene) {
    return scene->id_counter++;
}

function scene_node_t*
_scene_node_alloc(scene_t* scene) {
    scene_node_t* node = scene->node_free;
    if (node != nullptr) {
        stack_pop_n(scene->node_free, global_next);
    } else {
        node = (scene_node_t*)arena_alloc(scene->arena, sizeof(scene_node_t));
    }
    memset(node, 0, sizeof(scene_node_t));
    dll_push_back_np(scene->node_first, scene->node_last, node, global_next, global_prev);
    
    return node;
}

function void 
_scene_node_release(scene_t* scene, scene_node_t* node) {
    dll_remove_np(scene->node_first, scene->node_last, node, global_next, global_prev);
    stack_push_n(scene->node_free, node, global_next);
}

//~ ui functions 

function void 
ui_scene_hierarchy(scene_t* scene) {
    
    ui_scene_hierarchy_t* list = scene->ui_hierarchy;
    
    ui_push_size(ui_size_pixels(300.0f), ui_size_pixels(25.0f));
    ui_push_tagf("list");
    
    i32 depth = 0;
    i32 index = 0;
    for (scene_node_t* node = scene->node_root->child_first; node != nullptr;) {
        ui_scene_node_t* ui_node = _ui_scene_node_find(list, node);
        str_t node_string = str(node->name, node->name_length);
        
        // container
        ui_set_next_layout_dir(ui_dir_right);
        ui_set_next_rounding(vec4(0.0f));
        if (index % 2 == 0) {
            ui_set_next_tagf("alt");
        }
        
        ui_node_flags container_flags =
            ui_flag_mouse_interactable |
            ui_flag_draw_background |
            ui_flag_draw_hover_effects |
            ui_flag_draw_active_effects |
            ui_flag_draw_custom |
            ui_flag_ignore_parent_offset_x |
            ui_flag_anim_pos_y;
        
        ui_node_t* container_node = ui_node_from_string(container_flags, node_string);
        b8* is_selected = (b8*)arena_alloc(ui_build_arena(), sizeof(b8));
        *is_selected = (scene->node_selected == node) ? true : false;
        ui_node_set_custom_draw(container_node, _ui_scene_node_draw, is_selected);
        
        ui_push_parent(container_node);
        
        // indent
        ui_spacer(ui_size_pixels(15.0f * depth));
        
        // icon
        if (node->child_first != nullptr) {
            ui_set_next_font(ui_font_icon);
            ui_set_next_size(ui_size_pixels(16.0f), ui_size_percent(1.0f));
            ui_icon icon = (ui_node->opened ? ui_icon_down : ui_icon_right);
            str_t icon_string = str_format(ui_build_arena(),"%c", icon);
            
            ui_node_t* icon_node = ui_node_from_stringf(ui_flag_mouse_interactable | ui_flag_draw_text, "%p_icon", container_node);
            ui_node_set_display_string(icon_node, icon_string);
            
            ui_interaction icon_interaction = ui_interaction_from_node(icon_node);
            
            // toggle
            if (icon_interaction & ui_left_clicked && node->child_first != nullptr) {
                ui_node->opened = !ui_node->opened;
                scene->node_selected = node;
            }
            
        } else {
            ui_spacer(ui_size_pixels(16.0f));
        }
        
        ui_set_next_font_size(11.0f);
        ui_set_next_size(ui_size_by_text(2.0f), ui_size_percent(1.0f));
        ui_set_next_tagf("icon");
        if (node->mesh != nullptr) {
            ui_set_next_font(ui_font_icon);
            ui_labelf("%c", ui_icon_cube);
        } else {
            ui_labelf("%c", ' ');
        }
        
        // label
        ui_set_next_size(ui_size_percent(1.0f), ui_size_percent(1.0f));
        ui_set_next_text_alignment(ui_text_align_left);
        ui_label(node_string);
        
        //str_t text_edit_label = str_format(ui_build_arena(), "%p_text_edit", ui_node);
        //ui_text_edit(&ui_node->cursor, &ui_node->mark, (u8*)node->name, 128, &node->name_length, str(""), text_edit_label);
        
        ui_pop_parent();
        
        ui_interaction container_interaction = ui_interaction_from_node(container_node);
        if (container_interaction & ui_left_clicked) {
            scene->node_selected = node;
        }
        
        // determine next node
        scene_node_rec_t rec = scene_node_rec_depth_first(node, !ui_node->opened);
        depth += rec.push_count;
        depth -= rec.pop_count;
        node = rec.next;
        index++;
    }
    
    ui_pop_size();
    ui_pop_tag(); // tag: "list"
    
}

function void
ui_scene_node_properties(scene_node_t* node) {
    
    ui_set_next_size(ui_size_pixels(300.0f), ui_size_pixels(500.0f));
    ui_node_t* container = ui_node_from_string(ui_flag_draw_background | ui_flag_draw_border, str(""));
    ui_push_parent(container);
    
    if (node != nullptr) {
        ui_push_size(ui_size_pixels(300.0f), ui_size_pixels(25.0f));
        
        // label
        ui_label(str(node->name, node->name_length));
        
        ui_size_t spacer_size = ui_size_pixels(5.0f);
        
        ui_vec3_editf(&node->position, 0.01f, "%p_pos", node);
        ui_spacer(spacer_size);
        ui_vec3_editf(&node->rotation, 0.01f, "%p_rot", node);
        ui_spacer(spacer_size);
        ui_vec3_editf(&node->scale, 0.01f, "%p_scale", node);
        
        ui_pop_size();
    }
    
    ui_pop_parent();
}

//- ui internal functions 

function ui_scene_node_t* 
_ui_scene_node_find(ui_scene_hierarchy_t* list, scene_node_t* node) {
    
    ui_scene_node_t* result = nullptr;
    
    // find in hash map first
    u64 index = (u64)node % list->hash_list_count;
    ui_scene_node_hash_list_t* hash_list = &list->hash_list[index];
    
    for (ui_scene_node_t* n = hash_list->first; n != nullptr; n = n->next) {
        if (n->node == node) {
            result = n;
            break;
        }
    }
    
    // create and add to map if we didn't find it
    if (result == nullptr) {
        
        result = list->node_free;
        if (result != nullptr) {
            stack_pop(list->node_free);
        } else {
            result = (ui_scene_node_t*)arena_alloc(list->arena, sizeof(ui_scene_node_t));
        }
        memset(result, 0, sizeof(ui_scene_node_t));
        dll_push_back(hash_list->first, hash_list->last, result);
        
        result->node = node;
        result->opened = true;
    }
    
    return result;
}


function void
_ui_scene_node_draw(ui_node_t* node) {
    
    b8 is_selected = *(b8*)(node->custom_draw_data);
    f32 selected_t = ui_anim(ui_key_from_stringf(node->key, "anim_selected"), 0.0f, (f32)is_selected, ui_active_context->anim_rapid_rate);
    
    if (selected_t != 0.0f) {
        
        color_t color_accent = ui_color_from_key_name(node->tags_key, str("accent"));
        color_t color_overlay = color_lerp(color(0x00000000), color(color_accent.r, color_accent.g, color_accent.b, 0.3f),  selected_t);
        color_t color_border = color_lerp(color(0x00000000), color(color_accent.r, color_accent.g, color_accent.b, 0.7f),  selected_t);
        
        ui_r_set_next_color(color_border);
        ui_r_draw_rect(node->rect, 1.0f, 0.33f, node->rounding);
        
        // draw background
        ui_r_set_next_color(color_overlay);
        ui_r_draw_rect(node->rect, 0.0f, 0.33f, node->rounding);
        
        // draw overlay
        f32 height = rect_height(node->rect);
        f32 height_top = 0.5f;
        f32 height_bot = 0.25f;
        
        rect_t top_rect = rect_cut_bottom(node->rect, roundf(height * height_top));
        rect_t bottom_rect = rect_cut_top(node->rect, roundf(height * height_bot));
        color_t color_transparent = color_overlay;
        color_transparent.a = 0.0f;
        
        ui_r_set_next_color0(color_transparent);
        ui_r_set_next_color1(color_overlay);
        ui_r_set_next_color2(color_transparent);
        ui_r_set_next_color3(color_overlay);
        ui_r_draw_rect(top_rect, 0.0f, 0.33f, vec4(node->rounding.x, 0.0f, node->rounding.z, 0.0f));
        
        ui_r_set_next_color0(color_overlay);
        ui_r_set_next_color1(color_transparent);
        ui_r_set_next_color2(color_overlay);
        ui_r_set_next_color3(color_transparent);
        ui_r_draw_rect(bottom_rect, 0.0f, 0.33f, vec4(0.0f, node->rounding.y, 0.0f, node->rounding.w));
        
    }
    
}

#endif // SCENE_CPP