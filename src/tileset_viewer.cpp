// tileset_viewer.cpp

//- includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "render/sora_render_inc.h"
#include "render/sora_render_inc.cpp"

//- structs 

struct index_dist_pair_t {
    i32 index;
    f32 distance;
};

//- globals

global arena_t* arena;
global os_handle_t window;
global gfx_handle_t graphics_context;
global b8 quit = false;
global uvec2_t context_size;

global render_state_t* render_state;
global render_graph_t* render_graph;

global gfx_handle_t shader;
global gfx_handle_t debug_shader;
global gfx_handle_t texture;
global mesh_t* mesh;
global camera_t* camera;

global vec3_t debug_vertices[64];
global u32 debug_vertex_count;

//- functions

// app
function void app_init();
function void app_release();
function void app_frame();

function void main_pass_setup(render_pass_t* pass);
function void main_pass_execute(render_pass_t* pass);

function i32 compare_dist(const void* a, const void* b);

//- implementation

function void 
app_init() {
    
    arena = arena_create(gigabytes(2));
    window = os_window_open(str("tileset viewer"), 1280, 720);
    graphics_context = gfx_context_create(window, color(0x111317ff));
    render_state = render_create();
    context_size = gfx_context_get_size(graphics_context);
    
    // set window frame function
    os_window_set_frame_function(window, app_frame);
    
    // assets
    
    // load shaders
    { // 3d shader
        gfx_shader_attribute_t attributes[] = {
            { "POSITION", 0, gfx_vertex_format_float3, gfx_vertex_class_per_vertex },
            { "NORMAL", 0, gfx_vertex_format_float3, gfx_vertex_class_per_vertex },
            { "TANGENT", 0, gfx_vertex_format_float3, gfx_vertex_class_per_vertex },
            { "BITANGENT", 0, gfx_vertex_format_float3, gfx_vertex_class_per_vertex },
            { "TEXCOORD", 0, gfx_vertex_format_float2, gfx_vertex_class_per_vertex },
            { "COLOR", 0, gfx_vertex_format_float4, gfx_vertex_class_per_vertex },
        };
        
        shader = gfx_shader_load(str("res/shaders/shader_3d.hlsl"), attributes, array_count(attributes));
    }
    
    { // debug shader
        
        gfx_shader_attribute_t attributes[] = {
            { "POSITION", 0, gfx_vertex_format_float3, gfx_vertex_class_per_vertex },
            { "COLOR", 0, gfx_vertex_format_float4, gfx_vertex_class_per_vertex },
        };
        
        debug_shader = gfx_shader_load(str("res/shaders/shader_debug.hlsl"), attributes, array_count(attributes));
    }
    
    { // load textures 
        texture = gfx_texture_load(str("res/textures/matcap_4.png"));
    }
    
    { // load mesh
        mesh = mesh_load(arena, str("res/models/tile.obj"));
    }
    
    { // create camera
        camera = camera_create(arena, 85.0f, 0.01f, 100.0f);
    }
    
    // render graph
    render_graph = render_graph_create();
    render_pass_t* main_pass = render_pass_create(render_graph, str("main pass"), main_pass_setup, main_pass_execute);
    render_graph_setup(render_graph);
    
    
    // mesh analysis
    {
        
        temp_t scratch = scratch_begin();
        
        index_dist_pair_t* temp_indices = (index_dist_pair_t*)arena_alloc(scratch.arena, sizeof(index_dist_pair_t) * mesh->vertex_count);
        
        f32 high = 0.0f;
        f32 low = 100.0f;
        for (i32 i = 0; i < mesh->vertex_count; i++) {
            mesh_vertex_t* vertex = &mesh->vertices[i];
            
            // find high and low points
            if (vertex->position.y > high) { high = vertex->position.y; }
            if (vertex->position.y < low) { low = vertex->position.y; }
            
            // add index to list
            temp_indices[i].index = i;
            temp_indices[i].distance = vec3_length(vertex->position);
        }
        
        // sort indices
        qsort(temp_indices, mesh->vertex_count, sizeof(index_dist_pair_t), compare_dist);
        
        // get top 3 indices
        i32 i0 = temp_indices[0].index;
        i32 i1 = temp_indices[1].index;
        i32 i2 = temp_indices[2].index;
        
        // build debug mesh
        debug_vertices[0] = vec3(mesh->vertices[i0].position.x, low, mesh->vertices[i0].position.z);
        debug_vertices[1] = vec3(mesh->vertices[i0].position.x, high, mesh->vertices[i0].position.z);
        
        //debug_vertices[2] = vec3(mesh->vertices[i1].position.x, low, mesh->vertices[i1].position.z);
        //debug_vertices[3] = vec3(mesh->vertices[i1].position.x, high, mesh->vertices[i1].position.z);
        
        //debug_vertices[4] = vec3(mesh->vertices[i2].position.x, low, mesh->vertices[i2].position.z);
        //debug_vertices[5] = vec3(mesh->vertices[i2].position.x, high, mesh->vertices[i2].position.z);
        
        
        debug_vertex_count += 6;
        
        scratch_end(scratch);
    }
    
    
}

function void 
app_release() {
    
    render_release(render_state);
    gfx_context_release(graphics_context);
    os_window_close(window);
    
}

function void
app_frame() {
    
    os_update();
    
    // update context
    os_window_update(window);
    gfx_context_update(graphics_context);
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    if (!gfx_handle_equals(graphics_context, { 0 })) {
        context_size = gfx_context_get_size(graphics_context); 
        
        // update camera
        f32 dt = os_window_get_delta_time(window);
        camera_free_cam_input(camera, window);
        camera_update(camera, dt, vec2((f32)context_size.x, (f32)context_size.y));
        
        
        gfx_context_clear(graphics_context);
        
        gfx_context_present(graphics_context);
    }
    
}


//- main pass functions 

function void 
main_pass_setup(render_pass_t* pass) {
    
    log_infof("main_pass_setup");
    
}

function void 
main_pass_execute(render_pass_t* pass) {
    
    // render
    render_begin(render_state);
    
    gfx_pipeline_t default_pipeline = {
        gfx_fill_solid,
        gfx_cull_mode_back,
        gfx_topology_tris,
        gfx_filter_linear,
        gfx_wrap_repeat,
        gfx_depth,
        rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y),
        rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y)
    };
    
    gfx_pipeline_t debug_pipeline = {
        gfx_fill_solid,
        gfx_cull_mode_none,
        gfx_topology_lines,
        gfx_filter_nearest,
        gfx_wrap_repeat,
        gfx_depth,
        rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y),
        rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y)
    };
    
    render_set_next_pipeline(default_pipeline);
    render_set_next_shader(shader);
    render_set_next_constant(&camera->constants, sizeof(camera_constants_t), 0);
    render_set_next_texture(texture, 0);
    render_push_mesh(mesh->vertices, sizeof(mesh_vertex_t), mesh->vertex_count);
    
    render_push_pipeline(debug_pipeline);
    render_push_shader(debug_shader);
    render_push_constant(&camera->constants, sizeof(camera_constants_t), 0);
    
    for (u32 i = 0; i < debug_vertex_count; i += 2) {
        render_push_debug_line(debug_vertices[i], debug_vertices[i + 1], color(1.0f, 0.0f, 0.0f, 1.0f));
    }
    
    render_pop_constant();
    render_pop_shader();
    render_pop_pipeline();
    
    
    
    render_end(render_state);
}

function i32 
compare_dist(const void* a, const void* b) {
    index_dist_pair_t *pair_a = (index_dist_pair_t*)a;
    index_dist_pair_t *pair_b = (index_dist_pair_t*)b;
    
    if (pair_a->distance > pair_b->distance) return -1;
    if (pair_a->distance < pair_b->distance) return 1;
    return 0;
}

//- entry point

function i32 
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    gfx_init();
    
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    gfx_release();
    os_release();
    
	return 0;
}