// sora_os.h

#ifndef SORA_OS_H
#define SORA_OS_H

// TODO:
// 
// [ ] - pipes
// [ ] - shared memory
// [ ] - file maps
// [ ] - date times
// [ ] - monitors
// [ ] - 


//~ typedefs

typedef i32 os_thread_func(void*);
typedef void os_fiber_func(void*);
typedef void os_resize_callback_func();

//~ enums 

enum os_cursor {
    os_cursor_arrow,
    os_cursor_i_beam,
    os_cursor_wait,
    os_cursor_crosshair,
    os_cursor_wait_arrow,
    os_cursor_size_we,
    os_cursor_size_ns,
    os_cursor_size_nwse,
    os_cursor_size_nesw,
    os_cursor_size_all,
    os_cursor_hand,
    os_cursor_disabled,
    os_cursor_count,
};

enum os_key {
    
    os_key_null,
    
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

enum os_gamepad_button {
    
    os_gamepad_button_null,
    
    // face                xbox  |  playstation
    os_gamepad_button_a, //  a   |    cross
    os_gamepad_button_b, //  b   |    circle
    os_gamepad_button_x, //  x   |    square
    os_gamepad_button_y, //  y   |    triangle
    
    os_gamepad_button_cross = os_gamepad_button_a,
    os_gamepad_button_circle = os_gamepad_button_b,
    os_gamepad_button_square = os_gamepad_button_x,
    os_gamepad_button_triangle = os_gamepad_button_y,
    
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


typedef u32 os_modifiers;
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
    os_event_window_restore,
    os_event_window_maximize,
    os_event_window_fullscreen,
    os_event_window_paint,
    
    // keyboard events
    os_event_key_press,
    os_event_key_release,
    os_event_key_text,
    
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

typedef u32 os_window_flags;
enum {
    os_window_flag_none = 0,
    os_window_flag_minimized = (1 << 0),
    os_window_flag_maximized = (1 << 1),
    os_window_flag_fullscreen = (1 << 2),
    os_window_flag_borderless = (1 << 3),
    os_window_flag_always_on_top = (1 << 4),
    os_window_flag_transparent = (1 << 5),
};

typedef u32 os_file_flags;
enum {
    os_file_flag_none = (0),
    os_file_flag_is_read_only = (1 << 0),
    os_file_flag_is_hidden = (1 << 1),
    os_file_flag_is_directory = (1 << 2),
};

typedef u32 os_file_access_flags;
enum {
    os_file_access_flag_none = 0,
    os_file_access_flag_read = (1 << 0),
    os_file_access_flag_write = (1 << 1),
    os_file_access_flag_append = (1 << 2),
    os_file_access_flag_execute = (1 << 3),
    os_file_access_flag_shared_read = (1 << 4),
    os_file_access_flag_shared_write = (1 << 5),
    os_file_access_flag_attribute = (1 << 6),
};

typedef u32 os_file_iter_flags;
enum {
    os_file_iter_flag_none = 0,
    os_file_iter_flag_skip_directories = (1 << 0),
    os_file_iter_flag_skip_files = (1 << 1),
    os_file_iter_flag_skip_hidden_files = (1 << 2),
    os_file_iter_flag_recursive = (1 << 3),
    os_file_iter_flag_done = (1 << 4),
};

enum os_thread_priority {
    os_thread_priority_idle,
    os_thread_priority_low,
    os_thread_priority_normal,
    os_thread_priority_high,
    os_thread_priority_time_critical,
};

enum os_socket_type {
    os_socket_type_tcp,
    os_socket_type_udp,
};


//~ structs 

//- handles

struct os_window_t { u64 id; };

struct os_file_t { u64 id; };
struct os_file_iter_t { u64 id; };

struct os_process_t { u64 id; };

struct os_thread_t { u64 id; };
struct os_fiber_t { u64 id; };

struct os_mutex_t { u64 id; };
struct os_rw_mutex_t { u64 id; };
struct os_condition_variable_t { u64 id; };
struct os_semaphore_t { u64 id; };

// NOTE: these will be implemented later.
struct os_pipe_t { u64 id; };
struct os_timer_t { u64 id; };
struct os_socket_t { u64 id; };

//- events

struct os_event_t {
    os_event_t* next;
    os_event_t* prev;
    
    os_event_type type;
    
    u64 frame_count;
    u64 timestamp;
    os_modifiers modifiers;
    
    // window events
    os_window_t window;
    uvec2_t window_position;
    uvec2_t window_size;
    
    // keyboard events
    os_key key; // for 'os_event_key_press/release'
    u32 keycode; // for 'os_event_key_text'
    str_t string;
    
    // mouse events
    os_mouse_button mouse_button;
    vec2_t mouse_position; 
    vec2_t mouse_scroll;
    
};

//- info structs

struct os_system_info_t {
    
    // system
    char host_name[64];
    char user_name[64];
    
    // platform
    char platform_name[64]; // 'Windows', 'macOS', 'Linux', etc.
    char platform_version[64];
    
    // cpu
    char cpu_vendor[64]; // 'Intel', 'Apple', etc.
    char cpu_name[64]; // 'Intel(R) Core(TM) i7-8650 CPU @ 1.9GHz', etc.
    char cpu_architecture[64]; // 'x86_64', 'ARM64', etc.
    u32 cpu_logical_core_count;
    u32 cpu_physical_core_count;
    u32 cpu_cache_line_size;
    
    // memory
    u32 memory_page_size;
    u32 memory_allocation_granularity; // NOTE: mostly for Windows.
    u64 memory_available;
    u64 memory_total;
    u64 virtual_memory_available; // NOTE: if supported.
    u64 virtual_memory_total; // NOTE: if supported.
    
};

struct os_process_info_t {
    
    u32 process_id;
    u32 parent_process_id;
    u64 start_timestamp;
    
    char exe_name[512];
    char exe_path[512];
    char working_directory[512];
    
    // usage
    f64 cpu_usage;
    u64 memory_usage;
    u32 thread_count;
};

struct os_file_info_t {
    u64 size; // in bytes
    os_file_flags flags;
    
    u64 creation_time;
    u64 last_modified_time;
    u64 last_accessed_time;
};

//- core state

struct os_core_state_t {
    
    arena_t* arena;
    
    // events
    arena_t* events_arena;
    os_event_t* event_first;
    os_event_t* event_last;
    os_event_t* event_free;
    
    // info
    os_system_info_t system_info;
    os_process_info_t process_info;
    
    // input state
    b8 keys[os_key_count];
    b8 mouse_buttons[os_mouse_button_count];
    b8 gamepad_buttons[os_gamepad_button_count];
    
    // timing
    u64 time_frequency;
    
};

//~ globals

global os_core_state_t os_core_state;
thread_global os_thread_t os_current_thread;

//~ functions 

//- state (implemented per backend)

function void                    os_init();
function void                    os_release();
function void                    os_poll_events(); // NOTE: clears event list before polling.
function void                    os_abort(u32 exit_code);
function void                    os_sleep(u32 ms);

//- info 

function os_system_info_t        os_get_system_info();
function os_process_info_t       os_get_process_info(os_process_t process_handle);

//- timing (implemented per backend)

function u64                     os_get_time_ms();
function u64                     os_get_time_freq();

//- memory (implemented per backend)

function u64                     os_page_size();

//- handles (implemented once)

function b8                      os_window_equals(os_window_t window_handle_a, os_window_t window_handle_b);
function b8                      os_file_equals(os_file_t file_handle_a, os_file_t file_handle_b);
function b8                      os_process_equals(os_process_t process_handle_a, os_process_t process_handle_b);
function b8                      os_thread_equals(os_thread_t thread_handle_a, os_thread_t thread_handle_b);
function b8                      os_fiber_equals(os_fiber_t fiber_handle_a, os_fiber_t fiber_handle_b);
function b8                      os_mutex_equals(os_mutex_t mutex_handle_a, os_mutex_t mutex_handle_b);
function b8                      os_rw_mutex_equals(os_rw_mutex_t rw_mutex_handle_a, os_rw_mutex_t rw_mutex_handle_b);
function b8                      os_condition_variable_equals(os_condition_variable_t condition_variable_handle_a, os_condition_variable_t condition_variable_handle_b);
function b8                      os_semaphore_equals(os_semaphore_t semaphore_handle_a, os_semaphore_t semaphore_handle_b);
function b8                      os_socket_equals(os_socket_t socket_handle_a, os_socket_t socket_handle_b);

//- events (implemented once)

function void                    os_event_release(os_event_t* event);
function os_event_t*             os_events_push(os_window_t window, os_event_type type);
function os_event_t*             os_events_peek();
function os_event_t*             os_events_pop();
function void                    os_events_remove(os_event_t* event);
function os_event_t*             os_events_find(os_window_t window_handle, os_event_type type);
function os_event_t*             os_events_find_last(os_window_t window_handle, os_event_type type);
function void                    os_events_clear();

//- event queries (implemented once)

function b8                      os_modifiers_equal(os_modifiers a, os_modifiers b);
function b8                      os_key_press(os_window_t window_handle, os_key key, os_modifiers modifiers = os_modifier_any);
function b8                      os_key_release(os_window_t window_handle, os_key key, os_modifiers modifiers = os_modifier_any);
function u32                     os_key_text(os_window_t window_handle);
function b8                      os_key_is_down(os_key key);

function b8                      os_mouse_move(os_window_t window_handle);
function b8                      os_mouse_enter(os_window_t window_handle);
function b8                      os_mouse_leave(os_window_t window_handle);
function b8                      os_mouse_press(os_window_t window_handle, os_mouse_button mouse, os_modifiers modifiers = os_modifier_any);
function b8                      os_mouse_release(os_window_t window_handle, os_mouse_button mouse, os_modifiers modifiers = os_modifier_any);
function vec2_t                  os_mouse_scroll(os_window_t window_handle, os_modifiers modifiers = os_modifier_any);
function b8                      os_mouse_is_down(os_mouse_button mouse);

function b8                      os_drag_enter(os_window_t window_handle);
function b8                      os_drag_leave(os_window_t window_handle);
function b8                      os_drag_drop(os_window_t window_handle); 

//- windows (implemented per backend) 

function os_window_t             os_window_open(str_t title, u32 width, u32 height, os_window_flags window_flags = 0);
function void                    os_window_close(os_window_t window_handle);

function void                    os_window_set_resize_callback(os_window_t window_handle, os_resize_callback_func* callback_func);

function b8                      os_window_is_focused(os_window_t window_handle);
function b8                      os_window_is_minimized(os_window_t window_handle);
function b8                      os_window_is_maxmized(os_window_t window_handle);
function b8                      os_window_is_fullscnreen(os_window_t window_handle);
function str_t                   os_window_get_title(os_window_t window_handle);
function uvec2_t                 os_window_get_position(os_window_t window_handle);
function uvec2_t                 os_window_get_size(os_window_t window_handle);
function uvec2_t                 os_window_get_mouse_position(os_window_t window_handle);
function f32                     os_window_get_dpi(os_window_t window_handle);

function b8                      os_window_set_focus(os_window_t window_handle, b8 focus);
function b8                      os_window_set_minimize(os_window_t window_handle, b8 minimize);
function b8                      os_window_set_maximize(os_window_t window_handle, b8 maximize);
function b8                      os_window_set_fullscreen(os_window_t window_handle, b8 fullscreen);
function b8                      os_window_set_title(os_window_t window_handle, str_t title);
function b8                      os_window_set_position(os_window_t window_handle, uvec2_t position);
function b8                      os_window_set_size(os_window_t window_handle, uvec2_t size);
function b8                      os_window_set_mouse_position(os_window_t window_handle, uvec2_t position);

//- files (implemented per backend)

function os_file_t               os_file_open(str_t filepath, os_file_access_flags flags = os_file_access_flag_read | os_file_access_flag_shared_read);
function b8                      os_file_close(os_file_t file);

function str_t                   os_file_read_range(arena_t* arena, os_file_t file, u32 start, u32 end);
function str_t                   os_file_read_all(arena_t* arena, os_file_t file);
function str_t                   os_file_read_all(arena_t* arena, str_t filepath);
function u32                     os_file_read(os_file_t file_handle, u32 index, void* out_data, u32 size);
function u32                     os_file_write(os_file_t file, u32 index, void* data, u32 size);

function b8                      os_file_copy(str_t src_filepath, str_t dst_filepath);
function b8                      os_file_move(str_t filepath, str_t new_filepath);
function b8                      os_file_rename(str_t filepath, str_t new_filepath);
function b8                      os_file_delete(str_t filepath);

// file info
function b8                      os_file_exists(str_t filepath);
function os_file_info_t          os_file_get_info(os_file_t file_handle);
function os_file_info_t          os_file_get_info(str_t filepath);

//- processes (implemented per backend)

function os_process_t            os_process_create(str_t executable_path); // TODO: figure out good parameters.
function b8                      os_process_join(os_process_t process_handle, u64 timeout_us = u64_max);
function b8                      os_process_detach(os_process_t process_handle);
function b8                      os_process_kill(os_process_t process_handle, u32 exit_code = 0);

function os_process_t            os_process_get_current();
function u64                     os_process_get_current_id();
function u64                     os_process_get_id(os_process_t process_handle);

// TODO: we probably want more functions like 
// setting the working directories and env vars. 
// we also want to implement pipes.

//- threads (implemented per backend)

function os_thread_t             os_thread_create(os_thread_func* thread_func, void* params = nullptr, os_thread_priority priority = os_thread_priority_normal);
function b8                      os_thread_join(os_thread_t thread_handle, u64 timeout_us = u64_max);
function b8                      os_thread_detach(os_thread_t thread_handle);
function void                    os_thread_yield();

function os_thread_t             os_thread_get_current();
function u64                     os_thread_get_current_id();
function u64                     os_thread_get_id(os_thread_t* thread_handle);

function b8                      os_thread_set_name(os_thread_t thread_handle, str_t name);
function b8                      os_thread_set_priority(os_thread_t thread_handle, os_thread_priority priority);

//- fibers (implemented per backend)

function os_fiber_t              os_fiber_create(os_fiber_func* fiber_func, void* params = nullptr, u32 stack_size = 0);
function os_fiber_t              os_fiber_from_thread();
function void                    os_fiber_release(os_fiber_t fiber_handle);
function void                    os_fiber_switch(os_fiber_t fiber_handle);

//- mutexes (implemented per backend)

function os_mutex_t              os_mutex_create();
function void                    os_mutex_release(os_mutex_t mutex_handle);
function void                    os_mutex_lock(os_mutex_t mutex_handle);
function void                    os_mutex_unlock(os_mutex_t mutex_handle);

//- rw_mutexes/shared_mutex (implemented per backend)

function os_rw_mutex_t           os_rw_mutex_create();
function void                    os_rw_mutex_release(os_rw_mutex_t rw_mutex_handle);
function void                    os_rw_mutex_lock_r(os_rw_mutex_t rw_mutex_handle);
function void                    os_rw_mutex_unlock_r(os_rw_mutex_t rw_mutex_handle);
function void                    os_rw_mutex_lock_w(os_rw_mutex_t rw_mutex_handle);
function void                    os_rw_mutex_unlock_w(os_rw_mutex_t rw_mutex_handle);

//- condition variables

function os_condition_variable_t os_condition_variable_create();
function void                    os_condition_variable_release(os_condition_variable_t condition_variable_handle);
function b8                      os_condition_variable_wait(os_condition_variable_t condition_variable_handle, os_mutex_t mutex_handle, u64 timeout_us = u64_max);
function b8                      os_condition_variable_wait_rw_r(os_condition_variable_t condition_variable_handle, os_mutex_t rw_mutex_handle, u64 timeout_us = u64_max);
function b8                      os_condition_variable_wait_rw_w(os_condition_variable_t condition_variable_handle, os_mutex_t rw_mutex_handle, u64 timeout_us = u64_max);
function void                    os_condition_variable_signal(os_condition_variable_t condition_variable_handle);
function void                    os_condition_variable_broadcast(os_condition_variable_t condition_variable_handle);

//- semaphores (implemented per backend)

function os_semaphore_t          os_semaphore_create(u32 initial_count, u32 max_count, str_t name);
function os_semaphore_t          os_semaphore_open(str_t name);
function void                    os_semaphore_release(os_semaphore_t semaphore_handle);
function void                    os_semaphore_close(os_semaphore_t semaphore_handle); 
function b8                      os_semaphore_wait(os_semaphore_t semaphore_handle, u64 timeout_us = u64_max);
function void                    os_semaphore_signal(os_semaphore_t semaphore_handle);

//- sockets (implemented per backend)

function os_socket_t             os_socket_create(os_socket_type socket_type);
function void                    os_socket_release(os_socket_t socket_handle);
function void                    os_socket_bind(os_socket_t socket_handle, str_t address, u16 port);

function void                    os_socket_accept(os_socket_t socket_handle, str_t client_address, u16 client_port);
function void                    os_socket_send(os_socket_t socket_handle);
function void                    os_socket_recv(os_socket_t socket_handle);
function void                    os_socket_send_to(os_socket_t socket_handle);
function void                    os_socket_recv_from(os_socket_t socket_handle);

//~ internal functions (implemented once) 

//- core state

function void                    _os_init_core();
function void                    _os_release_core();

//~ backend includes

#if OS_BACKEND_WIN32
#    include "backends/os/sora_os_win32.h"
#elif OS_BACKEND_LINUX
#    error os backend not implemented.
#elif OS_BACKEND_MACOS
#    error os backend not implemented.
#else
#    error undefined os backend.
#endif 

#endif // SORA_OS_H