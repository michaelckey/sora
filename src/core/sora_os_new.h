// sora_os_new.h

#ifndef SORA_OS_H
#define SORA_OS_H

//~ typedefs

typedef void os_thread_func(void*);
typedef void os_fiber_func(void*);
typedef void os_frame_func();

//~ enums 

enum os_key {
    
    // modifiers
    os_key_left_shift,
    os_key_right_shift,
    os_key_left_ctrl,
    os_key_right_ctrl,
    os_key_left_alt,
    os_key_right_alt,
    os_key_left_super,
    os_key_right_super,
    
    // function
    os_key_esc,
    os_key_f1,
    os_key_f2,
    os_key_f3,
    os_key_f4,
    os_key_f5,
    os_key_f6,
    os_key_f7,
    os_key_f8,
    os_key_f9,
    os_key_f10,
    os_key_f11,
    os_key_f12,
    
    // navigation
    os_key_up,
    os_key_down,
    os_key_left,
    os_key_right,
    os_key_home,
    os_key_end,
    os_key_page_up,
    os_key_page_down,
    os_key_insert,
    os_key_delete,
    os_key_backspace,
    os_key_tab,
    os_key_enter,
    
    // letters
    os_key_a,
    os_key_b,
    os_key_c,
    os_key_d,
    os_key_e,
    os_key_f,
    os_key_g,
    os_key_h,
    os_key_i,
    os_key_j,
    os_key_k,
    os_key_l,
    os_key_m,
    os_key_n,
    os_key_o,
    os_key_p,
    os_key_q,
    os_key_r,
    os_key_s,
    os_key_t,
    os_key_u,
    os_key_v,
    os_key_w,
    os_key_x,
    os_key_y,
    os_key_z,
    
    // numbers
    os_key_0,
    os_key_1,
    os_key_2,
    os_key_3,
    os_key_4,
    os_key_5,
    os_key_6,
    os_key_7,
    os_key_8,
    os_key_9,
    
    // symbols
    os_key_space,
    os_key_minus,
    os_key_equals,
    os_key_left_bracket,
    os_key_right_bracket,
    os_key_backslash,
    os_key_semicolon,
    os_key_apostrophe,
    os_key_grave,
    os_key_comma,
    os_key_period,
    os_key_slash,
    
    // numpad
    os_key_num_0,
    os_key_num_1,
    os_key_num_2,
    os_key_num_3,
    os_key_num_4,
    os_key_num_5,
    os_key_num_6,
    os_key_num_7,
    os_key_num_8,
    os_key_num_9,
    os_key_num_add,
    os_key_num_sub,
    os_key_num_mul,
    os_key_num_div,
    os_key_num_dec,
    os_key_num_enter,
    
    // locks and misc
    os_key_caps_lock,
    os_key_scroll_lock,
    os_key_num_lock,
    os_key_print_screen,
    os_key_pause,
    
    os_key_count,
};

enum os_mouse_button {
    os_mouse_button_left,
    os_mouse_button_middle,
    os_mouse_button_right,
    
    os_mouse_button_count,
};

// TODO: add gamepad support
enum os_gamepad_button {
    
    // face                xbox  |  playstation
    os_gamepad_button_a, //  a   |    cross
    os_gamepad_button_b, //  b   |    circle
    os_gamepad_button_x, //  x   |    square
    os_gamepad_button_y, //  y   |    triangle
    
    // bumpers
    os_gamepad_button_left_bumper,
    os_gamepad_button_right_bumper,
    
    // thumb
    os_gamepad_button_left_thumb,
    os_gamepad_button_right_thumb,
    
    // dpad
    os_gamepad_button_dpad_up,
    os_gamepad_button_dpad_down,
    os_gamepad_button_dpad_left,
    os_gamepad_button_dpad_right,
    
    // system
    os_gamepad_button_start,
    os_gamepad_button_select,
    os_gamepad_button_home,
    
    os_gamepad_button_count,
};


typedef u32 os_modifier;
enum {
    os_modifier_none  = (0),
	os_modifier_shift = (1 << 0),
	os_modifier_ctrl  = (1 << 1),
	os_modifier_alt   = (1 << 2),
	os_modifier_super = (1 << 3),
    os_modifier_any   = (~0u),
};

enum os_event_type {
    os_event_null,
    
    // window events
    os_event_window_close,
    os_event_window_resize,
    os_event_window_move,
    os_event_window_focus,
    os_event_window_unfocus,
    os_event_window_minimize,
    os_event_widnow_restore,
    os_event_window_maximize,
    os_event_window_paint,
    
    // keyboard events
    os_event_key_press,
    os_event_key_release,
    os_event_char,
    
    // mouse events
    os_event_mouse_move,
    os_event_mouse_enter,
    os_event_mouse_leave,
    os_event_mouse_button_press,
    os_event_mouse_button_release,
    os_event_mouse_scroll,
    
    // drag and drop
    os_event_drag_enter,
    os_event_drag_leave,
    os_event_drag_drop,
    
    // clipboard events
    os_event_clipboard_update,
    
    // timer events
    os_event_timer,
    
    // system events
    os_event_power_suspend,
    os_event_power_resume,
    os_event_display_change,
    
    // NOTE: this is unused, but might be needed in the future for mobile devices.
    os_event_touch_begin,
    os_event_touch_update,
    os_event_touch_end,
    
};

//~ structs 

// handles

struct os_window_t { u64 id; };

struct os_file_t { u64 id; };
struct os_file_iter_t { u64 id; } // TODO: implement this 

struct os_process_t { u64 id; } // TODO: implement this
struct os_pipe_t { u64 id; } // TODO: implement this 

struct os_thread_t { u64 id; };
struct os_fiber_t { u64 id; };
struct os_mutex_t { u64 id; };
struct os_rw_mutex_t { u64 id; };
struct os_condition_variable_t { u64 id; };
struct os_semaphore_t { u64 id; }; // TODO: implement this

struct os_timer_t { u64 id; } // TODO: implement this 

// NOTE: this will be implemented when I get to networking. 
struct os_socket_t { u64 id; } // TODO: implement this 

// events
struct os_event_t {
    os_event_t* next;
    os_event_t* prev;
    
    os_event_type type;
    
    u64 frame_count;
    u64 timestamp;
    os_modifier modifiers;
    
    // window events
    os_handle_t window;
    uvec2_t window_position;
    uvec2_t window_size;
    
    // keyboard events
    os_key key;
    u32 char_code;
    str_t string;
    
    // mouse events
    os_mouse_button mouse_button;
    vec2_t mouse_position; 
    vec2_t mouse_scroll;
    
};

struct os_event_list_t {
    os_event_t* first;
    os_event_t* last;
    u32 count;
};

struct os_system_info_t {
    str_t os_name;
    str_t os_version;
    str_t architecture;
    str_t host_name;
    str_t user_name;
    
    u32 logical_core_count;
    u64 memory_available;
    u64 memory_total;
};

// TODO: implement this
struct os_process_info_t {
    u32 pid;
    u32 parent_pid;
    str_t exe_name;
    str_t path;
    str_t user;
    f64 cpu_usage;
    u64 memory_usage;
    u64 start_timestamp;
    b8 is_running;
};

// core state
struct os_core_state_t {
    
    // events
    arena_t* events_arena;
    os_event_list_t events;
    
    os_system_info_t system_info;
    os_process_info_t process_info;
    
};

//~ globals

global os_core_state_t os_core_state;

//~ functions 

// state (implemented per backend)
function void os_init();
function void os_release();
function void os_poll_events(); // NOTE: clears event_list before polling.
function void os_abort(u32 exit_code);
function void os_sleep(u32 ms);

function os_system_info_t os_get_system_info();

// timing (implemented per backend)
function u64 os_get_time_ms();
function u64 os_get_time_freq();

// handles (implemented once)
function b8 os_handle_equals(os_handle_t handle_a, os_handle_t handle_b);

// events (implemented once)
function void os_events_clear();
function void os_events_push(os_event_t* event);
function os_event_t* os_events_pop(os_event_t* event);
function os_event_t* os_events_find(os_handle_t window, os_event_type type);
function b8 os_key_press(os_handle_t window, os_key key, os_modifier modifiers);
function b8 os_key_release(os_handle_t window, os_key key, os_modifier modifiers);
function b8 os_key_is_down(os_handle_t window, os_key key);
function b8 os_mouse_press(os_handle_t window, os_key key, os_modifier modifiers = 0);
function b8 os_mouse_release(os_handle_t window, os_key key, os_modifier modifiers = 0);
function b8 os_mouse_scroll(os_handle_t window, os_key key, os_modifier modifiers = 0);

// windows (implemented per backend) 
function os_handle_t os_window_open();
function void os_window_close(os_handle_t window);

function b8 os_window_is_focused(os_handle_t window);
function void os_window_set_focus(os_handle_t window, b8 focus);
function b8 os_window_is_minimized(os_handle_t window);
function void os_window_set_minimize(os_handle_t window, b8 minimize);
function b8 os_window_is_maxmized(os_handle_t window);
function void os_window_set_maxmimize(os_handle_t window, b8 maximize);
function b8 os_window_is_fullscnreen(os_handle_t window);
function void os_window_set_fullscreen(os_handle_t window, b8 fullscreen);

function void os_window_set_title(os_handle_t window, str_t title);

// files (implemented per backend)
function os_handle_t os_file_open();
function void os_file_close(os_handle_t file);


// file info
//function os_file_info_t os_file_get_info(os_handle_t file);
//function os_file_info_t os_file_get_info(str_t filepath);


//~ internal functions 

// core state (implemented once)
function void _os_init_core();
function void _os_release_core();

// events
function os_event_t* _os_event_alloc(os_event_type type);


#endif // SORA_OS_H