// renderer_test.cpp

//~ includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "render/sora_render_inc.h"
#include "render/sora_render_inc.cpp"

//~ globals

global arena_t* arena;
global os_handle_t window;
global gfx_handle_t graphics_context;
global b8 quit = false;

global r_renderer_t* renderer;

global gfx_pipeline_t pipeline;
global r_camera_t* camera;
global gfx_handle_t shader;
global gfx_handle_t texture;
global r_resource_t* mesh;

//~ functions

// main render pass
function void main_pass_init(r_pass_t* pass);
function void main_pass_frame(r_pass_t* pass);

// app
function void app_init();
function void app_release();
function void app_frame();

//~ implementation

//- main render pass functions 

function void 
main_pass_init(r_pass_t* pass) {
    
    gfx_render_target_desc_t desc;
    desc.size = renderer->size;
    desc.sample_count = 8;
    desc.flags = gfx_render_target_flag_depth;
    desc.format = gfx_texture_format_rgba8;
    
    r_resource_t* render_target = r_render_target_create(renderer, str("render_target"), desc);
    
}

function void 
main_pass_frame(r_pass_t* pass) {
    
    r_resource_t* resource = r_resource_from_string(renderer, str("render_target"));
    
    // resize texture if needed
    uvec2_t context_size = gfx_context_get_size(graphics_context);
    uvec2_t render_target_size = gfx_render_target_get_size(resource->render_target);
    if (!uvec2_equals(render_target_size, context_size)) {
        gfx_render_target_resize(resource->render_target, context_size);
    }
    
    // update pipeline
    pipeline.viewport = rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y);
    pipeline.scissor = rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y);
    
    // clear render target
    gfx_render_target_clear(resource->render_target, color(0x131517ff));
    
    r_begin(pass->renderer);
    r_push_render_target(resource->render_target);
    r_push_pipeline(pipeline);
    r_push_shader(shader);
    r_push_texture(texture, 0);
    r_push_constant(&camera->constants, sizeof(r_camera_constants_t), 0);
    
    r_draw_mesh(mesh->mesh);
    
    r_pop_render_target();
    r_end(pass->renderer);
}

//- app functions 

function void 
app_init() {
    
    arena = arena_create(gigabytes(2));
    window = os_window_open(str("renderer test"), 1280, 720, os_window_flag_maximize);
    graphics_context = gfx_context_create(window, color(0x111215ff));
    
    os_window_set_frame_function(window, app_frame);
    
    // create renderer
    renderer = r_renderer_create(graphics_context);
    
    // create main pass
    r_pass_t* main_pass = r_pass_create(renderer, main_pass_init, main_pass_frame);
    r_pass_add_output(main_pass, str("output"), str("render_target"));
    r_pass_link(main_pass, str("output"), renderer->output_pass, str("input"));
    
    // init renderer
    r_init(renderer);
    
    // create camera
    camera = r_camera_create(renderer, r_camera_mode_free, 80.0f, 0.01f, 100.0f);
    
    // load mesh
    mesh = r_mesh_load(renderer, str("res/models/tileset.obj"));
    
    // load shader
    gfx_shader_attribute_t attributes[] = {
        { "POSITION", 0, gfx_vertex_format_float3, gfx_vertex_class_per_vertex },
        { "NORMAL", 0, gfx_vertex_format_float3, gfx_vertex_class_per_vertex },
        { "TANGENT", 0, gfx_vertex_format_float3, gfx_vertex_class_per_vertex },
        { "TEXCOORD", 0, gfx_vertex_format_float2, gfx_vertex_class_per_vertex },
        { "COLOR", 0, gfx_vertex_format_float4, gfx_vertex_class_per_vertex },
    };
    //shader = gfx_shader_load(str("res/shaders/shader_3d.hlsl"), attributes, array_count(attributes));
    
    //texture = gfx_texture_load(str("res/textures/matcap_4.png"));
    
    // create pipeline
    uvec2_t context_size = gfx_context_get_size(graphics_context);
    
    pipeline = { 
        gfx_fill_solid,
        gfx_cull_mode_back,
        gfx_topology_tris,
        gfx_filter_linear,
        gfx_wrap_repeat,
        gfx_depth,
        rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y),
        rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y)
    };
    
}

function void 
app_release() {
    r_renderer_release(renderer);
    
    gfx_context_release(graphics_context);
    os_window_close(window);
    arena_release(arena);
}


function void
app_frame() {
    os_get_events();
    
    // update 
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
    
    uvec2_t context_size = gfx_context_get_size(graphics_context);
    f32 dt = os_window_get_delta_time(window);
    
    // update camera 
    r_camera_free_mode_input(camera, window);
    r_camera_update(camera, rect(0.0f, 0.0f, (f32)context_size.x, (f32)context_size.y), dt);
    
    // render frame
    r_frame(renderer);
    
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