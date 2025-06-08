// tileset_viewer.cpp

//~ includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "ui/sora_ui_inc.h"
#include "ui/sora_ui_inc.cpp"

#include "game/debug.h"
#include "game/debug.cpp"

#include "game/tileset.h"
#include "game/tileset.cpp"

#include "game/planet.h"
#include "game/planet.cpp"

// TODO:
// 
// [ ] - generate barycentric meshes
// [ ] - generate ruleset based on hashes.
// [ ] - export tileset and rules to file.
// [ ] - import file.
// [ ] - render whole tileset at once.
// [ ] - render example scene with rules
// [ ] - write a tile renderer.
//     [ ] - draw meshes with transforms.
//     [ ] - maybe shadows and other fancy rendering?
// [ ] - rewrite the debug renderer.
//     [ ] - draw lines.
//     [ ] - draw simple shapes.
//     [ ] - draw meshes.
//     [ ] - draw text.
//     [ ] - draw poly line.
// [ ] - ui 
//     [ ] - remove renderer stack properties (push_thickness, pop_softness, etc.)
//     [ ] - add in panels.
//     [ ] - popups.
// [ ] - 
//
//

//- emums 

enum tsv_cmd_type {
    tsv_cmd_type_null,
    tsv_cmd_type_select_tile,
    tsv_cmd_type_select_face,
};

enum tsv_mode {
    tsv_mode_none,
    tsv_mode_tile_viewer,
};

//- structs

// commands
struct tsv_cmd_t {
    tsv_cmd_t* next;
    tsv_cmd_t* prev;
    
    tsv_cmd_type type;
    union {
        i32 selected_tile;
    };
};

struct tsv_cmd_list_t {
    tsv_cmd_t* first;
    tsv_cmd_t* last;
};

struct tsv_state_t {
    
    // arenas
    arena_t* arena;
    arena_t* command_arena;
    
    // mode
    tsv_mode mode;
    
    // commands
    tsv_cmd_list_t commands;
    tsv_cmd_t* cmd_free;
    
    // tileset
    tileset_t* tileset;
    u32 current_tile_index;
    tile_t* current_tile;
    ts_debug_tile_t* current_debug_tile;
    
    i32 selected_face_index;
    
};

struct ui_graph_data_t {
    vec2_t* data;
    u32 data_count;
};

//~ globals

global os_handle_t window;
global gfx_handle_t renderer;
global gfx_handle_t render_graph;
global ui_context_t* ui_context;
global b8 quit = false;

global arena_t* arena;

// assets
global gfx_handle_t camera;
global gfx_handle_t vertex_shader;
global gfx_handle_t pixel_shader;

// tile renderer
global gfx_handle_t vertex_buffer;
global gfx_handle_t index_buffer;
global gfx_handle_t constant_buffer;
global gfx_handle_t time_constant_buffer;

global tsv_state_t tsv_state;

global planet_t* planet;


//~ functions

// tsv 

function void tsv_init();
function void tsv_release();
function void tsv_begin_frame();
function void tsv_end_frame();

function void tsv_debug_draw();
function void tsv_ui();

function tsv_cmd_t* tsv_cmd_push(tsv_cmd_type type);
function void tsv_cmd_pop(tsv_cmd_t* cmd);

// custom ui
function ui_interaction ui_edge_circle(str_t label, u64 hash);
function void ui_edge_circle_draw(ui_node_t* node);

function void ui_graph(str_t label, vec2_t* data, u32 data_count);
function void ui_graph_draw(ui_node_t* node);

// utils
function vec2_t world_to_screen(vec3_t pos, mat4_t view, mat4_t projection, vec2_t window_size);

// main pass
function void main_pass_init(gfx_handle_t render_pass);
function void main_pass_execute(gfx_handle_t render_pass);

// app
function void app_init();
function void app_release();
function void app_frame();

//~ implementation

//- tsv functions

function void 
tsv_init() {
    
    tsv_state.arena = arena_create(gigabytes(1));
    tsv_state.command_arena = arena_create(kilobytes(4));
    
    tsv_state.mode = tsv_mode_none;
    tsv_state.tileset = tileset_load(tsv_state.arena, str("res/models/tileset.obj"));
    tsv_state.selected_face_index = -1;
    
    tsv_cmd_t* cmd = tsv_cmd_push(tsv_cmd_type_select_tile);
    cmd->selected_tile = 0;
    
}

function void 
tsv_release() {
    arena_release(tsv_state.command_arena);
    arena_release(tsv_state.arena);
}

function void
tsv_begin_frame() {
    
    // process commands
    for (tsv_cmd_t* cmd = tsv_state.commands.first; cmd != nullptr; cmd = cmd->next) {
        
        b8 taken = false;
        
        switch (cmd->type) {
            case tsv_cmd_type_select_tile: {
                
                tsv_state.current_tile_index = cmd->selected_tile;
                tsv_state.current_tile = &tsv_state.tileset->tiles[tsv_state.current_tile_index];
                tsv_state.current_debug_tile = &tsv_state.tileset->debug_tiles[tsv_state.current_tile_index];
                
                taken = true;
                break;
            }
        }
        
        if (taken) {
            tsv_cmd_pop(cmd);
        }
    }
    
}

function void
tsv_end_frame() {
    
    // deselect face
    if (os_mouse_release(window, os_mouse_button_left)) {
        tsv_state.selected_face_index = -1;
    }
    
}

function void
tsv_debug_draw() {
    
    if (tsv_state.mode != tsv_mode_none) {
        
        ts_debug_tile_t* debug_tile = tsv_state.current_debug_tile;
        color_t vertex_color = color(0xe2b345ff);
        
        // draw bounding volume
        debug_draw_line(debug_tile->bound_top.p0, debug_tile->bound_top.p1, vertex_color);
        debug_draw_line(debug_tile->bound_top.p1, debug_tile->bound_top.p2, vertex_color);
        debug_draw_line(debug_tile->bound_top.p2, debug_tile->bound_top.p0, vertex_color);
        
        debug_draw_line(debug_tile->bound_bottom.p0, debug_tile->bound_bottom.p1, vertex_color);
        debug_draw_line(debug_tile->bound_bottom.p1, debug_tile->bound_bottom.p2, vertex_color);
        debug_draw_line(debug_tile->bound_bottom.p2, debug_tile->bound_bottom.p0, vertex_color);
        
        debug_draw_line(debug_tile->bound_top.p0, debug_tile->bound_bottom.p0, vertex_color);
        debug_draw_line(debug_tile->bound_top.p1, debug_tile->bound_bottom.p1, vertex_color);
        debug_draw_line(debug_tile->bound_top.p2, debug_tile->bound_bottom.p2, vertex_color);
        
        // draw plane vertices
        //gfx_vertex_t* vertices = gfx_mesh_get_vertices(tsv_state.current_tile->mesh);
        //for (i32 i = 0; i < 3; i++) {
        /*for (i32 j = 0; j < debug_info->plane_vertex_count; j++) {
            gfx_vertex_t* vertex = &vertices[debug_info->plane_vertex_indices[j]];
            debug_draw_sphere(vertex->position, 0.01f, vertex_color);
        }*/
        
        // draw selected face 
        if (tsv_state.selected_face_index != -1) {
            ts_plane_debug_info_t* debug_info = &debug_tile->plane_debug_info[tsv_state.selected_face_index];
            ts_plane_t plane = debug_info->plane;
            debug_draw_quad(plane.a, plane.b, plane.c, plane.d, color(0xe2b34560));
        }
        //}
        
    }
    
}

function void
tsv_ui() {
    
    if (tsv_state.mode != tsv_mode_none) {
        
        
        // get sizes
        uvec2_t renderer_size = gfx_renderer_get_size(renderer);
        vec2_t window_size = vec2((f32)renderer_size.x, (f32)renderer_size.y);
        vec2_t window_center = vec2_mul(window_size, 0.5f);
        
        // debug info
        ui_push_size(ui_size_pixels(150.0f), ui_size_pixels(25.0f));
        
        // statistics
        ui_labelf("tile_count: %u", tsv_state.tileset->tile_count);
        ui_labelf("current_tile: %u", tsv_state.current_tile_index);
        
        if (ui_buttonf("next_mesh") & ui_left_clicked) {
            // TODO:
        }
        
        if (ui_buttonf("prev_mesh") & ui_left_clicked) {
            // TODO: 
        }
        
        ui_pop_size();
        
        
        // tile info
        if (tsv_state.current_tile != nullptr) {
            
            ts_debug_tile_t* debug_tile = tsv_state.current_debug_tile;
            
            mat4_t view = gfx_camera_get_view(camera);
            mat4_t projection = gfx_camera_get_projection(camera);
            
            for (i32 i = 0; i < 3; i++) {
                ts_plane_debug_info_t* info = &debug_tile->plane_debug_info[i];
                
                // draw ui vertex graph
                if (tsv_state.selected_face_index == i) {
                    vec2_t* distances = debug_tile->distances[i];
                    u32 distance_count = debug_tile->distance_count[i];
                    ui_set_next_fixed_pos(window_center.x - 225.0f, window_size.y - 200.0f);
                    ui_set_next_size(ui_size_pixels(450.0f), ui_size_pixels(150.0f));
                    ui_graph(str("graph"), distances, distance_count);
                }
                
                vec2_t screen_pos = world_to_screen(info->plane_center, view, projection, vec2((f32)renderer_size.x, (f32)renderer_size.y));
                str_t text = str_format(ui_build_arena(), "%x##%i", tsv_state.current_tile->edge_hashes[i],i);
                ui_set_next_fixed_pos(screen_pos.x - 20.0f, screen_pos.y - 20.0f);
                ui_set_next_size(ui_size_pixels(40.0f), ui_size_pixels(40.0f));
                
                if (ui_edge_circle(text, tsv_state.current_tile->edge_hashes[i]) & ui_left_clicked) {
                    tsv_state.selected_face_index = i;
                }
            }
        }
        
    }
    
}

function tsv_cmd_t* 
tsv_cmd_push(tsv_cmd_type type) {
    tsv_cmd_t* cmd = tsv_state.cmd_free;
    if (cmd != nullptr) {
        stack_pop(tsv_state.cmd_free);
    } else {
        cmd = (tsv_cmd_t*)arena_alloc(tsv_state.command_arena, sizeof(tsv_cmd_t));
    }
    memset(cmd, 0, sizeof(tsv_cmd_t));
    dll_push_back(tsv_state.commands.first, tsv_state.commands.last, cmd);
    
    cmd->type = type;
    
    return cmd;
}

function void 
tsv_cmd_pop(tsv_cmd_t* cmd) {
    dll_remove(tsv_state.commands.first, tsv_state.commands.last, cmd);
    stack_push(tsv_state.cmd_free, cmd);
}

//- custom ui

function ui_interaction 
ui_edge_circle(str_t label, u64 hash) {
    ui_node_t* node = ui_node_from_string(ui_flag_draw_custom | ui_flag_mouse_interactable, label);
    u64* data = (u64*)arena_alloc(ui_build_arena(), sizeof(u64));
    *data = hash;
    ui_node_set_custom_draw(node, ui_edge_circle_draw, data);
    ui_interaction interaction = ui_interaction_from_node(node);
    
    // tooltip
    if (interaction & ui_hovered) {
        ui_tooltip_begin();
        
        ui_set_next_size(ui_size_by_text(1.0f), ui_size_by_text(1.0f));
        ui_label(label);
        
        ui_tooltip_end();
    }
    
    return interaction;
}

function void 
ui_edge_circle_draw(ui_node_t* node) {
    
    u64 hash = *(u64*)node->custom_draw_data;
    
    vec2_t center = rect_center(node->rect);
    f32 radius = rect_width(node->rect) * 0.5f;
    ui_r_push_softness(0.75f);
    
    color_t border_color_light = color(0x262728ff);
    border_color_light = color_lerp(border_color_light, color_blend(border_color_light, color(0xffffff35)), node->hover_t);
    border_color_light = color_lerp(border_color_light, color_blend(border_color_light, color(0xffffff35)), node->active_t);
    
    color_t border_color_dark = color(0xe2e4e7ff);
    border_color_dark = color_lerp(border_color_dark, color_blend(border_color_dark, color(0xffffff35)), node->hover_t);
    border_color_dark = color_lerp(border_color_dark, color_blend(border_color_dark, color(0xffffff35)), node->active_t);
    
    f32 hue = (hash % 360) / 360.0f;
    color_t hash_color = color_hsv_to_rgb(color(hue, 0.9f, 0.65f, 1.0f));
    hash_color = color_lerp(hash_color, color_blend(hash_color, color(0xffffff35)), node->hover_t);
    hash_color = color_lerp(hash_color, color_blend(hash_color, color(0xffffff35)), node->active_t);
    
    if (ui_key_equals(ui_active_context->key_focused, node->key)) {
        ui_r_set_next_color(color(0xe7e432ff));
        ui_r_draw_circle(center, radius + 1.0f, 0.0f, 360.0f);
    } else {
        ui_r_set_next_color(border_color_light);
        ui_r_draw_circle(center, radius, 0.0f, 360.0f);
        ui_r_set_next_color(border_color_dark);
        ui_r_draw_circle(center, radius - 1.0f, 0.0f, 360.0f);
    }
    
    ui_r_set_next_color(hash_color);
    ui_r_draw_circle(center, radius - 2.0f, 0.0f, 360.0f);
    
    ui_r_pop_softness();
}

function void
ui_graph(str_t label, vec2_t* data, u32 data_count) {
    
    ui_node_t* node = ui_node_from_string(ui_flag_draw_custom, label);
    
    ui_graph_data_t* graph_data = (ui_graph_data_t*)arena_alloc(ui_build_arena(), sizeof(ui_graph_data_t));
    graph_data->data = data;
    graph_data->data_count = data_count;
    ui_node_set_custom_draw(node, ui_graph_draw, graph_data);
    
}

function void
ui_graph_draw(ui_node_t* node) {
    
    ui_graph_data_t* graph_data = (ui_graph_data_t*)node->custom_draw_data;
    
    ui_r_push_softness(0.5f);
    
    // background
    ui_r_set_next_color(color(0x090909ff));
    ui_r_set_next_rounding(node->rounding);
    ui_r_draw_rect(node->rect);
    
    // border
    ui_r_set_next_color0(color(0x454545ff));
    ui_r_set_next_color1(color(0x353535ff));
    ui_r_set_next_color2(color(0x454545ff));
    ui_r_set_next_color3(color(0x353535ff));
    ui_r_set_next_rounding(node->rounding);
    ui_r_set_next_thickness(1.0f);
    ui_r_draw_rect(node->rect);
    
    f32 padding = 15.0f; 
    f32 graph_width = rect_width(node->rect) - (padding * 2);
    f32 graph_height = rect_height(node->rect) - (padding * 2);
    
    // find data max and min
    f32 min_width = f32_max;
    f32 max_width = f32_min;
    f32 min_height = f32_max;
    f32 max_height = f32_min;
    for (i32 i = 0; i < graph_data->data_count; i++) {
        if (graph_data->data[i].x > max_width) { max_width = graph_data->data[i].x; }
        if (graph_data->data[i].y > max_height) { max_height = graph_data->data[i].y; }
        if (graph_data->data[i].x < min_width) { min_width = graph_data->data[i].x; }
        if (graph_data->data[i].y < min_height) { min_height = graph_data->data[i].y; }
    }
    
    // draw points
    for (i32 i = 0; i < graph_data->data_count; i++) {
        vec2_t point = graph_data->data[i];
        
        f32 new_x = node->rect.x0 + padding + remap(point.x, min_width, max_width, 0.0f, graph_width);
        f32 new_y = node->rect.y0 + padding + remap(point.y, min_height, max_height, 0.0f, graph_height);
        
        ui_r_set_next_color(color(0xb3e856ff));
        ui_r_draw_circle(vec2(new_x, new_y), 4.0f, 0.0f, 360.0f);
    }
    
    ui_r_pop_softness();
}

function vec2_t 
world_to_screen(vec3_t pos, mat4_t view, mat4_t projection, vec2_t window_size) {
    vec2_t result = vec2(-1, -1);
    
    mat4_t clip_matrix = mat4_mul(projection, view);
    vec4_t world_pos = vec4(pos.x, pos.y, pos.z, 1.0f);
    
    vec4_t clip_space_pos = mat4_mul(clip_matrix, world_pos);
    
    if (clip_space_pos.w > 0.0f) {
        vec3_t ndc = vec3(clip_space_pos.x / clip_space_pos.w, clip_space_pos.y / clip_space_pos.w, clip_space_pos.z / clip_space_pos.w);
        
        f32 pos_x = (ndc.x * 0.5f + 0.5f) * window_size.x;
        f32 pos_y = (1.0f - (ndc.y * 0.5f + 0.5f)) * window_size.y;
        result = vec2(pos_x, pos_y);
    }
    
    return result;
}


//- main pass functions  

function void 
main_pass_init(gfx_handle_t render_pass) {
    
    // create render target
    uvec2_t renderer_size = gfx_renderer_get_size(renderer);
    
    gfx_render_target_desc_t desc = { 0 };
    desc.size = renderer_size;
    desc.sample_count = 8;
    desc.colorbuffer_format = gfx_texture_format_rgba8;
    desc.depthbuffer_format = gfx_texture_format_d32;
    gfx_handle_t ms_render_target = gfx_render_target_create_ex(desc);
    gfx_render_pass_set_output(render_pass, str("output_ms"), ms_render_target);
    
    gfx_handle_t render_target = gfx_render_target_create(renderer_size, gfx_texture_format_rgba8);
    gfx_render_pass_set_output(render_pass, str("output"), render_target);
    
}

function void 
main_pass_execute(gfx_handle_t render_pass) {
    
    // get render targets
    gfx_handle_t ms_render_target = gfx_render_pass_get_output(render_pass, str("output_ms"));
    gfx_handle_t render_target = gfx_render_pass_get_output(render_pass, str("output"));
    
    uvec2_t renderer_size = gfx_renderer_get_size(renderer);
    uvec2_t render_target_size = gfx_render_target_get_size(render_target);
    rect_t viewport = rect(0.0f, 0.0f, (f32)renderer_size.x, (f32)renderer_size.y);
    
    // resize render target
    if (!uvec2_equals(renderer_size, render_target_size)) {
        gfx_render_target_resize(render_target, renderer_size);
        gfx_render_target_resize(ms_render_target, renderer_size);
    }
    
    gfx_render_target_clear(ms_render_target, color(0x121415ff), 1.0f);
    gfx_set_render_target(ms_render_target);
    
    // draw mesh
    gfx_set_viewport(viewport);
    gfx_set_scissor(viewport);
    gfx_set_rasterizer(gfx_fill_solid, gfx_cull_back);
    gfx_set_topology(gfx_topology_tris);
    gfx_set_sampler(gfx_filter_linear, gfx_wrap_repeat, 0);
    gfx_set_depth_mode(gfx_depth);
    gfx_set_shader(vertex_shader);
    gfx_set_shader(pixel_shader);
    gfx_set_buffer(vertex_buffer, 0, sizeof(gfx_vertex_t));
    gfx_set_buffer(index_buffer, 0);
    gfx_set_buffer(constant_buffer, 0);
    //gfx_draw_indexed(gfx_mesh_get_index_count(tsv_state.current_tile->mesh));
    
    // debug
    {
        
        debug_draw_line(vec3(-10.0f, 0.0f, 0.0f), vec3(10.0f, 0.0f, 0.0f), color(0xe2131490));
        debug_draw_line(vec3(0.0f, -10.0f, 0.0f), vec3(0.0f, 10.0f, 0.0f), color(0x13e21490));
        debug_draw_line(vec3(0.0f, 0.0f, -10.0f), vec3(0.0f, 0.0f, 10.0f), color(0x1314e290));
        
        // tsv debug
        tsv_debug_draw();
        
        // draw triangulation
        for (i32 i = 0; i < planet->triangle_count; i++) {
            
            p_tri_t* tri = &planet->triangles[i];
            
            vec3_t p0 = planet->points[tri->vertices[0]];
            vec3_t p1 = planet->points[tri->vertices[1]];
            vec3_t p2 = planet->points[tri->vertices[2]];
            
            u32 hash = i * 3216491246;
            f32 hue = (hash % 360) / 360.0f;
            color_t col = color_hsv_to_rgb(color(hue, 0.6f, 0.4f, 1.0f));
            debug_draw_tri(p0, p1, p2, col);
        }
        
        // flush batches
        debug_render();
    }
    
    
    gfx_handle_t color_texture = gfx_render_target_get_texture(render_target);
    gfx_handle_t ms_color_texture = gfx_render_target_get_texture(ms_render_target);
    gfx_blit(color_texture, ms_color_texture);
    
    // draw ui
    gfx_set_render_target(render_target);
    ui_begin(ui_context);
    
    // sizes
    vec2_t window_size = vec2((f32)renderer_size.x, (f32)renderer_size.y);
    vec2_t window_center = vec2_mul(window_size, 0.5f);
    
    // menu bar
    ui_set_next_size(ui_size_percent(1.0f), ui_size_pixels(30.0f));
    ui_set_next_layout_dir(ui_dir_right);
    ui_node_t* menu_bar = ui_node_from_key(ui_flag_draw_background | ui_flag_draw_border, { 0 });
    
    ui_push_size(ui_size_pixels(75.0f), ui_size_percent(1.0f));
    ui_push_parent(menu_bar);
    
    ui_buttonf("File");
    ui_buttonf("Edit");
    ui_buttonf("View");
    
    ui_pop_parent();
    ui_pop_size();
    
    
    ui_push_size(ui_size_pixels(150.0f), ui_size_pixels(25.0f));
    
    ui_pop_size();
    
    // tsv ui
    tsv_ui();
    
    ui_end(ui_context);
    
    gfx_renderer_blit(renderer, color_texture);
    gfx_renderer_present(renderer);
    
}

//- app functions 

function void 
app_init() {
    
    arena = arena_create(gigabytes(2));
    
    // create window
    window = os_window_open(str("tileset viewer"), 1280, 720, os_window_flag_maximize);
    os_window_set_frame_function(window, app_frame);
    
    // create renderer
    renderer = gfx_renderer_create(window);
    
    // create render graph
    // TODO: simplify
    render_graph = gfx_render_graph_create(renderer);
    gfx_handle_t main_pass = gfx_render_pass_create(render_graph, main_pass_init, main_pass_execute);
    gfx_render_pass_add_output(main_pass, str("output"));
    gfx_render_pass_add_output(main_pass, str("output_ms"));
    gfx_render_graph_compile(render_graph);
    
    // create ui
    ui_context = ui_context_create(window, renderer);
    
    // create camera
    camera = gfx_camera_create(arena, gfx_camera_mode_free, 80.0f, 0.01f, 100.0f);
    
    // load shaders
    // TODO: remove
    vertex_shader = gfx_shader_load(str("res/shaders/shader_3d.hlsl"), gfx_shader_flag_vertex);
    pixel_shader = gfx_shader_load(str("res/shaders/shader_3d.hlsl"), gfx_shader_flag_pixel);
    
    // renderer stuff
    // TODO: remove
    vertex_buffer = gfx_buffer_create(gfx_buffer_type_vertex, megabytes(64));
    index_buffer = gfx_buffer_create(gfx_buffer_type_index, megabytes(64));
    constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, kilobytes(64));
    
    // prepare for renderering
    debug_set_camera(camera);
    
    // create planet
    planet = planet_create(str("res/models/planet_small.obj"));
    
}

function void 
app_release() {
    ui_context_release(ui_context);
    gfx_render_graph_release(render_graph);
    gfx_renderer_release(renderer);
    os_window_close(window);
    arena_release(arena);
}


function void
app_frame() {
    
    // events
    os_get_events();
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    // update window and renderer
    os_window_update(window);
    gfx_renderer_update(renderer);
    
    // get common stats
    f32 dt = os_window_get_delta_time(window);
    uvec2_t renderer_size = gfx_renderer_get_size(renderer);
    rect_t viewport = rect(0.0f, 0.0f, (f32)renderer_size.x, (f32)renderer_size.y);
    
    // update tsv state
    tsv_begin_frame();
    
    // update camera
    // TODO: remove constant buffer
    gfx_camera_free_mode_input(camera, window);
    gfx_camera_update(camera, viewport, dt);
    gfx_camera_constants_t* constants = gfx_camera_get_constants(camera);
    gfx_buffer_fill(constant_buffer, constants, sizeof(gfx_camera_constants_t));
    
    // execute render graph
    gfx_render_graph_execute(render_graph);
    
    
    tsv_end_frame();
}

//- entry point 

function i32 
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    gfx_init();
    font_init();
    ui_init();
    tsv_init();
    debug_init();
    
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    debug_release();
    tsv_release();
    ui_release();
    font_release();
    gfx_release();
    os_release();
    
    return 0;
}