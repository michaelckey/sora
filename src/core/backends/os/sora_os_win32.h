// sora_os_win32.h

#ifndef SORA_OS_WIN32_H
#define SORA_OS_WIN32_H

//- includes

#pragma push_macro("function")
#undef function
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX
#include <windows.h>
#include <timeapi.h>
#include <dwmapi.h>
#pragma pop_macro("function")

//- defines

// define memory functions
#define os_mem_reserve(size) os_w32_mem_reserve(size);
#define os_mem_release(ptr, size) os_w32_mem_release(ptr, size);
#define os_mem_commit(ptr, size) os_w32_mem_commit(ptr, size);
#define os_mem_decommit(ptr, size) os_w32_mem_decommit(ptr, size);

// window procedure cases
#define WM_NCUAHDRAWCAPTION (0x00AE)
#define WM_NCUAHDRAWFRAME (0x00AF)

//- enums

enum os_w32_entity_type {
	os_w32_entity_type_null,
	os_w32_entity_type_thread,
	os_w32_entity_type_mutex,
	os_w32_entity_type_rw_mutex,
	os_w32_entity_type_condition_variable,
    os_w32_entity_type_fiber,
	os_w32_entity_type_file_iter,
};

//- structs

// entity
struct os_w32_entity_t {
	os_w32_entity_t* next;
    
	os_w32_entity_type type;
	union {
        
        // thread
		struct {
			HANDLE handle;
			DWORD tid;
			void* ptr;
            void* params;
			os_thread_function_t* func;
		} thread;
        
        // mutex
		CRITICAL_SECTION mutex;
        
        // rw_mutex
		SRWLOCK rw_mutex;
        
        // condition_variable
		CONDITION_VARIABLE cv;
        
        // fiber
        LPVOID fiber;
        
        // file iter
        struct {
            os_file_iter_flags flags;
            HANDLE handle;
            WIN32_FIND_DATAW find_data;
        } file_iter;
        
	};
    
};

// window
struct os_w32_window_t {
    
	// window list 
	os_w32_window_t* next;
	os_w32_window_t* prev;
    
	// flags
	os_window_flags flags;
    
	// win32
	HWND handle;
	
	// info
    str_t title;
	uvec2_t resolution;
	b8 borderless;
	b8 composition_enabled;
	b8 maximized;
	b8 is_moving;
    
	os_frame_function_t* frame_func;
    
	// sizing
	WINDOWPLACEMENT last_window_placement; // for fullscreen
    
	// custom title bar client area
	arena_t* title_bar_arena;
	os_title_bar_client_area_t* title_bar_client_area_first;
	os_title_bar_client_area_t* title_bar_client_area_last;
    
	// time
	LARGE_INTEGER tick_current;
	LARGE_INTEGER tick_previous;
	f64 delta_time;
	f64 elasped_time;
    
	// input state
	vec2_t mouse_pos;
	vec2_t mouse_pos_last;
	vec2_t mouse_delta;
};

// state
struct os_w32_state_t {
    
	// arenas
	arena_t* window_arena;
	arena_t* entity_arena;
	arena_t* event_list_arena;
	
    // system info
    os_system_info_t system_info;
    
	// entities
	os_w32_entity_t* entity_free;
	CRITICAL_SECTION entity_mutex;
    
	// window list
	os_w32_window_t* window_first;
	os_w32_window_t* window_last;
	os_w32_window_t* window_free;
    
	// event list
	os_event_list_t event_list;
    
	// time
	LARGE_INTEGER time_frequency;
	UINT blink_time;
	UINT double_click_time;
    
	// cursor
	HCURSOR cursors[os_cursor_count];
    
	b8 new_borderless_window = false; // TODO: not sure if needed
    
};

//- globals

global os_w32_state_t os_state;

//- win32 specific functions

// memory
function void* os_w32_mem_reserve(u64 size);
function void os_w32_mem_release(void* ptr, u64 size);
function void os_w32_mem_commit(void* ptr, u64 size);
function void os_w32_mem_decommit(void* ptr, u64 size);

// windows
function os_handle_t      os_w32_handle_from_window(os_w32_window_t* window);
function os_w32_window_t* os_w32_window_from_handle(os_handle_t window);
function os_w32_window_t* os_w32_window_from_hwnd(HWND window);
function HWND             os_w32_hwnd_from_window(os_w32_window_t* window);

// entities
function os_w32_entity_t* os_w32_entity_create(os_w32_entity_type type);
function void os_w32_entity_release(os_w32_entity_t* entity);

// files
function os_file_flags os_w32_file_flags_from_attributes(DWORD attributes);

// time
function u32 os_w32_sleep_ms_from_endt_us(u64 endt_us);

// thread entry point
function DWORD os_w32_thread_entry_point(void*);

// window procedure
LRESULT CALLBACK os_w32_window_procedure(HWND, UINT, WPARAM, LPARAM);

#endif // SORA_OS_WIN32_H