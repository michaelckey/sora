// sora_os_win32.h

#ifndef SORA_OS_WIN32_H
#define SORA_OS_WIN32_H

//~ includes 

#pragma push_macro("function") // NOTE: my define for function messes up win32
#undef function
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX
#include <windows.h>
#include <timeapi.h>
#include <dwmapi.h> // for custom window borders
#include <winternl.h> // for getting the version
#include <tlhelp32.h> // for getting parent process id
#include <psapi.h> // for memory usage
#pragma pop_macro("function")

//~ defines 

// memory functions
#define os_mem_reserve(size) os_w32_mem_reserve(size)
#define os_mem_release(ptr, size) os_w32_mem_release(ptr, size)
#define os_mem_commit(ptr, size) os_w32_mem_commit(ptr, size)
#define os_mem_decommit(ptr, size) os_w32_mem_decommit(ptr, size)

// extra win32 procedure cases
#define WM_NCUAHDRAWCAPTION (0x00AE)
#define WM_NCUAHDRAWFRAME (0x00AF)

// keymap
#define os_w32_key_mappings \
os_w32_key_map(VK_LSHIFT, os_key_left_shift)\
os_w32_key_map(VK_RSHIFT, os_key_right_shift)\
os_w32_key_map(VK_LCONTROL, os_key_left_ctrl)\
os_w32_key_map(VK_RCONTROL, os_key_right_ctrl)\
os_w32_key_map(VK_LMENU, os_key_left_alt)\
os_w32_key_map(VK_RMENU, os_key_right_alt)\
os_w32_key_map(VK_LWIN, os_key_left_super)\
os_w32_key_map(VK_RWIN, os_key_right_super)\
os_w32_key_map(VK_ESCAPE, os_key_esc)\
os_w32_key_map(VK_F1, os_key_f1)\
os_w32_key_map(VK_F2, os_key_f2)\
os_w32_key_map(VK_F3, os_key_f3)\
os_w32_key_map(VK_F4, os_key_f4)\
os_w32_key_map(VK_F5, os_key_f5)\
os_w32_key_map(VK_F6, os_key_f6)\
os_w32_key_map(VK_F7, os_key_f7)\
os_w32_key_map(VK_F8, os_key_f8)\
os_w32_key_map(VK_F9, os_key_f9)\
os_w32_key_map(VK_F10, os_key_f10)\
os_w32_key_map(VK_F11, os_key_f11)\
os_w32_key_map(VK_F12, os_key_f12)\
os_w32_key_map(VK_UP, os_key_up)\
os_w32_key_map(VK_DOWN, os_key_down)\
os_w32_key_map(VK_LEFT, os_key_left)\
os_w32_key_map(VK_RIGHT, os_key_right)\
os_w32_key_map(VK_HOME, os_key_home)\
os_w32_key_map(VK_END, os_key_end)\
os_w32_key_map(VK_PRIOR, os_key_page_up)\
os_w32_key_map(VK_NEXT, os_key_page_down)\
os_w32_key_map(VK_INSERT, os_key_insert)\
os_w32_key_map(VK_DELETE, os_key_delete)\
os_w32_key_map(VK_BACK, os_key_backspace)\
os_w32_key_map(VK_TAB, os_key_tab)\
os_w32_key_map(VK_RETURN, os_key_enter)\
os_w32_key_map('A', os_key_a)\
os_w32_key_map('B', os_key_b)\
os_w32_key_map('C', os_key_c)\
os_w32_key_map('D', os_key_d)\
os_w32_key_map('E', os_key_e)\
os_w32_key_map('F', os_key_f)\
os_w32_key_map('G', os_key_g)\
os_w32_key_map('H', os_key_h)\
os_w32_key_map('I', os_key_i)\
os_w32_key_map('J', os_key_j)\
os_w32_key_map('K', os_key_k)\
os_w32_key_map('L', os_key_l)\
os_w32_key_map('M', os_key_m)\
os_w32_key_map('N', os_key_n)\
os_w32_key_map('O', os_key_o)\
os_w32_key_map('P', os_key_p)\
os_w32_key_map('Q', os_key_q)\
os_w32_key_map('R', os_key_r)\
os_w32_key_map('S', os_key_s)\
os_w32_key_map('T', os_key_t)\
os_w32_key_map('U', os_key_u)\
os_w32_key_map('V', os_key_v)\
os_w32_key_map('W', os_key_w)\
os_w32_key_map('X', os_key_x)\
os_w32_key_map('Y', os_key_y)\
os_w32_key_map('Z', os_key_z)\
os_w32_key_map('0', os_key_0)\
os_w32_key_map('1', os_key_1)\
os_w32_key_map('2', os_key_2)\
os_w32_key_map('3', os_key_3)\
os_w32_key_map('4', os_key_4)\
os_w32_key_map('5', os_key_5)\
os_w32_key_map('6', os_key_6)\
os_w32_key_map('7', os_key_7)\
os_w32_key_map('8', os_key_8)\
os_w32_key_map('9', os_key_9)\
os_w32_key_map(VK_SPACE, os_key_space)\
os_w32_key_map(VK_OEM_MINUS, os_key_minus)\
os_w32_key_map(VK_OEM_PLUS, os_key_equals)\
os_w32_key_map(VK_OEM_4, os_key_left_bracket)\
os_w32_key_map(VK_OEM_6, os_key_right_bracket)\
os_w32_key_map(VK_OEM_5, os_key_backslash)\
os_w32_key_map(VK_OEM_1, os_key_semicolon)\
os_w32_key_map(VK_OEM_7, os_key_apostrophe)\
os_w32_key_map(VK_OEM_3, os_key_grave)\
os_w32_key_map(VK_OEM_COMMA, os_key_comma)\
os_w32_key_map(VK_OEM_PERIOD, os_key_period)\
os_w32_key_map(VK_OEM_2, os_key_slash)\
os_w32_key_map(VK_NUMPAD0, os_key_num_0)\
os_w32_key_map(VK_NUMPAD1, os_key_num_1)\
os_w32_key_map(VK_NUMPAD2, os_key_num_2)\
os_w32_key_map(VK_NUMPAD3, os_key_num_3)\
os_w32_key_map(VK_NUMPAD4, os_key_num_4)\
os_w32_key_map(VK_NUMPAD5, os_key_num_5)\
os_w32_key_map(VK_NUMPAD6, os_key_num_6)\
os_w32_key_map(VK_NUMPAD7, os_key_num_7)\
os_w32_key_map(VK_NUMPAD8, os_key_num_8)\
os_w32_key_map(VK_NUMPAD9, os_key_num_9)\
os_w32_key_map(VK_ADD, os_key_num_add)\
os_w32_key_map(VK_SUBTRACT, os_key_num_sub)\
os_w32_key_map(VK_MULTIPLY, os_key_num_mul)\
os_w32_key_map(VK_DIVIDE, os_key_num_div)\
os_w32_key_map(VK_DECIMAL, os_key_num_dec)\
os_w32_key_map(VK_CAPITAL, os_key_caps_lock)\
os_w32_key_map(VK_SCROLL, os_key_scroll_lock)\
os_w32_key_map(VK_NUMLOCK, os_key_num_lock)\
os_w32_key_map(VK_SNAPSHOT, os_key_print_screen)\
os_w32_key_map(VK_PAUSE, os_key_pause)

//~ win32 specifics 

typedef NTSTATUS (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW); // for windows versioning. 

// for transparent windwo blur

enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5
};

struct ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor; // AABBGGRR format (alpha first)
    DWORD AnimationId;
};

struct WINCOMPATTR_DATA {
    DWORD Attribute;
    PVOID Data;
    SIZE_T SizeOfData;
};

#define WCA_ACCENT_POLICY 19

//~ enums

enum os_w32_entity_type {
    os_w32_entity_null,
    os_w32_entity_thread,
    os_w32_entity_mutex,
    os_w32_entity_rw_mutex,
    os_w32_entity_condition_variable,
    os_w32_entity_semaphore,
    os_w32_entity_fiber,
    os_w32_entity_file_iter,
    os_w32_entity_count,
};

enum os_w32_window_mode {
    os_w32_window_mode_normal,
    os_w32_window_mode_minimized,
    os_w32_window_mode_maximized,
    os_w32_window_mode_fullscreen,
};

//~ structs 

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
			os_thread_func* func;
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

struct os_w32_window_t {
    
    os_w32_window_t* next;
    os_w32_window_t* prev;
    
    // state
    os_window_flags flags;
    os_w32_window_mode mode;
    b8 focused;
    
    str_t title;
    uvec2_t position;
    uvec2_t size;
    WINDOWPLACEMENT last_window_placement;
    uvec2_t mouse_position;
    
    HWND handle;
    os_resize_callback_func* resize_callback_func;
    
};

struct os_w32_state_t {
    
    arena_t* arena;
    HINSTANCE hinstance;
    
    // window list
    os_w32_window_t* window_first;
    os_w32_window_t* window_last;
    os_w32_window_t* window_free;
    
    // entity list
    os_w32_entity_t* entity_free;
    CRITICAL_SECTION entity_mutex;
    
    b8 new_borderless_window;
    
};

//~ globals 

global os_w32_state_t os_w32_state;

//~ win32 specific functions

// keys
function os_modifiers os_w32_get_modifiers();
function os_key os_w32_os_key_from_vkey(WPARAM vkey);
function WPARAM os_w32_vkey_from_os_key(os_key key);

// time 
function DWORD os_w32_sleep_ms_from_timeout_us(u64 timeout_us);

// memory
function void* os_w32_mem_reserve(u64 size);
function void os_w32_mem_release(void* ptr, u64 size);
function b8   os_w32_mem_commit(void* ptr, u64 size);
function void os_w32_mem_decommit(void* ptr, u64 size);

// entities
function os_w32_entity_t* os_w32_entity_alloc(os_w32_entity_type type);
function void os_w32_entity_release(os_w32_entity_t* entity);

// windows
function os_window_t os_window_handle_from_w32_window(os_w32_window_t* window);
function os_w32_window_t* os_w32_window_from_window_handle(os_window_t window_handle);
function os_w32_window_t* os_w32_window_from_hwnd(HWND handle);

function void os_w32_window_enable_acrylic_blur(HWND handle, BYTE opacity, COLORREF col);

// file
function os_file_flags os_w32_file_flags_from_attributes(DWORD attributes);

// entry points
function DWORD os_w32_thread_entry_point(void* ptr);

// window procedure
LRESULT CALLBACK os_w32_window_procedure(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);

#endif // SORA_OS_WIN32_H
