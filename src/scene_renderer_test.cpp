// scene_renderer_test.cpp

// TODO:
//
// [ ] - finish panel docking.
//     [ ] - rework tab redordering.
//     [ ] - move panels around.
//     [ ] - dropspaces.
// [ ] - transform widgets.
//     [ ] - float edit -> text edit. (simple eval system?)
//     [ ] - 
// [ ] - implement pbr. maybe materials?
// [ ] - add in render passes again.
// [ ] - post processing.
// [ ] - volumetrics.
// [ ] - gizmos
// [ ] - 

//~ includes

#include "core/sora_inc.h"
//#include "utils/profile.h"

#include "core/sora_inc.cpp"
//#include "utils/profile.cpp"

#include "ui/sora_ui_inc.h"
#include "ui/sora_ui_inc.cpp"

#include "game/game_inc.h"
#include "game/game_inc.cpp"


//~ globals

global arena_t* arena;
global os_handle_t window;
global gfx_handle_t context;
global ui_context_t* ui;
global b8 quit = false;

global scene_t* scene;
global renderer_t* renderer;
global camera_t* camera;

global mesh_t* mesh0;
global mesh_t* mesh1;
global gfx_handle_t vertex_shader;
global gfx_handle_t pixel_shader;
global gfx_handle_t water_vertex_shader;
global gfx_handle_t water_pixel_shader;

global ui_dockspace_t* dockspace;


//~ functions

// app
function void app_init();
function void app_release();
function void app_frame();

//~ implementation 

//- app functions 

function void 
app_init() {
    
    // create arena
    arena = arena_create(gigabytes(2));
    
    // open window and create renderer
    window = os_window_open(str("scene renderer"), 1280, 720, os_window_flag_maximize);
    os_window_set_frame_function(window, app_frame);
    
    // create graphics context
    context = gfx_context_create(window);
    
    // create ui context and set theme
    ui = ui_context_create(window, context);
    
    // set ui theme
    ui_context_set_color(ui, str("background"), color(0x232627ff));
    ui_context_set_color(ui, str("background alt"), color(0x141718ff));
    ui_context_set_color(ui, str("background shadow"), color(0x00000080));
    ui_context_set_color(ui, str("hover"), color(0xe7f1ff25));
    ui_context_set_color(ui, str("active"), color(0xe7f1ff25));
    ui_context_set_color(ui, str("border"), color(0x343839ff));
    ui_context_set_color(ui, str("border shadow"), color(0x00000080));
    ui_context_set_color(ui, str("text"), color(0xe2e2e3ff));
    ui_context_set_color(ui, str("text shadow"), color(0x00000080));
    ui_context_set_color(ui, str("accent"), color(0x0084ffff));
    ui_context_set_color(ui, str("background list"), color(0x141718ff));
    ui_context_set_color(ui, str("background list alt"), color(0x191c1dff));
    ui_context_set_color(ui, str("text icon"), color(0x85cf60ff));
    ui_context_set_color(ui, str("border red"), color(0xe9e2835ff));
    ui_context_set_color(ui, str("border green"), color(0x328345ff));
    ui_context_set_color(ui, str("border blue"), color(0x0474c1ff));
    
    ui_context_set_color(ui, str("text green"), color(0x22C55Eff));
    ui_context_set_color(ui, str("text orange"), color(0xF97316ff));
    ui_context_set_color(ui, str("text red"), color(0xEF4444ff));
    
    // load assets
    mesh0 = mesh_load(arena, str("res/models/cube.obj"));
    mesh1 = mesh_load(arena, str("res/models/sphere.obj"));
    vertex_shader = gfx_shader_load(str("res/shaders/shader_pbr.hlsl"), gfx_shader_flag_vertex);
    pixel_shader = gfx_shader_load(str("res/shaders/shader_pbr.hlsl"), gfx_shader_flag_pixel);
    water_vertex_shader = gfx_shader_load(str("res/shaders/shader_water.hlsl"), gfx_shader_flag_vertex);
    water_pixel_shader = gfx_shader_load(str("res/shaders/shader_water.hlsl"), gfx_shader_flag_pixel);
    // create scene
    scene = scene_create();
    
    scene_node_t* node1 = scene_node_create(scene, str("test 1"));
    scene_node_t* node2 = scene_node_create(scene, str("test 2"));
    scene_node_t* node3 = scene_node_create(scene, str("test 3"));
    scene_node_t* node4 = scene_node_create(scene, str("test 4"));
    scene_node_t* node5 = scene_node_create(scene, str("test 5"));
    scene_node_t* node6 = scene_node_create(scene, str("test 6"));
    scene_node_t* node7 = scene_node_create(scene, str("test 7"));
    scene_node_t* node8 = scene_node_create(scene, str("test 8"));
    
    scene_node_add_child(node2, node3);
    scene_node_add_child(node5, node6);
    scene_node_add_child(node6, node7);
    
    node3->mesh = mesh1;
    
    // create camera
    camera = camera_create(arena, camera_mode_free, 80.0f, 0.01f, 100.0f);
    
    // create renderer
    renderer = renderer_create(context);
    
    
    
    dockspace = ui_dockspace_create(ui);
    ui_view_t* view0 = ui_view_create(dockspace, str("Tab 1"), nullptr);
    ui_view_t* view1 = ui_view_create(dockspace, str("Tab 2"), nullptr);
    ui_view_t* view2 = ui_view_create(dockspace, str("Tab 3"), nullptr);
    ui_view_insert(view2, dockspace->panel_root);
    ui_view_insert(view1, dockspace->panel_root);
    ui_view_insert(view0, dockspace->panel_root);
    
}


function void 
app_release() {
    
    renderer_release(renderer);
    
    // unload assets
    gfx_shader_release(vertex_shader);
    gfx_shader_release(pixel_shader);
    
    scene_release(scene);
    
    ui_context_release(ui);
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
    
    //profile_begin_frame();
    //prof_begin("app_frame");
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    // build ui
    ui_begin(ui);
    
    // scene heirarchy list
    ui_scene_hierarchy(scene);
    
    ui_scene_node_properties(scene->node_selected);
    
    
    ui_push_size(ui_size_pixels(400.0f), ui_size_pixels(25.0f));
    
    f64 gpu_dt = gfx_context_get_delta_time(context);
    ui_labelf("gpu frame time: %.3f ms (%.1f fps)", gpu_dt * 1000.0f, 1.0f / gpu_dt);
    
    // profiler
    /*for (i32 i = 0; i < profiler.last_entry_count; i++) {
        profile_entry_t* entry = &profiler.last_entries[i];
        f64 elapsed_time = (f32)entry->cycles * 1000.0f / (f64)profiler.cpu_freq;
        
        if (elapsed_time < 2.0f) {
            ui_push_tagf("green");
        } else if (elapsed_time < 5.0f) {
            ui_push_tagf("orange");
        } else {
            ui_push_tagf("red");
        }
        
        ui_row_begin();
        ui_push_size(ui_size_pixels(100.0f), ui_size_percent(1.0f));
        ui_push_text_alignment(ui_text_align_left);
        
        ui_labelf("%s", entry->name);
        ui_labelf("%.2f (ms)", elapsed_time);
        ui_labelf("%u (cycles)", entry->cycles);
        ui_labelf("%u %s", entry->hit_count, entry->hit_count == 1 ? "hit" : "hits");
        
        ui_pop_text_alignment();
        ui_pop_size();
        ui_row_end();
        
        
    }
    */
    ui_pop_size();
    
    
    ui_end(ui);
    
    f32 dt = os_window_get_delta_time(window);
    uvec2_t window_size = os_window_get_size(window);
    rect_t viewport = rect(0.0f, 0.0f, (f32)window_size.x, (f32)window_size.y);
    
    // update camera
    camera_free_mode_input(camera, window);
    camera_update(camera, viewport, dt);
    renderer_set_camera(renderer, camera);
    
    // update scene
    scene_update(scene);
    
    // render
    gfx_set_context(context);
    gfx_context_clear(context, color(0x1D1F20ff));
    
    // render scene
    for (scene_node_t* node = scene->node_root, *next = nullptr; node != nullptr; node = next) {
        scene_node_rec_t rec = scene_node_rec_depth_first(node, false);
        next = rec.next;
        
        if (node->mesh != nullptr) {
            //renderer_draw_mesh(renderer, node->mesh, node->transform, water_vertex_shader, water_pixel_shader);
        }
        
    }
    
    // render ui
    ui_render(ui);
    
    gfx_context_present(context);
    
    
    //prof_end();
    //profile_end_frame();
}

//- entry point

function i32 
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    //profile_init(); 
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
    //profile_release();
    os_release();
    
	return 0;
}