// camera.h

#ifndef CAMERA_H
#define CAMERA_H

//- enums 

typedef u32 camera_mode;
enum {
	camera_mode_disable_roll = (1 << 0),
	camera_mode_disable_move_in_world_plane = (1 << 1),
	camera_mode_clamp_pitch = (1 << 2),
	camera_mode_clamp_yaw = (1 << 3),
	camera_mode_clamp_roll = (1 << 4),
    
	camera_mode_free = 0,
	camera_mode_first_person = camera_mode_disable_roll | camera_mode_disable_move_in_world_plane | camera_mode_clamp_pitch,
	camera_mode_orbit = camera_mode_disable_roll | camera_mode_clamp_pitch
};

//- structs 

struct camera_constants_t {
    mat4_t view_projection;
    mat4_t view;
    mat4_t projection;
    mat4_t inv_view;
    mat4_t inv_projection;
    vec3_t camera_position;
};

struct camera_t {
    
    camera_mode mode;
    camera_constants_t constants;
    
    vec3_t target_position;
    vec3_t position;
    
    f32 target_distance;
    f32 distance;
    
    quat_t target_orientation;
    quat_t orientation;
    
    vec3_t translational_input;
    vec3_t rotational_input;
    
    f32 speed;
    f32 target_speed;
    f32 target_fov;
    f32 fov;
    f32 z_near;
    f32 z_far;
    f32 min_pitch;
    f32 max_pitch;
    f32 min_yaw;
    f32 max_yaw;
    f32 min_roll;
    f32 max_roll;
};

//- functions 

function camera_t* camera_create(arena_t* arena, camera_mode mode, f32 fov, f32 z_near, f32 z_far);
function void camera_free_mode_input(camera_t* camera, os_handle_t window);
function void camera_update(camera_t* camera, rect_t viewport, f32 dt);

#endif // CAMERA_H