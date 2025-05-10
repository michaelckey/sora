// renderer_test.cpp

//~ includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "ui/sora_ui_inc.h"
#include "ui/sora_ui_inc.cpp"

//- structs

// debug drawing

struct debug_vertex_t {
    vec3_t position;
    color_t color;
};

struct debug_batch_t {
    debug_batch_t* next;
    debug_batch_t* prev;
    
    debug_vertex_t* vertices;
    u32 vertex_count;
};

struct debug_state_t {
    
    arena_t* arena;
    
    gfx_handle_t vertex_shader;
    gfx_handle_t pixel_shader;
    
    gfx_handle_t vertex_buffer;
    
    debug_batch_t* batch_first;
    debug_batch_t* batch_last;
    debug_batch_t* batch_free;
    
};


// mesh analyzer

struct index_dist_pair_t {
    i32 index;
    f32 distance;
};

struct plane_t {
    vec3_t origin;
    vec3_t u;
    vec3_t v;
    vec3_t normal;
    
    // original points
    vec3_t a;
    vec3_t b;
    vec3_t c;
};

//~ globals

global arena_t* arena;
global os_handle_t window;
global gfx_handle_t renderer;
global ui_context_t* ui_context;
global b8 quit = false;

global gfx_handle_t render_graph;

global gfx_handle_t vertex_shader;
global gfx_handle_t pixel_shader;
global gfx_handle_t compute_shader;

global gfx_handle_t vertex_buffer;
global gfx_handle_t index_buffer;
global gfx_handle_t constant_buffer;
global gfx_handle_t time_constant_buffer;

global gfx_handle_t camera;
global gfx_handle_t mesh;

global debug_state_t debug_state;


global vec3_t positions[3];


//~ functions

// debug draw

function void debug_init();
function void debug_release();
function void debug_render(gfx_handle_t camera_constants);

function debug_batch_t* debug_get_batch(u32 count);

function void debug_draw_line(vec3_t p0, vec3_t p1, color_t color);
function void debug_draw_cube(vec3_t center, vec3_t size, color_t color);
function void debug_draw_sphere(vec3_t c, f32 r, color_t color);

// analyze mesh

function plane_t plane_from_triangle(vec3_t a, vec3_t b, vec3_t c);
function vec2_t plane_project(plane_t plane, vec3_t point);
function f32 quantize(f32 value, f32 epsilon);
function vec3_t quantize_vec2(vec3_t v, f32 epsilon);
function u64 hash_vec2(vec2_t v);
function i32 compare_vec2(const void* a, const void* b);
function i32 compare_dist(const void* a, const void* b);
function void find_vertices_on_plane(gfx_vertex_t* vertices, u32 vertex_count, plane_t plane, i32* out_indices, u32* out_count, u32 max_count);


// main pass
function void main_pass_init(gfx_handle_t render_pass);
function void main_pass_execute(gfx_handle_t render_pass);

// app
function void app_init();
function void app_release();
function void app_frame();

//~ implementation

//- debug draw functions 

function void 
debug_init() {
    
    debug_state.arena = arena_create(gigabytes(1));
    
    debug_state.vertex_shader = gfx_shader_load(str("res/shaders/shader_debug.hlsl"), gfx_shader_flag_vertex);
    debug_state.pixel_shader = gfx_shader_load(str("res/shaders/shader_debug.hlsl"), gfx_shader_flag_pixel);
    
    debug_state.vertex_buffer = gfx_buffer_create(gfx_buffer_type_vertex, megabytes(64));
    
    debug_state.batch_first = nullptr;
    debug_state.batch_last = nullptr;
    debug_state.batch_free = nullptr;
    
}

function void 
debug_release() {
    
    gfx_buffer_release(debug_state.vertex_buffer);
    gfx_shader_release(debug_state.vertex_shader);
    gfx_shader_release(debug_state.pixel_shader);
    
    arena_release(debug_state.arena);
}

function void 
debug_render(gfx_handle_t camera_constants) {
    
    gfx_set_rasterizer(gfx_fill_solid, gfx_cull_back);
    gfx_set_topology(gfx_topology_lines);
    gfx_set_depth_mode(gfx_depth);
    
    gfx_set_shader(debug_state.vertex_shader);
    gfx_set_shader(debug_state.pixel_shader);
    
    for (debug_batch_t* batch = debug_state.batch_first; batch != nullptr; batch = batch->next) {
        gfx_buffer_fill(debug_state.vertex_buffer, batch->vertices, sizeof(debug_vertex_t) * batch->vertex_count);
        gfx_set_buffer(debug_state.vertex_buffer, 0, sizeof(debug_vertex_t));
        gfx_set_buffer(camera_constants, 0);
        gfx_draw(batch->vertex_count);
    }
    
    // clear the state
    //arena_clear(debug_state.arena);
    //debug_state.batch_first = nullptr;
    //debug_state.batch_last = nullptr;
    //debug_state.batch_free = nullptr;
    
}

function debug_batch_t* 
debug_get_batch(u32 count) {
    
    debug_batch_t* batch = nullptr;
    for (debug_batch_t* b = debug_state.batch_first; b != nullptr; b = b->next) {
        if (((b->vertex_count + count) * sizeof(debug_vertex_t)) < megabytes(64)) {
            batch = b;
            break;
        }
    }
    
    if (batch == nullptr) {
        
        batch = debug_state.batch_free;
        if (batch != nullptr) {
            stack_pop(debug_state.batch_free);
        } else {
            batch = (debug_batch_t*)arena_alloc(debug_state.arena, sizeof(debug_batch_t));
            batch->vertices = (debug_vertex_t*)arena_alloc(debug_state.arena, megabytes(64));
        }
        batch->next = nullptr;
        batch->prev = nullptr;
        batch->vertex_count = 0;
        dll_push_back(debug_state.batch_first, debug_state.batch_last, batch);
        
    }
    
    return batch;
}


function void 
debug_draw_line(vec3_t p0, vec3_t p1, color_t color) {
    debug_batch_t* batch = debug_get_batch(2);
    
    batch->vertices[batch->vertex_count++] = { p0, color };
    batch->vertices[batch->vertex_count++] = { p1, color };
}

function void
debug_draw_cube(vec3_t center, vec3_t size, color_t color) {
    
    vec3_t half = {
        size.x * 0.5f,
        size.y * 0.5f,
        size.z * 0.5f
    };
    
    vec3_t p0 = { center.x - half.x, center.y - half.y, center.z - half.z };
    vec3_t p1 = { center.x + half.x, center.y - half.y, center.z - half.z };
    vec3_t p2 = { center.x + half.x, center.y + half.y, center.z - half.z };
    vec3_t p3 = { center.x - half.x, center.y + half.y, center.z - half.z };
    vec3_t p4 = { center.x - half.x, center.y - half.y, center.z + half.z };
    vec3_t p5 = { center.x + half.x, center.y - half.y, center.z + half.z };
    vec3_t p6 = { center.x + half.x, center.y + half.y, center.z + half.z };
    vec3_t p7 = { center.x - half.x, center.y + half.y, center.z + half.z };
    
    debug_draw_line(p0, p1, color);
    debug_draw_line(p1, p5, color);
    debug_draw_line(p5, p4, color);
    debug_draw_line(p4, p0, color);
    
    debug_draw_line(p3, p2, color);
    debug_draw_line(p2, p6, color);
    debug_draw_line(p6, p7, color);
    debug_draw_line(p7, p3, color);
    
    debug_draw_line(p0, p3, color);
    debug_draw_line(p1, p2, color);
    debug_draw_line(p5, p6, color);
    debug_draw_line(p4, p7, color);
    
}

function void
debug_draw_sphere(vec3_t center, f32 radius, color_t color) {
    
    const i32 segments = 24;
    
    f32 sin_table[segments];
    f32 cos_table[segments];
    for (i32 i = 0; i < segments; i++) {
        f32 angle = (2.0f * f32_pi * i) / segments;
        sin_table[i] = sinf(angle);
        cos_table[i] = cosf(angle);
    }
    
    for (i32 i = 0; i < segments; i++) {
        i32 next = (i + 1) % segments;
        vec3_t p0 = {
            center.x + radius * cos_table[i],
            center.y + radius * sin_table[i],
            center.z
        };
        vec3_t p1 = {
            center.x + radius * cos_table[next],
            center.y + radius * sin_table[next],
            center.z
        };
        debug_draw_line(p0, p1, color);
    }
    
    for (i32 i = 0; i < segments; i++) {
        i32 next = (i + 1) % segments;
        vec3_t p0 = {
            center.x + radius * cos_table[i],
            center.y,
            center.z + radius * sin_table[i]
        };
        vec3_t p1 = {
            center.x + radius * cos_table[next],
            center.y,
            center.z + radius * sin_table[next]
        };
        debug_draw_line(p0, p1, color);
    }
    
    for (i32 i = 0; i < segments; i++) {
        i32 next = (i + 1) % segments;
        vec3_t p0 = {
            center.x,
            center.y + radius * cos_table[i],
            center.z + radius * sin_table[i]
        };
        vec3_t p1 = {
            center.x,
            center.y + radius * cos_table[next],
            center.z + radius * sin_table[next]
        };
        debug_draw_line(p0, p1, color);
    }
}

//- analyze mesh functions 

function plane_t 
plane_from_triangle(vec3_t a, vec3_t b, vec3_t c) {
    plane_t p;
    p.origin = a;
    p.u = vec3_normalize(vec3_sub(b, a));
    vec3_t ab = p.u;
    vec3_t ac = vec3_sub(c, a);
    p.normal = vec3_normalize(vec3_cross(ab, ac));
    p.v = vec3_cross(p.normal, p.u);
    p.a = a;
    p.b = b;
    p.c = c;
    return p;
}

function vec2_t 
plane_project(plane_t plane, vec3_t point) {
    vec3_t local = vec3_sub(point, plane.origin);
    vec2_t result;
    result.x = vec3_dot(local, plane.u);
    result.y = vec3_dot(local, plane.v);
    return result;
}

function f32 
quantize(f32 value, f32 epsilon) {
    return floorf(value / epsilon + 0.5f) * epsilon;
}

function vec2_t 
quantize_vec2(vec2_t v, f32 epsilon) {
    vec2_t q;
    q.x = quantize(v.x, epsilon);
    q.y = quantize(v.y, epsilon);
    return q;
}

function u64 
hash_vec2(vec2_t v) {
    u64 hash = 14695981039346656037ULL;
    u32* parts = (u32*)&v;
    for (int i = 0; i < 2; i++) {
        hash ^= parts[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

function i32 
compare_vec2(const void* a, const void* b) {
    vec2_t* va = (vec2_t*)a;
    vec2_t* vb = (vec2_t*)b;
    if (va->x != vb->x) return (va->x < vb->x) ? -1 : 1;
    if (va->y != vb->y) return (va->y < vb->y) ? -1 : 1;
    return 0;
}

function i32 
compare_dist(const void* a, const void* b) {
    index_dist_pair_t *pair_a = (index_dist_pair_t*)a;
    index_dist_pair_t *pair_b = (index_dist_pair_t*)b;
    
    if (pair_a->distance > pair_b->distance) return -1;
    if (pair_a->distance < pair_b->distance) return 1;
    return 0;
}

function void 
find_vertices_on_plane(gfx_vertex_t* vertices, u32 vertex_count, plane_t plane, i32* out_indices, u32* out_count, u32 max_count) {
    u32 count = 0;
    
    for (u32 i = 0; i < vertex_count; i++) {
        vec3_t v = vertices[i].position;
        vec3_t to_point = vec3_sub(v, plane.origin);
        f32 dist = vec3_dot(to_point, plane.normal);
        
        if (fabsf(dist) < 0.01f) {
            if (count < max_count) {
                out_indices[count++] = i;
            }
        }
    }
    
    *out_count = count;
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
    gfx_draw_indexed(gfx_mesh_get_index_count(mesh));
    
    // draw debug mesh
    debug_render(constant_buffer);
    
    gfx_handle_t color_texture = gfx_render_target_get_texture(render_target);
    gfx_handle_t ms_color_texture = gfx_render_target_get_texture(ms_render_target);
    gfx_blit(color_texture, ms_color_texture);
    
    // draw ui
    gfx_set_render_target(render_target);
    ui_begin(ui_context);
    ui_push_font_size(12.0f);
    
    mat4_t view = gfx_camera_get_view(camera);
    mat4_t projection = gfx_camera_get_projection(camera);
    
    for (i32 i = 0; i < 3; i++) {
        vec2_t screen_pos = world_to_screen(positions[i], view, projection, vec2((f32)renderer_size.x, (f32)renderer_size.y) );
        ui_set_next_fixed_pos(screen_pos.x - 15.0f, screen_pos.y - 15.0f);
        ui_set_next_size(ui_size_by_text(2.0f), ui_size_by_text(2.0f));
        ui_buttonf("edge_%i", i);
    }
    
    ui_pop_font_size();
    ui_end(ui_context);
    
    // run compute shader on texture
    //gfx_set_shader(compute_shader);
    //gfx_set_texture(color_texture, 0);
    //gfx_set_buffer(time_constant_buffer, 0);
    //gfx_dispatch((renderer_size.x + 31) / 32, (renderer_size.y + 31) / 32, 1);
    
    
    gfx_renderer_blit(renderer, color_texture);
    gfx_renderer_present(renderer);
    
}

//- app functions 

function void 
app_init() {
    
    arena = arena_create(gigabytes(2));
    window = os_window_open(str("gfx test"), 1280, 720, os_window_flag_maximize);
    renderer = gfx_renderer_create(window);
    ui_context = ui_context_create(window, renderer);
    
    os_window_set_frame_function(window, app_frame);
    
    debug_init();
    
    // load assets
    vertex_shader = gfx_shader_load(str("res/shaders/shader_3d.hlsl"), gfx_shader_flag_vertex);
    pixel_shader = gfx_shader_load(str("res/shaders/shader_3d.hlsl"), gfx_shader_flag_pixel);
    compute_shader = gfx_shader_load(str("res/shaders/shader_compute.hlsl"), gfx_shader_flag_compute);
    
    mesh = gfx_mesh_load(arena, str("res/models/tile_2.obj"));
    camera = gfx_camera_create(arena, gfx_camera_mode_free, 80.0f, 0.01f, 100.0f);
    
    vertex_buffer = gfx_buffer_create(gfx_buffer_type_vertex, megabytes(64));
    index_buffer = gfx_buffer_create(gfx_buffer_type_index, megabytes(64));
    constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, kilobytes(64));
    time_constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, kilobytes(1));
    
    // fill buffers
    gfx_buffer_fill(vertex_buffer, gfx_mesh_get_vertices(mesh), sizeof(gfx_vertex_t) * gfx_mesh_get_vertex_count(mesh));
    gfx_buffer_fill(index_buffer, gfx_mesh_get_indices(mesh), sizeof(i32) * gfx_mesh_get_index_count(mesh));
    
    // render graph
    render_graph = gfx_render_graph_create(renderer);
    gfx_handle_t main_pass = gfx_render_pass_create(render_graph, main_pass_init, main_pass_execute);
    gfx_render_pass_add_output(main_pass, str("output"));
    gfx_render_pass_add_output(main_pass, str("output_ms"));
    gfx_render_graph_compile(render_graph);
    
    // build debug mesh
    {
        temp_t scratch = scratch_begin();
        gfx_vertex_t* vertices = gfx_mesh_get_vertices(mesh);
        u32 vertex_count = gfx_mesh_get_vertex_count(mesh);
        
        index_dist_pair_t* temp_indices = (index_dist_pair_t*)arena_alloc(scratch.arena, sizeof(index_dist_pair_t) * vertex_count);
        
        f32 high = 0.0f;
        f32 low = 100.0f;
        for (u32 i = 0; i < vertex_count; i++) {
            gfx_vertex_t* vertex = &vertices[i];
            
            // find high and low points
            if (vertex->position.y > high) { high = vertex->position.y; }
            if (vertex->position.y < low) { low = vertex->position.y; }
            
            // add index to list
            temp_indices[i].index = i;
            temp_indices[i].distance = vec3_length(vertex->position);
        }
        
        // sort indices
        qsort(temp_indices, vertex_count, sizeof(index_dist_pair_t), compare_dist);
        
        // get top 3 indices
        i32 i0 = temp_indices[0].index;
        i32 i1 = temp_indices[1].index;
        i32 i2 = temp_indices[2].index;
        
        
        vec3_t l0 = vec3(vertices[i0].position.x, low, vertices[i0].position.z);
        vec3_t h0 = vec3(vertices[i0].position.x, high, vertices[i0].position.z);
        
        vec3_t l1 = vec3(vertices[i1].position.x, low, vertices[i1].position.z);
        vec3_t h1 = vec3(vertices[i1].position.x, high, vertices[i1].position.z);
        
        vec3_t l2 = vec3(vertices[i2].position.x, low, vertices[i2].position.z);
        vec3_t h2 = vec3(vertices[i2].position.x, high, vertices[i2].position.z);
        
        // build debug mesh
        debug_draw_line(l0, h0, color(0xf27665ff));
        debug_draw_line(l1, h1, color(0xf27665ff));
        debug_draw_line(l2, h2, color(0xf27665ff));
        
        debug_draw_line(h0, h1, color(0xf27665ff));
        debug_draw_line(h1, h2, color(0xf27665ff));
        debug_draw_line(h2, h0, color(0xf27665ff));
        
        debug_draw_line(l0, l1, color(0xf27665ff));
        debug_draw_line(l1, l2, color(0xf27665ff));
        debug_draw_line(l2, l0, color(0xf27665ff));
        
        // find vertices on plane
        
        plane_t planes[3] = {
            plane_from_triangle(l0, l1, h0),
            plane_from_triangle(l1, l2, h1),
            plane_from_triangle(l2, l0, h2),
        };
        
        color_t colors[3] = {
            color(0xe2d956ff),
            color(0xe562d9ff),
            color(0xed9562ff),
        };
        
        for (i32 i = 0; i < 3; i++) {
            plane_t plane = planes[i];
            
            i32* plane_indices = (i32*)arena_alloc(scratch.arena, sizeof(i32) * 32);
            vec2_t* projected_positions = (vec2_t*)arena_alloc(scratch.arena, sizeof(vec2_t) * 32);
            u32 plane_index_count = 0;
            
            find_vertices_on_plane(vertices, vertex_count, plane, plane_indices, &plane_index_count, 32);
            for (i32 j = 0; j < plane_index_count; j++) {
                gfx_vertex_t* vertex = &vertices[plane_indices[j]];
                vec3_t position = vertex->position;
                vec2_t projected_position = quantize_vec2(plane_project(plane, position), 0.01f);
                debug_draw_sphere(vertex->position, 0.015f, colors[i]);
                projected_positions[j] = projected_position;
            }
            
            // sort and hash projected positions
            qsort(projected_positions, plane_index_count, sizeof(vec2_t), compare_vec2);
            
            u64 hash = 14695981039346656037ULL;
            for (u32 j = 0; j < plane_index_count; j++) {
                hash ^= hash_vec2(projected_positions[j]);
                hash *= 1099511628211ULL;
            }
            
            printf("plane_hash_%i: %u\n", i, hash);
            
            // draw hash color
            color_t hash_color = color_hsv_to_rgb(color((hash % 360) / 360.0f, 0.7f, 0.8f, 1.0f));
            
            vec3_t v1 = vec3_sub(plane.b, plane.a);
            vec3_t v2 = vec3_sub(plane.c, plane.a);
            vec3_t plane_center = vec3_add(plane.a, vec3_mul(vec3_add(v1, v2), 0.5f));
            vec3_t offset_pos = vec3_add(plane_center, vec3_mul(plane.normal, 0.15f));
            
            positions[i] = offset_pos;
            
            //debug_draw_line(plane_center, offset_pos, hash_color);
            //debug_draw_sphere(offset_pos, 0.1f, hash_color);
            
        }
        
        scratch_end(scratch);
    }
    
    //gfx_buffer_fill(debug_vertex_buffer, debug_mesh.vertices, sizeof(debug_vertex_t) * debug_mesh.vertex_count);
    
    
}

function void 
app_release() {
    
    debug_release();
    
    gfx_render_graph_release(render_graph);
    
    ui_context_release(ui_context);
    gfx_renderer_release(renderer);
    os_window_close(window);
    arena_release(arena);
}


function void
app_frame() {
    os_get_events();
    
    // update 
    os_window_update(window);
    gfx_renderer_update(renderer);
    
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    uvec2_t renderer_size = gfx_renderer_get_size(renderer);
    
    // update camera
    f32 dt = os_window_get_delta_time(window);
    rect_t viewport = rect(0.0f, 0.0f, (f32)renderer_size.x, (f32)renderer_size.y);
    gfx_camera_free_mode_input(camera, window);
    gfx_camera_update(camera, viewport, dt);
    gfx_camera_constants_t* constants = gfx_camera_get_constants(camera);
    gfx_buffer_fill(constant_buffer, constants, sizeof(gfx_camera_constants_t));
    
    // update time
    f32 elapsed_time = os_window_get_elapsed_time(window);
    gfx_buffer_fill(time_constant_buffer, &elapsed_time, sizeof(f32));
    
    
    gfx_render_graph_execute(render_graph);
    
}

//- entry point 

function i32 
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    gfx_init();
    font_init();
    ui_init();
    
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    ui_release();
    font_release();
    gfx_release();
    os_release();
    
	return 0;
}