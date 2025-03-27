// sora_os.h

#ifndef SORA_OS_H
#define SORA_OS_H

// os layer todos:
// 
// [x] - reduce memory arenas used if possible.
// [x] - switch to handles for windows, threads, files, etcs.
// [x] - group os objects into entities.
// [ ] - redo window update system.
//

//- defines 

//- typedefs

typedef void os_thread_function_t(void*);
typedef void os_fiber_function_t(void*);
typedef void os_frame_function_t();

//- enums

enum os_key {
	os_key_null = 0,
	os_key_backspace = 8,
	os_key_tab = 9,
	os_key_enter = 13,
	os_key_shift = 16,
	os_key_ctrl = 17,
	os_key_alt = 18,
	os_key_caps = 20,
	os_key_esc = 27,
	os_key_space = 32,
	os_key_page_up = 33,
	os_key_page_down = 34,
	os_key_end = 35,
	os_key_home = 36,
	os_key_left = 37,
	os_key_up = 38,
	os_key_right = 39,
	os_key_down = 40,
	os_key_delete = 46,
	os_key_0 = 48,
	os_key_1 = 49,
	os_key_2 = 50,
	os_key_3 = 51,
	os_key_4 = 52,
	os_key_5 = 53,
	os_key_6 = 54,
	os_key_7 = 55,
	os_key_8 = 56,
	os_key_9 = 57,
	os_key_A = 65,
	os_key_B = 66,
	os_key_C = 67,
	os_key_D = 68,
	os_key_E = 69,
	os_key_F = 70,
	os_key_G = 71,
	os_key_H = 72,
	os_key_I = 73,
	os_key_J = 74,
	os_key_K = 75,
	os_key_L = 76,
	os_key_M = 77,
	os_key_N = 78,
	os_key_O = 79,
	os_key_P = 80,
	os_key_Q = 81,
	os_key_R = 82,
	os_key_S = 83,
	os_key_T = 84,
	os_key_U = 85,
	os_key_V = 86,
	os_key_W = 87,
	os_key_X = 88,
	os_key_Y = 89,
	os_key_Z = 90,
	os_key_equal = 187,
	os_key_comma = 188,
	os_key_minus = 189,
	os_key_period = 190,
	os_key_forward_slash = 191,
	os_key_left_bracket = 219,
	os_key_backslash = 220,
	os_key_right_bracket = 221,
	os_key_quote = 222,
	os_key_insert = 45,
	os_key_semicolon = 186,
	os_key_F1 = 112,
	os_key_F2 = 113,
	os_key_F3 = 114,
	os_key_F4 = 115,
	os_key_F5 = 116,
	os_key_F6 = 117,
	os_key_F7 = 118,
	os_key_F8 = 119,
	os_key_F9 = 120,
	os_key_F10 = 121,
	os_key_F11 = 122,
	os_key_F12 = 123,
	os_key_F13 = 124,
	os_key_F14 = 125,
	os_key_F15 = 126,
	os_key_F16 = 127,
	os_key_F17 = 128,
	os_key_F18 = 129,
	os_key_F19 = 130,
	os_key_F20 = 131,
	os_key_F21 = 132,
	os_key_F22 = 133,
	os_key_F23 = 134,
	os_key_F24 = 135,
	os_key_grave_accent = 192,
};

enum os_mouse_button {
	os_mouse_button_left,
	os_mouse_button_middle,
	os_mouse_button_right,
	os_mouse_button_count,
};

typedef u32 os_modifiers;
enum {
    os_modifier_none = 0,
	os_modifier_shift = (1 << 0),
	os_modifier_ctrl = (1 << 1),
	os_modifier_alt = (1 << 2),
    os_modifier_any = (1 << 31),
};

enum os_event_type {
	os_event_type_null,
	os_event_type_window_close,
	os_event_type_window_resize,
	os_event_type_key_press,
	os_event_type_key_release,
	os_event_type_text,
	os_event_type_mouse_press,
	os_event_type_mouse_release,
	os_event_type_mouse_scroll,
	os_event_type_mouse_move,
};

typedef u32 os_file_access_flag;
enum {
	os_file_access_flag_none = (0),
	os_file_access_flag_read = (1 << 0),
	os_file_access_flag_write = (1 << 1),
	os_file_access_flag_execute = (1 << 2),
	os_file_access_flag_append = (1 << 3),
	os_file_access_flag_share_read = (1 << 4),
	os_file_access_flag_share_write = (1 << 5),
	os_file_access_flag_attribute = (1 << 6),
};

enum os_cursor {
	os_cursor_null,
	os_cursor_pointer,
	os_cursor_I_beam,
	os_cursor_resize_EW,
	os_cursor_resize_NS,
	os_cursor_resize_NWSE,
	os_cursor_resize_NESW,
	os_cursor_resize_all,
	os_cursor_hand_point,
	os_cursor_disabled,
	os_cursor_count,
};

typedef u32 os_window_flags;
enum {
	os_window_flag_null,
	os_window_flag_borderless = (1 << 0),
};

typedef i32 os_sys_color;
enum {
	os_sys_color_none,
	os_sys_color_accent,
	os_sys_color_inactive_border,
};

typedef u32 os_file_flags;
enum {
    os_file_flag_is_read_only = (1 << 0),
    os_file_flag_is_hidden = (1 << 1),
    os_file_flag_is_folder = (1 << 2),
};

typedef u32 os_file_iter_flags;
enum {
    os_file_iter_flag_skip_folders = (1 << 0),
    os_file_iter_flag_skip_files = (1 << 1),
    os_file_iter_flag_skip_hidden_files = (1 << 2),
    os_file_iter_flag_recursive = (1 << 3),
    os_file_iter_flag_done = (1 << 31),
};

//- structs

struct os_handle_t {
	u64 data[1];
};

struct os_title_bar_client_area_t {
	os_title_bar_client_area_t* next;
	os_title_bar_client_area_t* prev;
	rect_t area;
};

struct os_event_t {
    
	// event list
	os_event_t* next;
	os_event_t* prev;
    
	os_event_type type;
	os_handle_t window;
	os_key key;
	os_mouse_button mouse;
	os_modifiers modifiers;
	u32 character;
	vec2_t position;
	vec2_t scroll;
};

struct os_event_list_t {
	os_event_t* first;
	os_event_t* last;
	u32 count;
};

struct os_file_info_t {
    str_t name;
    os_file_flags flags;
	u64 size;
    u64 creation_time;
    u64 last_access_time;
	u64 last_write_time;
};

struct os_system_info_t {
	u32 logical_processor_count;
};

//- functions

// state (implemented per backends)
function void             os_init();
function void             os_release();
function void             os_update();
function void             os_abort(u32 exit_code);
function void             os_sleep(u32 ms);
function u64              os_time_microseconds();
function b8               os_any_window_exist();
function void             os_set_cursor(os_cursor cursor);
function os_system_info_t os_get_system_info();
function u32              os_get_thread_id();

function os_modifiers os_get_modifiers();
function b8           os_key_is_down(os_key key);
function b8           os_mouse_is_down(os_mouse_button button);

// handle
function b8           os_handle_equals(os_handle_t a, os_handle_t b);

// events
function void         os_event_push(os_event_t* event);
function void         os_event_pop(os_event_t* event);
function os_event_t*  os_event_get(os_event_type type);
function b8           os_key_press(os_handle_t window, os_key key, os_modifiers modifiers = os_modifier_any);
function b8           os_key_release(os_handle_t window, os_key key, os_modifiers modifiers = os_modifier_any);
function b8           os_mouse_press(os_handle_t window, os_mouse_button button, os_modifiers modifiers = os_modifier_any);
function b8           os_mouse_release(os_handle_t window, os_mouse_button button, os_modifiers modifiers = os_modifier_any);
function f32          os_mouse_scroll(os_handle_t window);
function vec2_t       os_mouse_move(os_handle_t window);
function b8           os_mouse_button_is_down(os_mouse_button button);

// window (implemented per backend)
function os_handle_t  os_window_open(str_t title, u32 width, u32 height, os_window_flags flags = 0);
function void         os_window_close(os_handle_t window);
function void         os_window_update(os_handle_t window);
function void         os_window_focus(os_handle_t window);
function void         os_window_minimize(os_handle_t window);
function void         os_window_maximize(os_handle_t window);
function void         os_window_restore(os_handle_t window);
function void         os_window_fullscreen(os_handle_t window);
function void         os_window_set_title(os_handle_t window, str_t title);
function str_t        os_window_get_title(os_handle_t window);
function u32          os_window_get_dpi(os_handle_t window);

function b8           os_window_is_maximized(os_handle_t window);
function b8           os_window_is_minimized(os_handle_t window);
function b8           os_window_is_fullscreen(os_handle_t window);
function b8           os_window_is_active(os_handle_t window);

function void         os_window_clear_title_bar_client_area(os_handle_t window);
function void         os_window_add_title_bar_client_area(os_handle_t window, rect_t area);

function void         os_window_set_frame_function(os_handle_t window, os_frame_function_t* func);

function vec2_t       os_window_get_cursor_pos(os_handle_t window);
function void         os_window_set_cursor_pos(os_handle_t window, vec2_t pos);

function uvec2_t      os_window_get_size(os_handle_t window);
function f32          os_window_get_delta_time(os_handle_t window);
function f32          os_window_get_elapsed_time(os_handle_t window);
function vec2_t       os_window_get_mouse_delta(os_handle_t window);

// graphical messages (implemented per backend)
function void         os_graphical_message(b8 error, str_t title, str_t msg);

// memory (implemented per backend)
function u64          os_page_size();

// file (implemented per backend)
function os_handle_t  os_file_open(str_t filepath, os_file_access_flag access_flag = os_file_access_flag_read | os_file_access_flag_share_read | os_file_access_flag_share_write);
function void         os_file_close(os_handle_t file);
function str_t        os_file_read_range(arena_t* arena, os_handle_t file, u32 min, u32 max);
function str_t        os_file_read_all(arena_t* arena, str_t filepath);
function str_t        os_file_read_all(arena_t* arena, os_handle_t file);

// file info (implemented per backend)
function os_file_info_t os_file_get_info(os_handle_t file);
function os_file_info_t os_file_get_info(str_t filepath);

// file iterator (implemented per backend)
function os_handle_t  os_file_iter_begin(str_t filepath, os_file_iter_flags flags = 0);
function void         os_file_iter_end(os_handle_t iter);
function b8           os_file_iter_next(arena_t* arena, os_handle_t iter, os_file_info_t* file_info);

// threads (implemented per backend)
function os_handle_t  os_thread_create(os_thread_function_t* thread_func, void* params = nullptr);
function b8           os_thread_join(os_handle_t thread, u64 endt_us = u64_max);
function void         os_thread_detach(os_handle_t thread);
function void         os_thread_set_name(os_handle_t thread, str_t name);

// mutexes (implemented per backend)
function os_handle_t  os_mutex_create();
function void         os_mutex_release(os_handle_t mutex);
function void         os_mutex_lock(os_handle_t mutex);
function void         os_mutex_unlock(os_handle_t mutex);

// rw mutexes (implemented per backend)
function os_handle_t  os_rw_mutex_create();
function void         os_rw_mutex_release(os_handle_t rw_mutex);
function void         os_rw_mutex_lock_r(os_handle_t rw_mutex);
function void         os_rw_mutex_unlock_r(os_handle_t rw_mutex);
function void         os_rw_mutex_lock_w(os_handle_t rw_mutex);
function void         os_rw_mutex_unlock_w(os_handle_t rw_mutex);

// condition variable (implemented per backend)
function os_handle_t  os_condition_variable_create();
function void         os_condition_variable_release(os_handle_t cv);
function b8           os_condition_variable_wait(os_handle_t cv, os_handle_t mutex, u64 endt_us);
function b8           os_condition_variable_wait_rw_r(os_handle_t cv, os_handle_t rw_mutex, u64 endt_us);
function b8           os_condition_variable_wait_rw_w(os_handle_t cv, os_handle_t rw_mutex, u64 endt_us);
function void         os_condition_variable_signal(os_handle_t cv);
function void         os_condition_variable_broadcast(os_handle_t cv);

// fiber (implemented per backend)
function os_handle_t  os_fiber_create(u32 stack_size, os_fiber_function_t* fiber_func, void* params = nullptr);
function void         os_fiber_release(os_handle_t fiber);
function void         os_fiber_switch(os_handle_t fiber);
function os_handle_t  os_fiber_from_thread();

//- backend includes

#if defined(OS_BACKEND_WIN32)
#include "backends/os/sora_os_win32.h"
#elif defined(OS_BACKEND_LINUX)
#include "backends/os/sora_os_linux.h"
#elif defined(OS_BACKEND_MACOS)
#include "backends/os/sora_os_macos.h"
#else 
#error undefined os backend.
#endif 

#endif // SORA_OS_H