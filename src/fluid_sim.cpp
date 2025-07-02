// lock_free_queue.cpp

//~ includes

#include "core/sora_inc.h" // core engine
#include "ui/sora_ui_inc.h" // ui layer
#include "utils/camera.h" // camera

#include "core/sora_inc.cpp"
#include "ui/sora_ui_inc.cpp"
#include "utils/camera.cpp"

// TODO:
//
// [ ] - spatial grid pass
// [ ] - implement a gpu sort
// [ ] - calculate densities
// [ ] - calculate pressure forces
// [ ] - 

//~ defines

//~ structs 

struct color_ramp_segment_t {
    color_t color;
    f32 t;
};

struct spatial_index_t {
    u32 index;
    u32 hash;
    u32 key;
};

struct sort_constants_t {
    u32 count;
    u32 group_width;
    u32 group_height;
    u32 step_index;
};

struct sim_constants_t {
    u32 count;
    f32 smoothing_radius;
    f32 delta_time;
    f32 target_density;
    f32 pressure_multiplier;
    f32 near_pressure_multiplier;
    u32 padding0;
    u32 padding1;
};

//~ globals

global arena_t* arena;
global os_handle_t window;
global gfx_handle_t context;
global ui_context_t* ui;
global b8 quit = false;
global camera_t* camera;

// speed color ramp
global u32 segment_count = 4;
global color_ramp_segment_t segments[4] = { 
    { color(0x3A2FCEff), 0.0f },
    { color(0x96FFCCff), 0.6f },
    { color(0xFFC61Eff), 0.75f },
    { color(0xFF6214ff), 1.0f },
};

// sim
global u32 particle_count = 1000; 

// compute shaders

global i32 thread_group_size = 256;
global i32 num_groups = (particle_count + thread_group_size - 1) / thread_group_size;

global gfx_handle_t initial_setup_compute_shader;
global gfx_handle_t external_forces_compute_shader;
global gfx_handle_t spatial_grid_compute_shader;
global gfx_handle_t sort_compute_shader; 
global gfx_handle_t calculate_offsets_compute_shader; 
global gfx_handle_t density_compute_shader;
global gfx_handle_t pressure_force_compute_shader;
global gfx_handle_t integrate_compute_shader;

global sort_constants_t sort_constants;
global gfx_handle_t sort_constant_buffer;
global sim_constants_t sim_constants;
global gfx_handle_t sim_constant_buffer;

// particle buffers
global gfx_handle_t positions_buffer;
global gfx_handle_t velocities_buffer;
global gfx_handle_t densities_buffer;
global gfx_handle_t predicted_positions_buffer;
global gfx_handle_t spatial_indices_buffer;
global gfx_handle_t spatial_offsets_buffer;
global gfx_handle_t cell_buffer;
global gfx_handle_t colors_buffer;

// renderer
global gfx_handle_t vertex_shader;
global gfx_handle_t pixel_shader;
global gfx_handle_t camera_constants;
global gfx_handle_t render_target;
global gfx_handle_t render_target_texture;

//~ functions

// helper
function color_t color_ramp_sample(color_ramp_segment_t* segments, u32 count, f32 t);
function u32 pack_color(color_t col);

// app
function void app_init();
function void app_release();
function void app_frame();

//~ implementation 

//- helper functions 

function color_t
color_ramp_sample(color_ramp_segment_t* segments, u32 count, f32 t) {
    
    color_t result;
    
    if (t <= 0.0f) {
        result = segments[0].color;
    } else if (t >= 1.0f){
        result = segments[count - 1].color;
    } else {
        
        for (int i = 0; i < count - 1; i++) {
            f32 t0 = segments[i].t;
            f32 t1 = segments[i + 1].t;
            if (t >= t0 && t <= t1) {
                f32 local_t = (t - t0) / (t1 - t0);
                result = color_lerp(segments[i].color, segments[i + 1].color, local_t);
            }
        }
    }
    
    return result;
}

function u32 
pack_color(color_t col) {
    u32 r = (u32)(col.r * 255.0f) & 0xff;
    u32 g = (u32)(col.g * 255.0f) & 0xff;
    u32 b = (u32)(col.b * 255.0f) & 0xff;
    u32 a = (u32)(col.a * 255.0f) & 0xff;
    
    return (a << 24) | (b << 16) | (g << 8) | r;
}

function u32
next_power_of_two(u32 n) {
    if (n == 0) return 1;
    
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    
    return n;
}

function f32 
log(f32 f, f32 p) {
    return logf(f) / logf(p);
}

function void 
dispatch_shader(gfx_handle_t shader) {
    
    gfx_set_shader(shader);
    
    // set constant buffers
    gfx_set_buffer(sim_constant_buffer, 1);
    
    // set structured buffers
    gfx_set_buffer(positions_buffer, 0);
    gfx_set_buffer(velocities_buffer, 1);
    gfx_set_buffer(predicted_positions_buffer, 2);
    gfx_set_buffer(densities_buffer, 3);
    gfx_set_buffer(spatial_indices_buffer, 4);
    gfx_set_buffer(spatial_offsets_buffer, 5);
    gfx_set_buffer(colors_buffer, 6);
    
    gfx_dispatch(num_groups, 1, 1);
    
    // unset all
    gfx_set_buffer({ 0 }, 0);
    gfx_set_buffer({ 0 }, 1);
    gfx_set_buffer({ 0 }, 2);
    gfx_set_buffer({ 0 }, 3);
    gfx_set_buffer({ 0 }, 4);
    gfx_set_buffer({ 0 }, 5);
    gfx_set_buffer({ 0 }, 6);
    
    gfx_set_shader({ 0 });
    
}

//- app functions 

function void 
app_init() {
    
    //printf("size: %u\n", sizeof(sim_constants_t));
    
    // create arena
    arena = arena_create(gigabytes(2));
    
    // open window and create renderer
    window = os_window_open(str("fluid sim"), 1280, 720, 0);
    os_window_set_frame_function(window, app_frame);
    
    // create graphics context
    context = gfx_context_create(window);
    
    // create ui context
    ui = ui_context_create(window, context);
    ui_context_default_theme(ui);
    
    // create camera
    camera = camera_create(arena, camera_mode_free, 80.0f, 0.01f, 1000.0f);
    camera_look_at(camera, vec3(35.0f, 5.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
    camera_constants = gfx_buffer_create(gfx_buffer_type_constant, sizeof(camera_constants_t));
    
    // create render target
    gfx_render_target_desc_t render_target_desc;
    render_target_desc.colorbuffer_format = gfx_texture_format_rgba8;
    render_target_desc.depthbuffer_format = gfx_texture_format_d32;
    render_target_desc.size = gfx_context_get_size(context);
    render_target_desc.sample_count = 8;
    render_target = gfx_render_target_create_ex(render_target_desc);
    render_target_texture = gfx_render_target_get_texture(render_target);
    
    // load shaders
    vertex_shader = gfx_shader_load(str("res/shaders/shader_fluid.hlsl"), gfx_shader_flag_vertex | gfx_shader_flag_per_instance);
    pixel_shader = gfx_shader_load(str("res/shaders/shader_fluid.hlsl"), gfx_shader_flag_pixel);
    
    // load compute shaders
    initial_setup_compute_shader = gfx_compute_shader_load(str("res/shaders/shader_compute_fluid.hlsl"), str("intial_setup_pass"));
    external_forces_compute_shader = gfx_compute_shader_load(str("res/shaders/shader_compute_fluid.hlsl"), str("external_forces_pass"));
    spatial_grid_compute_shader = gfx_compute_shader_load(str("res/shaders/shader_compute_fluid.hlsl"), str("spatial_grid_pass"));
    sort_compute_shader = gfx_compute_shader_load(str("res/shaders/shader_compute_fluid.hlsl"), str("sort_pass"));
    calculate_offsets_compute_shader = gfx_compute_shader_load(str("res/shaders/shader_compute_fluid.hlsl"), str("calculate_offsets_pass"));
    density_compute_shader = gfx_compute_shader_load(str("res/shaders/shader_compute_fluid.hlsl"), str("density_pass"));
    pressure_force_compute_shader = gfx_compute_shader_load(str("res/shaders/shader_compute_fluid.hlsl"), str("pressure_force_pass"));
    integrate_compute_shader = gfx_compute_shader_load(str("res/shaders/shader_compute_fluid.hlsl"), str("integrate_pass"));
    
    // create buffers 
    gfx_buffer_desc_t desc;
    desc.type = gfx_buffer_type_structured;
    desc.size = particle_count; // this becomes count
    desc.usage = gfx_usage_dynamic;
    
    desc.stride = sizeof(vec3_t);
    positions_buffer = gfx_buffer_create_ex(desc);
    velocities_buffer = gfx_buffer_create_ex(desc);
    predicted_positions_buffer = gfx_buffer_create_ex(desc);
    
    desc.stride = sizeof(vec2_t);
    densities_buffer = gfx_buffer_create_ex(desc);
    
    desc.stride = sizeof(spatial_index_t);
    spatial_indices_buffer = gfx_buffer_create_ex(desc);
    
    desc.stride = sizeof(u32);
    spatial_offsets_buffer = gfx_buffer_create_ex(desc);
    
    desc.stride = sizeof(u32);
    colors_buffer = gfx_buffer_create_ex(desc);
    
    
    desc.stride = sizeof(u32) * 3;
    cell_buffer = gfx_buffer_create_ex(desc);
    
    sort_constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, sizeof(sort_constants_t));
    
    sim_constants.count = particle_count;
    sim_constants.smoothing_radius = 0.5f;
    sim_constants.delta_time = 0.061f;
    sim_constants.target_density = 10.0f;
    sim_constants.pressure_multiplier = 10.0f;
    sim_constants.near_pressure_multiplier = 1.0f;
    sim_constant_buffer = gfx_buffer_create(gfx_buffer_type_constant, sizeof(sim_constants_t));
    gfx_buffer_fill(sim_constant_buffer, &sim_constants, sizeof(sim_constants_t));
    
    gfx_set_shader(initial_setup_compute_shader);
    gfx_set_buffer(positions_buffer, 0);
    gfx_set_buffer(velocities_buffer, 1);
    gfx_set_buffer(colors_buffer, 6);
    gfx_set_buffer(sim_constant_buffer, 1);
    gfx_dispatch(num_groups, 1, 1);
    gfx_set_buffer({0}, 0);
    gfx_set_buffer({0}, 1);
    gfx_set_buffer({0}, 6);
    
}


function void 
app_release() {
    gfx_context_release(context);
    os_window_close(window);
    arena_release(arena);
}

function void
app_frame() {
    os_get_events();
    
    // update window and context
    os_window_update(window);
    gfx_context_update(context);
    
    f32 dt = os_window_get_delta_time(window);
    uvec2_t window_size = os_window_get_size(window);
    rect_t viewport = rect(0.0f, 0.0f, (f32)window_size.x, (f32)window_size.y);
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    persist f32 time_step = 1.0f;
    
    //- update 
    {
        
        // update the camera
        camera_free_mode_input(camera, window);
        camera_update(camera, viewport, dt);
        gfx_buffer_fill(camera_constants, &camera->constants, sizeof(camera_constants_t));
        
        // build ui
        ui_begin(ui);
        ui_push_size(ui_size_pixels(300.0f), ui_size_pixels(25.0f));
        
        ui_set_next_text_alignment(ui_text_align_left);
        ui_labelf("frame_time: %.2f ms (%.1f fps)", dt * 1000.0f, 1.0f / dt);
        
        
        ui_sliderf(&time_step, 0.00f, 1.0f, "time_steps");
        ui_sliderf(&sim_constants.smoothing_radius, 0.01f, 2.0f, "smoothing_radius");
        ui_sliderf(&sim_constants.target_density, 0.01f, 1000.0f, "target_density");
        ui_sliderf(&sim_constants.pressure_multiplier, 0.01f, 100.0f, "pressure_multiplier");
        ui_sliderf(&sim_constants.near_pressure_multiplier, 0.01f, 100.0f, "near_pressure_multiplier");
        
        ui_pop_size();
        ui_end(ui);
        
    }
    
    //- sim update
    for (i32 i = 0; i < 1; i++) {
        
        // update settings
        sim_constants.delta_time = lerp(0.0f, dt / 1.0f, time_step);
        gfx_buffer_fill(sim_constant_buffer, &sim_constants, sizeof(sim_constants_t));
        
        // external forces
        dispatch_shader(external_forces_compute_shader);
        
        // spatial grid
        dispatch_shader(spatial_grid_compute_shader);
        
        // bitonic merge sort
        sort_constants.count = particle_count;
        gfx_set_shader(sort_compute_shader);
        
        // set structured buffers
        gfx_set_buffer(positions_buffer, 0);
        gfx_set_buffer(velocities_buffer, 1);
        gfx_set_buffer(predicted_positions_buffer, 2);
        gfx_set_buffer(densities_buffer, 3);
        gfx_set_buffer(spatial_indices_buffer, 4);
        gfx_set_buffer(spatial_offsets_buffer, 5);
        gfx_set_buffer(colors_buffer, 6);
        
        
        i32 num_stage = (i32)log(next_power_of_two(particle_count), 2);
        for (i32 i = 0; i < num_stage; i++) {
            for (i32 j = 0; j < i + 1; j++) {
                
                sort_constants.group_width = (1 << i - j);
                sort_constants.group_height = 2 * sort_constants.group_width - 1;
                sort_constants.step_index = j;
                gfx_buffer_fill(sort_constant_buffer, &sort_constants, sizeof(sort_constants_t));
                gfx_set_buffer(sort_constant_buffer, 0); // constant buffer
                
                u32 interations = next_power_of_two(particle_count) / 2;
                u32 num_group = interations / thread_group_size;
                gfx_dispatch(num_group, 1, 1);
                
            }
        }
        
        // unset all
        gfx_set_buffer({ 0 }, 0);
        gfx_set_buffer({ 0 }, 1);
        gfx_set_buffer({ 0 }, 2);
        gfx_set_buffer({ 0 }, 3);
        gfx_set_buffer({ 0 }, 4);
        gfx_set_buffer({ 0 }, 5);
        gfx_set_buffer({ 0 }, 6);
        gfx_set_shader({ 0 });
        
        // calculate offsets
        dispatch_shader(calculate_offsets_compute_shader);
        
        // density 
        dispatch_shader(density_compute_shader);
        
        // pressure forces
        dispatch_shader(pressure_force_compute_shader);
        
        // intergrate
        dispatch_shader(integrate_compute_shader);
        
    }
    
    
    //- render 
    {
        
        // resize render target if needed
        uvec2_t render_target_size = gfx_render_target_get_size(render_target);
        if (!uvec2_equals(render_target_size, window_size)) {
            gfx_render_target_resize(render_target, window_size);
        }
        
        gfx_set_render_target(render_target);
        gfx_render_target_clear(render_target);
        
        gfx_set_viewport(viewport);
        gfx_set_scissor(viewport);
        gfx_set_rasterizer(gfx_fill_solid, gfx_cull_back);
        gfx_set_topology(gfx_topology_tri_strip);
        gfx_set_sampler(gfx_filter_nearest, gfx_wrap_repeat, 0);
        gfx_set_depth_mode(gfx_depth);
        
        gfx_set_shader(vertex_shader);
        gfx_set_shader(pixel_shader);
        gfx_set_buffer(positions_buffer, 0);
        gfx_set_buffer(colors_buffer, 1);
        gfx_set_buffer(camera_constants, 0);
        gfx_draw_instanced(4, particle_count);
        
        gfx_context_clear(context, color(0x000000ff));
        gfx_set_context(context);
        
        // blit render target set
        gfx_context_blit(context, render_target_texture);
        
        ui_render(ui);
        
        gfx_context_present(context);
    }
    
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