// camera.cpp

#ifndef CAMERA_CPP
#define CAMERA_CPP

function camera_t*
camera_create(arena_t* arena, camera_mode mode, f32 fov, f32 z_near, f32 z_far) {
    
    camera_t* camera = (camera_t*)arena_calloc(arena, sizeof(camera_t));
    
    camera->mode = mode;
    camera->target_position = vec3(0.0f, 0.0f, 0.0f);
    camera->position = camera->target_position;
    
    camera->target_orientation = quat_look_at(camera->position, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    camera->orientation = camera->target_orientation;
    
    camera->target_fov = fov;
    camera->fov = fov;
    camera->z_near = z_near;
    camera->z_far = z_far;
    
    return camera;
}

function void
camera_look_at(camera_t* camera, vec3_t from, vec3_t to) {
    camera->target_position = from;
    camera->target_orientation = quat_look_at(camera->target_position, to, vec3(0.0f, 1.0f, 0.0f));
}

function void 
camera_free_mode_input(camera_t* camera, os_handle_t window) {
    
    // get delta time
    f32 dt = os_window_get_delta_time(window);
    
    // get input
    f32 forward_input = 0.0f;
    f32 right_input = 0.0f;
    f32 up_input = 0.0f;
    f32 roll_input = 0.0f;
    f32 pitch_input = 0.0f;
    f32 yaw_input = 0.0f;
    f32 target_speed = 0.5f;
    
    // only get input if window is focused
    if (os_window_is_active(window)) {
        forward_input = (f32)(os_key_is_down(os_key_W) - os_key_is_down(os_key_S));
        right_input = (f32)(os_key_is_down(os_key_D) - os_key_is_down(os_key_A));
        up_input = (f32)(os_key_is_down(os_key_space) - os_key_is_down(os_key_ctrl));
        roll_input = (f32)(os_key_is_down(os_key_E) - os_key_is_down(os_key_Q));
    }
    
    persist vec2_t mouse_start;
    
    // mouse input
    if (os_mouse_press(window, os_mouse_button_right)) {
        mouse_start =  os_window_get_cursor_pos(window);
        os_set_cursor(os_cursor_null);
    }
    
    if (os_mouse_release(window, os_mouse_button_right)) {
        os_set_cursor(os_cursor_pointer);
    }
    
    if (os_mouse_is_down(os_mouse_button_right) && os_window_is_active(window)) {
        vec2_t mouse_pos = os_window_get_cursor_pos(window);
        vec2_t delta = vec2_sub(mouse_start, mouse_pos);
        os_window_set_cursor_pos(window, mouse_start);
        yaw_input = delta.x;
        pitch_input = delta.y;
    }
    
    //speed
    if (os_key_is_down(os_key_shift)) {
        camera->target_speed = 15.0f;
    } else {
        camera->target_speed = 5.0f;
    }
    
    // fov
    f32 scroll_delta = os_mouse_scroll(window);
    camera->target_fov -= scroll_delta * 1.5f;
    camera->target_fov = clamp(camera->target_fov, 1.0f, 160.0f);
    
    // animate
    f32 rate = 1.0f - powf(2.0f, -30.0f * dt);
    
    camera->fov += (camera->target_fov - camera->fov) * rate;
    if (fabsf(camera->target_fov - camera->fov) < 0.05f) { camera->fov = camera->target_fov; }
    
    camera->speed += (camera->target_speed - camera->speed) * rate;
    if (fabsf(camera->target_speed - camera->speed) < 0.05f) { camera->speed = camera->target_speed; }
    
    camera->translational_input = vec3(forward_input, right_input, up_input);
    camera->rotational_input = vec3(pitch_input, yaw_input, roll_input);
}

function void 
camera_update(camera_t* camera, rect_t viewport, f32 dt) {
    
    f32 pitch_input = camera->rotational_input.x;
    f32 yaw_input = camera->rotational_input.y;
    f32 roll_input = camera->rotational_input.z;
    
    f32 forward_input = camera->translational_input.x;
    f32 right_input = camera->translational_input.y;
    f32 up_input = camera->translational_input.z;
    
    // clamp input
    vec3_t euler_angle = quat_to_euler_angle(camera->target_orientation);
    
    if (camera->mode & camera_mode_clamp_pitch) {
        pitch_input = max(degrees(camera->min_pitch - euler_angle.x), pitch_input);
        pitch_input = min(degrees(camera->max_pitch - euler_angle.x), pitch_input);
    }
    
    if (camera->mode & camera_mode_clamp_yaw) {
        yaw_input = max(degrees(camera->min_yaw - euler_angle.y), yaw_input);
        yaw_input = min(degrees(camera->max_yaw - euler_angle.y), yaw_input);
    }
    
    if (camera->mode & camera_mode_clamp_roll) {
        roll_input = max(degrees(camera->min_roll - euler_angle.z), roll_input);
        roll_input = min(degrees(camera->max_roll - euler_angle.z), roll_input);
    }
    
    const f32 sensitivity = 1.0f;
    f32 zoom_adjustment = (camera->fov / 160.0f);
    quat_t pitch = quat_from_axis_angle({ 1.0f, 0.0f, 0.0f }, zoom_adjustment * sensitivity * pitch_input * dt);
    quat_t yaw = quat_from_axis_angle({ 0.0f, 1.0f, 0.0f }, zoom_adjustment * sensitivity * yaw_input * dt);
    quat_t roll = quat_from_axis_angle({ 0.0f, 0.0f, 1.0f }, 2.5f * sensitivity * roll_input * dt);
    
    // orientation
    if (camera->mode & camera_mode_disable_roll) {
        camera->target_orientation = quat_mul(pitch, camera->target_orientation);
        camera->target_orientation = quat_mul(camera->target_orientation, yaw);
    } else {
        camera->target_orientation = quat_mul(pitch, camera->target_orientation);
        camera->target_orientation = quat_mul(yaw, camera->target_orientation);
        camera->target_orientation = quat_mul(roll, camera->target_orientation);
    }
    
    // smooth orientation
    camera->orientation = quat_slerp(camera->orientation, camera->target_orientation, 30.0f * dt);
    
    // translate
    vec3_t translation = vec3_mul(vec3_normalize(vec3(right_input, up_input, forward_input)), dt * camera->speed);
    vec3_t rotated_translation = vec3_rotate(translation, camera->orientation);
    camera->target_position = vec3_add(camera->target_position, rotated_translation);
    
    // animate
    f32 rate = 1.0f - powf(2.0f, -30.0f * dt);
    camera->position = vec3_add(camera->position, vec3_mul(vec3_sub(camera->target_position, camera->position), rate));
    if (fabsf(camera->target_position.x - camera->position.x) < 0.0001f) { camera->position.x = camera->target_position.x; }
    if (fabsf(camera->target_position.y - camera->position.y) < 0.0001f) { camera->position.y = camera->target_position.y; }
    if (fabsf(camera->target_position.z - camera->position.z) < 0.0001f) { camera->position.z = camera->target_position.z; }
    
    // update constants
    vec2_t size = vec2(viewport.x1 - viewport.x0, viewport.y1 - viewport.y0);
    
    camera->constants.view = mat4_mul(mat4_from_quat(camera->orientation), mat4_translate(vec3_negate(camera->position)));
    camera->constants.inv_view = mat4_inverse(camera->constants.view);
    camera->constants.projection = mat4_perspective(camera->fov, size.x / size.y, camera->z_near, camera->z_far);
    camera->constants.inv_projection = mat4_inverse(camera->constants.projection);
    camera->constants.view_projection = mat4_mul(camera->constants.projection, camera->constants.view);
    camera->constants.camera_position = camera->position;
    
    // reset input
    camera->translational_input = vec3(0.0f);
    camera->rotational_input = vec3(0.0f);
    
}

#endif // CAMERA_CPP

