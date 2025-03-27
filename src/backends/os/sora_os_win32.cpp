// sora_os_win32.cpp

#ifndef SORA_OS_WIN32_CPP
#define SORA_OS_WIN32_CPP

//- include lib
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "dwmapi")

//~ implementation

//- state functions 

function void
os_init() {
    
	// arenas
	os_state.window_arena = arena_create(megabytes(64));
	os_state.event_list_arena = arena_create(kilobytes(64));
	os_state.entity_arena = arena_create(megabytes(128));
    
	// init entity free list
	os_state.entity_free = nullptr;
    
	// init window list
	os_state.window_first = nullptr;
	os_state.window_last = nullptr;
	os_state.window_free = nullptr;
    
    
	// time
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&os_state.time_frequency);
	os_state.blink_time = GetCaretBlinkTime();
	os_state.double_click_time = GetDoubleClickTime();
    
    // system info
    SYSTEM_INFO w32_system_info = { 0 };
    GetSystemInfo(&w32_system_info);
    os_state.system_info.logical_processor_count = (u32)w32_system_info.dwNumberOfProcessors;
    
	// register window class
	WNDCLASS window_class = { 0 };
	window_class.lpfnWndProc = os_w32_window_procedure;
	window_class.lpszClassName = "sora_window_class";
	window_class.hInstance = GetModuleHandle(NULL);
	window_class.hCursor = LoadCursorA(0, IDC_ARROW);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassA(&window_class);
    
	// load cursors
	os_state.cursors[os_cursor_pointer] = LoadCursorA(NULL, IDC_ARROW);
	os_state.cursors[os_cursor_I_beam] = LoadCursorA(NULL, IDC_IBEAM);
	os_state.cursors[os_cursor_resize_EW] = LoadCursorA(NULL, IDC_SIZEWE);
	os_state.cursors[os_cursor_resize_NS] = LoadCursorA(NULL, IDC_SIZENS);
	os_state.cursors[os_cursor_resize_NWSE] = LoadCursorA(NULL, IDC_SIZENWSE);
	os_state.cursors[os_cursor_resize_NESW] = LoadCursorA(NULL, IDC_SIZENESW);
	os_state.cursors[os_cursor_resize_all] = LoadCursorA(NULL, IDC_SIZEALL);
	os_state.cursors[os_cursor_hand_point] = LoadCursorA(NULL, IDC_HAND);
	os_state.cursors[os_cursor_disabled] = LoadCursorA(NULL, IDC_NO);
    
	// init locks
	InitializeCriticalSection(&os_state.entity_mutex);
    
}

function void
os_release() {
    
	// release locks
	DeleteCriticalSection(&os_state.entity_mutex);
    
	// release arenas
	arena_release(os_state.window_arena);
	arena_release(os_state.event_list_arena);
}

function void
os_update() {
    prof_scope("os_update") {
        
        // clear event list
        arena_clear(os_state.event_list_arena);
        os_state.event_list = { 0 };
        
        // dispatch win32 messages
        for (MSG message; PeekMessageA(&message, 0, 0, 0, PM_REMOVE);) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        
        // update each window
        for (os_w32_window_t* window = os_state.window_first; window != 0; window = window->next) {
            
            // window size
            RECT w32_rect = { 0 };
            GetClientRect(window->handle, &w32_rect);
            window->resolution = uvec2((w32_rect.right - w32_rect.left), (w32_rect.bottom - w32_rect.top));
            
            // mouse position
            POINT cursor_pos;
            GetCursorPos(&cursor_pos);
            ScreenToClient(window->handle, &cursor_pos);
            window->mouse_pos_last = window->mouse_pos;
            window->mouse_pos = { (f32)cursor_pos.x, (f32)cursor_pos.y };
            window->mouse_delta = vec2_sub(window->mouse_pos, window->mouse_pos_last);
            
            // time
            window->tick_previous = window->tick_current;
            QueryPerformanceCounter(&window->tick_current);
            window->delta_time = (f64)(window->tick_current.QuadPart - window->tick_previous.QuadPart) / (f64)os_state.time_frequency.QuadPart;
            window->elasped_time += window->delta_time;
            
            window->maximized = IsZoomed(window->handle);
        }
        
    }
}

function void
os_abort(u32 exit_code) {
	ExitProcess(exit_code);
}

function void
os_sleep(u32 ms) {
	Sleep(ms);
}

function u64
os_time_microseconds() {
	LARGE_INTEGER current_time;
	QueryPerformanceCounter(&current_time);
	f64 time_in_seconds = ((f64)current_time.QuadPart) / ((f64)os_state.time_frequency.QuadPart);
	u64 time = (u64)(time_in_seconds * 1000000);
	return time;
}

function f64
os_time_as_microseconds(u64 time) {
    return (f64)(time / 1000000);
}

function b8
os_any_window_exist() {
    return os_state.window_first != nullptr;
}

function void
os_set_cursor(os_cursor cursor) {
    if (cursor == os_cursor_null) {
        while(ShowCursor(0) >= 0);
    } else {
        while(ShowCursor(1) < 0);
        HCURSOR hcursor = os_state.cursors[cursor];
        SetCursor(hcursor);
    }
}

function os_system_info_t 
os_get_system_info() {
    return os_state.system_info;
}

function u32
os_get_thread_id() {
    return GetCurrentThreadId();
}


function color_t
os_get_sys_color(os_sys_color id) {
    
    // TODO: this is kinda hacky.
    
    DWORD result_color;
    
    if (id == os_sys_color_accent) {
        DWORD size = 0;
        RegGetValueA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\DWM", "AccentColor", RRF_RT_REG_DWORD, nullptr, &result_color, &size);
    } else {
        result_color = GetSysColor(id);
    }
    
    
    f32 r = (f32)GetRValue(result_color) / 255.0f;
    f32 g = (f32)GetGValue(result_color) / 255.0f;
    f32 b = (f32)GetBValue(result_color) / 255.0f;
    color_t result = color(r, g, b, 1.0f);
    
    return result;
}

function os_modifiers
os_get_modifiers() {
    os_modifiers modifiers = 0;
    if (GetKeyState(VK_CONTROL) & 0x8000) {
        modifiers |= os_modifier_ctrl;
    }
    if (GetKeyState(VK_SHIFT) & 0x8000) {
        modifiers |= os_modifier_shift;
    }
    if (GetKeyState(VK_MENU) & 0x8000) {
        modifiers |= os_modifier_alt;
    }
    return modifiers;
}

function b8 
os_key_is_down(os_key key) {
    b8 down = false;
    if (GetKeyState(key) & 0x8000) {
        down = true;
    }
    return down;
}

function b8
os_mouse_is_down(os_mouse_button button) {
    b8 down = false;
    if (GetKeyState(button) & 0x8000) {
        down = true;
    }
    return down;
}

//- window functions

function os_handle_t
os_window_open(str_t title, u32 width, u32 height, os_window_flags flags) {
    
    // grab from free list or allocate one
    os_w32_window_t* window = nullptr;
    window = os_state.window_free;
    if (window != nullptr) {
        stack_pop(os_state.window_free);
    } else {
        window = (os_w32_window_t*)arena_alloc(os_state.window_arena, sizeof(os_w32_window_t));
    }
    memset(window, 0, sizeof(os_w32_window_t));
    dll_push_back(os_state.window_first, os_state.window_last, window);
    
    // fill struct
    window->title = title;
    window->flags = flags;
    QueryPerformanceCounter(&window->tick_current);
    window->tick_previous = window->tick_current;
    window->delta_time = 0.0;
    window->elasped_time = 0.0;
    window->resolution = uvec2(width, height);
    window->last_window_placement.length = sizeof(WINDOWPLACEMENT);
    // borderless
    window->borderless = flags & os_window_flag_borderless;
    BOOL enabled = 0;
    DwmIsCompositionEnabled(&enabled);
    window->composition_enabled = enabled;
    
    if (window->borderless) {
        window->title_bar_arena = arena_create(megabytes(8));
    }
    
    // adjust window size
    DWORD style = WS_OVERLAPPEDWINDOW;
    
    // validate size
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, style, FALSE);
    i32 adjusted_width = rect.right - rect.left;
    i32 adjusted_height = rect.bottom - rect.top;
    
    // open window
    DWORD ex_flags = WS_EX_APPWINDOW;
    os_state.new_borderless_window = !!(flags & os_window_flag_borderless);
    window->handle = CreateWindowExA(ex_flags, "sora_window_class", (char*)title.data,
                                     style, CW_USEDEFAULT, CW_USEDEFAULT, adjusted_width, adjusted_height,
                                     nullptr, nullptr, GetModuleHandle(NULL), nullptr);
    os_state.new_borderless_window = false;
    ShowWindow(window->handle, SW_SHOW);
    
    
    os_handle_t window_handle = os_w32_handle_from_window(window);
    return window_handle;
}

function void
os_window_close(os_handle_t handle) {
    
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    
    if (window != nullptr) {
        
        // remove from list and push to free list
        dll_remove(os_state.window_first, os_state.window_last, window);
        stack_push(os_state.window_free, window);
        
        // release arena if needed
        if (window->title_bar_arena != nullptr) { arena_release(window->title_bar_arena); }
        
        // destroy window
        if (window->handle != nullptr) { DestroyWindow(window->handle);  }
        
    }
}

function void
os_window_update(os_handle_t window) {
    // TODO: the user should update each window
}

function void 
os_window_focus(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    SetForegroundWindow(window->handle);
    SetFocus(window->handle);
}

function void
os_window_minimize(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    ShowWindow(window->handle, SW_MINIMIZE);
}

function void
os_window_maximize(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    ShowWindow(window->handle, SW_MAXIMIZE);
}

function void
os_window_restore(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    ShowWindow(window->handle, SW_RESTORE);
}

function void
os_window_fullscreen(os_handle_t handle) {
    
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    
    DWORD window_style = GetWindowLong(window->handle, GWL_STYLE);
    if (window_style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if (GetWindowPlacement(window->handle, &window->last_window_placement) &&
            GetMonitorInfo(MonitorFromWindow(window->handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) {
            SetWindowLong(window->handle, GWL_STYLE, window_style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window->handle, HWND_TOP,
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right -
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom -
                         monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(window->handle, GWL_STYLE, window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window->handle, &window->last_window_placement);
        SetWindowPos(window->handle, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    
}

function void
os_window_set_title(os_handle_t handle, str_t title) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    SetWindowTextA(window->handle, (char*)title.data);
    window->title = title;
}

function str_t
os_window_get_title(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    return window->title;
}

function u32
os_window_get_dpi(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    return GetDpiForWindow(window->handle);
}

function b8 
os_window_is_maximized(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    b8 result = (IsZoomed(window->handle));
    return result;
}

function b8 
os_window_is_minimized(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    b8 result = (IsIconic(window->handle));
    return result;
}

function b8
os_window_is_fullscreen(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    DWORD window_style = GetWindowLong(window->handle, GWL_STYLE);
    return !(window_style & WS_OVERLAPPEDWINDOW);
}

function b8
os_window_is_active(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    HWND active_hwnd = GetActiveWindow();
    return (active_hwnd == window->handle);
}

function void 
os_window_clear_title_bar_client_area(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    arena_clear(window->title_bar_arena);
    window->title_bar_client_area_first = window->title_bar_client_area_last = nullptr;
}

function void
os_window_add_title_bar_client_area(os_handle_t handle, rect_t area) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    os_title_bar_client_area_t* title_bar_client_area = (os_title_bar_client_area_t*)arena_alloc(window->title_bar_arena, sizeof(os_title_bar_client_area_t));
    title_bar_client_area->area = area;
    dll_push_back(window->title_bar_client_area_first, window->title_bar_client_area_last, title_bar_client_area);
}

function void
os_window_set_frame_function(os_handle_t handle, os_frame_function_t* func) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    window->frame_func = func;
}

function vec2_t
os_window_get_cursor_pos(os_handle_t handle) {
    /*os_w32_window_t* window = os_w32_window_from_handle(handle);
    POINT cursor_pos;
    GetCursorPos(&cursor_pos);
    ScreenToClient(window->handle, &cursor_pos);
    return vec2(cursor_pos.x, cursor_pos.y);*/
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    return window->mouse_pos;
}

function void
os_window_set_cursor_pos(os_handle_t handle, vec2_t pos) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    POINT p = { pos.x, pos.y };
    ClientToScreen(window->handle, &p);
    SetCursorPos(p.x, p.y);
}

function uvec2_t 
os_window_get_size(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    return window->resolution;
}

function f32 
os_window_get_delta_time(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    return window->delta_time;
}

function f32
os_window_get_elapsed_time(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    return window->elasped_time;
}

function vec2_t 
os_window_get_mouse_delta(os_handle_t handle) {
    os_w32_window_t* window = os_w32_window_from_handle(handle);
    return window->mouse_delta;
}

//- graphical message

function void
os_graphical_message(b8 error, str_t title, str_t msg) {
    MessageBoxA(0, (char*)msg.data, (char*)title.data, MB_OK |(!!error*MB_ICONERROR));
}

//- memory functions

function u64
os_page_size() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}

//- file functions

function os_handle_t
os_file_open(str_t filepath, os_file_access_flag flags) {
    
    os_handle_t result = { 0 };
    DWORD access_flags = 0;
    DWORD share_mode = 0;
    DWORD creation_disposition = OPEN_EXISTING;
    
    if (flags & os_file_access_flag_read) { access_flags |= GENERIC_READ; }
    if (flags & os_file_access_flag_write) { access_flags |= GENERIC_WRITE; }
    if (flags & os_file_access_flag_execute) { access_flags |= GENERIC_EXECUTE; }
    if (flags & os_file_access_flag_share_read) { share_mode |= FILE_SHARE_READ; }
    if (flags & os_file_access_flag_share_write) { share_mode |= FILE_SHARE_WRITE; }
    if (flags & os_file_access_flag_write) { creation_disposition = CREATE_ALWAYS; }
    if (flags & os_file_access_flag_append) { creation_disposition = OPEN_ALWAYS; }
    if (flags & os_file_access_flag_attribute) { access_flags = READ_CONTROL | FILE_READ_ATTRIBUTES;  share_mode = FILE_SHARE_READ; }
    
    HANDLE handle = CreateFileA((char*)filepath.data, access_flags, share_mode, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (handle == INVALID_HANDLE_VALUE) {
        //printf("[error] failed to open file '%.*s'\n", filepath.size, filepath.data);
    } else {
        result.data[0] = (u64)handle;
    }
    
    return result;
}

function void
os_file_close(os_handle_t file) {
    if (os_handle_equals(file, {0})) { return; }
    HANDLE handle = (HANDLE)file.data[0];
    CloseHandle(handle);
}

function str_t
os_file_read_range(arena_t* arena, os_handle_t file, u32 start, u32 length) {
    
    HANDLE handle = (HANDLE)file.data[0];
    
    str_t result = { 0 };
    LARGE_INTEGER off_li = { 0 };
    off_li.QuadPart = start;
    
    if (SetFilePointerEx(handle, off_li, 0, FILE_BEGIN)) {
        u32 bytes_to_read = length;
        u32 bytes_actually_read = 0;
        result.data = (u8*)arena_alloc(arena, sizeof(u8) * bytes_to_read);
        result.size = 0;
        
        u8* ptr = (u8*)result.data;
        u8* opl = (u8*)result.data + bytes_to_read;
        
        for (;;) {
            u32 unread = (u32)(opl - ptr);
            DWORD to_read = (DWORD)(min(unread, u32_max));
            DWORD did_read = 0;
            if (!ReadFile(handle, ptr, to_read, &did_read, 0)) {
                break;
            }
            ptr += did_read;
            result.size += did_read;
            if (ptr >= opl) {
                break;
            }
        }
    }
    
    return result;
    
}

function str_t
os_file_read_all(arena_t* arena, str_t filepath) {
    
    str_t data = str("");
    os_handle_t file = os_file_open(filepath);
    data = os_file_read_all(arena, file);
    os_file_close(file);
    
    return data;
}

function str_t
os_file_read_all(arena_t* arena, os_handle_t file) {
    os_file_info_t info = os_file_get_info(file);
    str_t data = os_file_read_range(arena, file, 0, info.size);
    return data;
}



//- file info functions

function os_file_info_t
os_file_get_info(os_handle_t file) {
    
    HANDLE handle = (HANDLE)file.data[0];
    
    os_file_info_t result = { 0 };
    BY_HANDLE_FILE_INFORMATION file_info;
    GetFileInformationByHandle(handle, &file_info);
    
    result.flags = os_w32_file_flags_from_attributes(file_info.dwFileAttributes);
    result.size = ((u64)file_info.nFileSizeLow) | (((u64)file_info.nFileSizeHigh) << 32);
    result.creation_time = ((u64)file_info.ftCreationTime.dwLowDateTime) | (((u64)file_info.ftCreationTime.dwHighDateTime) << 32);
    result.last_access_time = ((u64)file_info.ftLastAccessTime.dwLowDateTime) | (((u64)file_info.ftLastAccessTime.dwHighDateTime) << 32);
    result.last_write_time = ((u64)file_info.ftLastWriteTime.dwLowDateTime) | (((u64)file_info.ftLastWriteTime.dwHighDateTime) << 32);
    
    return result;
}

function os_file_info_t
os_file_get_info(str_t filepath) {
    os_handle_t handle = os_file_open(filepath);
    os_file_info_t result = os_file_get_info(handle); 
    os_file_close(handle);
    return result;
}


//- file iterator

function os_handle_t
os_file_iter_begin(str_t filepath, os_file_iter_flags flags) {
    
    temp_t scratch = scratch_begin();
    
    // get entity
    os_w32_entity_t* entity = os_w32_entity_create(os_w32_entity_type_file_iter);
    
    // set flags
    entity->file_iter.flags = flags;
    
    // convert path to wide
    str_t search_path = str_format(scratch.arena, "%.*s\\*", filepath.size, filepath.data);
    str16_t wide_path = str16_from_str(scratch.arena, search_path);
    
    // find first file
    entity->file_iter.handle = FindFirstFileW((WCHAR*)wide_path.data, &entity->file_iter.find_data);
    
    scratch_end(scratch);
    
    os_handle_t handle = { (u64)entity };
    return handle;
}

function void
os_file_iter_end(os_handle_t iter) {
    
    // get entity
    os_w32_entity_t* entity = (os_w32_entity_t*)(iter.data[0]);
    
    // close file
    if (entity->file_iter.handle != 0) {
        FindClose(entity->file_iter.handle);
    }
    
}

function b8 
os_file_iter_next(arena_t* arena, os_handle_t iter, os_file_info_t* file_info) {
    
    temp_t scratch = scratch_begin();
    
    // get entity
    os_w32_entity_t* entity = (os_w32_entity_t*)(iter.data[0]);
    os_file_iter_flags flags = entity->file_iter.flags;
    
    b8 result = false;
    
    if (!(flags & os_file_iter_flag_done) && entity->file_iter.handle != INVALID_HANDLE_VALUE) {
        
        do {
            
            // check is usable
            b8 usable_file = true;
            
            WCHAR *file_name = entity->file_iter.find_data.cFileName;
            DWORD attributes = entity->file_iter.find_data.dwFileAttributes;
            
            if (file_name[0] == '.'){
                if (flags & os_file_iter_flag_skip_hidden_files) {
                    usable_file = false;
                } else if (file_name[1] == 0) {
                    usable_file = false;
                } else if (file_name[1] == '.' && file_name[2] == 0) {
                    usable_file = false;
                }
            }
            
            // skip folders and files
            b8 is_folder = attributes & FILE_ATTRIBUTE_DIRECTORY;
            
            if (is_folder){
                if (flags & os_file_iter_flag_skip_folders){
                    usable_file = false;
                }
            } else{
                if (flags & os_file_iter_flag_skip_files ){
                    usable_file = false;
                }
            }
            
            // emit if usable
            if (usable_file){
                
                str16_t wide_path = str16((u16*)file_name);
                
                file_info->flags = os_w32_file_flags_from_attributes(attributes);
                file_info->name = str_from_str16(arena, wide_path);
                file_info->size = (u32)entity->file_iter.find_data.nFileSizeLow | (((u32)entity->file_iter.find_data.nFileSizeHigh)<<32);
                result = true;
                
                if (!FindNextFileW(entity->file_iter.handle, &entity->file_iter.find_data)){
                    entity->file_iter.flags |= os_file_iter_flag_done;
                }
                
                break;
                
            }
        } while(FindNextFileW(entity->file_iter.handle, &entity->file_iter.find_data));
        
    }
    
    // complete
    if (!result) {
        entity->file_iter.flags |= os_file_iter_flag_done;
    }
    
    scratch_end(scratch);
    
    return result;
}

//- thread functions

function os_handle_t
os_thread_create(os_thread_function_t* thread_function, void* params) {
    
    // get entity
    os_w32_entity_t* entity = os_w32_entity_create(os_w32_entity_type_thread);
    
    entity->thread.func = thread_function;
    entity->thread.handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)os_w32_thread_entry_point, entity, 0, &entity->thread.tid);
    entity->thread.params = params;
    
    os_handle_t handle = { (u64)entity };
    return handle;
}

function b8
os_thread_join(os_handle_t thread, u64 endt_us) {
    
    DWORD sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
    os_w32_entity_t* entity = (os_w32_entity_t*)(thread.data[0]);
    DWORD wait_result = WAIT_OBJECT_0;
    
    if (entity != nullptr) {
        wait_result = WaitForSingleObject(entity->thread.handle, sleep_ms);
        CloseHandle(entity->thread.handle);
        os_w32_entity_release(entity);
    }
    
    return (wait_result == WAIT_OBJECT_0);
}

function void
os_thread_detach(os_handle_t thread) {
    
    os_w32_entity_t* entity = (os_w32_entity_t*)(thread.data[0]);
    
    if (entity != nullptr) {
        CloseHandle(entity->thread.handle);
        os_w32_entity_release(entity);
    }
}

function void
os_thread_set_name(os_handle_t thread, str_t name) {
    
    os_w32_entity_t* entity = (os_w32_entity_t*)(thread.data[0]);
    
    if (entity != nullptr) {
        temp_t scratch = scratch_begin();
        str16_t thread_wide = str16_from_str(scratch.arena, name);
        SetThreadDescription(entity->thread.handle, (WCHAR*)thread_wide.data);
        scratch_end(scratch);
    }
}



//- mutex functions

function os_handle_t 
os_mutex_create() {
    os_w32_entity_t* entity = os_w32_entity_create(os_w32_entity_type_mutex);
    InitializeCriticalSection(&entity->mutex);
    os_handle_t handle = { (u64)entity };
    return handle;
}

function void 
os_mutex_release(os_handle_t mutex) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(mutex.data[0]);
    DeleteCriticalSection(&entity->mutex);
    os_w32_entity_release(entity);
}

function void 
os_mutex_lock(os_handle_t mutex) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(mutex.data[0]);
    EnterCriticalSection(&entity->mutex);
}

function void 
os_mutex_unlock(os_handle_t mutex) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(mutex.data[0]);
    LeaveCriticalSection(&entity->mutex);
}



//- rw_mutex functions

function os_handle_t
os_rw_mutex_create() {
    os_w32_entity_t* entity = os_w32_entity_create(os_w32_entity_type_rw_mutex);
    InitializeSRWLock(&entity->rw_mutex);
    os_handle_t handle = { (u64)entity };
    return handle;
}

function void 
os_rw_mutex_release(os_handle_t rw_mutex) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(rw_mutex.data[0]);
    os_w32_entity_release(entity);
}

function void 
os_rw_mutex_lock_r(os_handle_t rw_mutex) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(rw_mutex.data[0]);
    AcquireSRWLockShared(&entity->rw_mutex);
}

function void 
os_rw_mutex_unlock_r(os_handle_t rw_mutex) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(rw_mutex.data[0]);
    ReleaseSRWLockShared(&entity->rw_mutex);
}

function void 
os_rw_mutex_lock_w(os_handle_t rw_mutex) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(rw_mutex.data[0]);
    AcquireSRWLockExclusive(&entity->rw_mutex);
}

function void 
os_rw_mutex_unlock_w(os_handle_t rw_mutex) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(rw_mutex.data[0]);
    ReleaseSRWLockExclusive(&entity->rw_mutex);
}



//- condition variables

function os_handle_t 
os_condition_variable_create() {
    os_w32_entity_t* entity = os_w32_entity_create(os_w32_entity_type_rw_mutex);
    InitializeConditionVariable(&entity->cv);
    os_handle_t handle = { (u64)entity };
    return handle;
}

function void 
os_condition_variable_release(os_handle_t cv) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(cv.data[0]);
    os_w32_entity_release(entity);
}

function b8 
os_condition_variable_wait(os_handle_t cv, os_handle_t mutex, u64 endt_us) {
    u32 sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
    BOOL result = 0;
    if (sleep_ms > 0) {
        os_w32_entity_t* entity = (os_w32_entity_t*)(cv.data[0]);
        os_w32_entity_t* mutex_entity = (os_w32_entity_t*)(mutex.data[0]);
        result = SleepConditionVariableCS(&entity->cv, &mutex_entity->mutex, sleep_ms);
    }
    return (b8)result;
}

function b8 
os_condition_variable_wait_rw_r(os_handle_t cv, os_handle_t rw_mutex, u64 endt_us) {
    u32 sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
    BOOL result = 0;
    if (sleep_ms > 0) {
        os_w32_entity_t* entity = (os_w32_entity_t*)(cv.data[0]);
        os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex.data[0]);
        result = SleepConditionVariableSRW(&entity->cv, &rw_mutex_entity->rw_mutex, sleep_ms, CONDITION_VARIABLE_LOCKMODE_SHARED);
    }
    return (b8)result;
}

function b8 
os_condition_variable_wait_rw_w(os_handle_t cv, os_handle_t rw_mutex, u64 endt_us) {
    u32 sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
    BOOL result = 0;
    if (sleep_ms > 0) {
        os_w32_entity_t* entity = (os_w32_entity_t*)(cv.data[0]);
        os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex.data[0]);
        result = SleepConditionVariableSRW(&entity->cv, &rw_mutex_entity->rw_mutex, sleep_ms, 0);
    }
    return (b8)result;
}

function void 
os_condition_variable_signal(os_handle_t cv) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(cv.data[0]);
    WakeConditionVariable(&entity->cv);
}

function void 
os_condition_variable_broadcast(os_handle_t cv) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(cv.data[0]);
    WakeAllConditionVariable(&entity->cv);
}

//- fiber functions

function os_handle_t
os_fiber_create(u32 stack_size, os_fiber_function_t* fiber_func, void* params) {
    os_w32_entity_t* entity = os_w32_entity_create(os_w32_entity_type_fiber);
    
    //entity->fiber.params = params;
    //entity->fiber.func = fiber_func;
    entity->fiber.fiber_id = CreateFiber(stack_size, (LPFIBER_START_ROUTINE)fiber_func, params);
    
    os_handle_t handle = { (u64)entity };
    return handle;
}

function void
os_fiber_release(os_handle_t fiber) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(fiber.data[0]);
    DeleteFiber(entity->fiber.fiber_id);
    os_w32_entity_release(entity);
}

function void
os_fiber_switch(os_handle_t fiber) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(fiber.data[0]);
    SwitchToFiber(entity->fiber.fiber_id);
}

function os_handle_t
os_fiber_from_thread() {
    os_w32_entity_t* entity = os_w32_entity_create(os_w32_entity_type_fiber);
    entity->fiber.fiber_id = ConvertThreadToFiber(nullptr);
    os_handle_t handle = { (u64)entity };
    return handle;
}


//- win32 specific functions

// memory functions 

function void*
os_w32_mem_reserve(u64 size) {
    u64 size_snapped = size;
    size_snapped += gigabytes(1) - 1;
    size_snapped -= size_snapped % gigabytes(1);
    void* ptr = VirtualAlloc(0, size_snapped, MEM_RESERVE, PAGE_NOACCESS);
    return ptr;
}

function void
os_w32_mem_release(void* ptr, u64 size) {
    VirtualFree(ptr, 0, MEM_RELEASE);
}

function void
os_w32_mem_commit(void* ptr, u64 size) {
    u64 page_snapped = size;
    page_snapped += os_page_size() - 1;
    page_snapped -= page_snapped % os_page_size();
    VirtualAlloc(ptr, page_snapped, MEM_COMMIT, PAGE_READWRITE);
}

function void
os_w32_mem_decommit(void* ptr, u64 size) {
# pragma warning( push )
# pragma warning( disable : 6250 )
    VirtualFree(ptr, size, MEM_DECOMMIT);
# pragma warning( pop )
}


// window functions

function os_handle_t 
os_w32_handle_from_window(os_w32_window_t* window) {
    os_handle_t handle = { (u64)window };
    return handle;
}

function os_w32_window_t* 
os_w32_window_from_handle(os_handle_t window) {
    os_w32_window_t* w32_window = (os_w32_window_t*)(window.data[0]);
    return w32_window;
}

function os_w32_window_t* 
os_w32_window_from_hwnd(HWND window) {
    os_w32_window_t* result = 0;
    for (os_w32_window_t* w = os_state.window_first; w; w = w->next) {
        if (w->handle == window) {
            result = w;
            break;
        }
    }
    return result;
}

function HWND 
os_w32_hwnd_from_window(os_w32_window_t* window) {
    return window->handle;
}


// entity functions 

function os_w32_entity_t* 
os_w32_entity_create(os_w32_entity_type type) {
    
    os_w32_entity_t* entity = nullptr;
    
    EnterCriticalSection(&os_state.entity_mutex);
    {
        // grab from free list or create one
        entity = os_state.entity_free;
        if (entity != nullptr) {
            stack_pop(os_state.entity_free);
        } else {
            entity = (os_w32_entity_t*)arena_alloc(os_state.entity_arena, sizeof(os_w32_entity_t));
        }
        memset(entity, 0, sizeof(os_w32_entity_t));
    }
    LeaveCriticalSection(&os_state.entity_mutex);
    
    // set type
    entity->type = type;
    
    return entity;
}

function void 
os_w32_entity_release(os_w32_entity_t* entity) {
    
    entity->type = os_w32_entity_type_null;
    
    EnterCriticalSection(&os_state.entity_mutex);
    {
        // push to free stack
        stack_push(os_state.entity_free, entity);
    }
    LeaveCriticalSection(&os_state.entity_mutex);
}

// file functions

function os_file_flags 
os_w32_file_flags_from_attributes(DWORD attributes) {
    
    os_file_flags flags = 0;
    
    if (attributes & FILE_ATTRIBUTE_READONLY) { flags |= os_file_flag_is_read_only; }
    if (attributes & FILE_ATTRIBUTE_HIDDEN) { flags |= os_file_flag_is_hidden; }
    if (attributes & FILE_ATTRIBUTE_DIRECTORY) { flags |= os_file_flag_is_folder; }
    
    return flags;
}


// time functions

function u32 
os_w32_sleep_ms_from_endt_us(u64 endt_us) {
    u32 sleep_ms = 0;
    if (endt_us == u64_max) {
        sleep_ms = INFINITE;
    } else {
        u64 begint = os_time_microseconds();
        if (begint < endt_us) {
            u64 sleep_us = endt_us - begint;
            sleep_ms = (u32)((sleep_us + 999) / 1000);
        }
    }
    return sleep_ms;
}

// thread entry point function

function DWORD
os_w32_thread_entry_point(void* ptr) {
    os_w32_entity_t* entity = (os_w32_entity_t*)ptr;
    
    // init thread context
    thread_context_create();
    
    // launch thread function
    entity->thread.func(entity->thread.params);
    
    // release thread context
    thread_context_release();
    
    return 0;
}

// window procedure function

LRESULT CALLBACK
os_w32_window_procedure(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
    
    os_w32_window_t* window = os_w32_window_from_hwnd(handle);
    os_handle_t window_handle = os_w32_handle_from_window(window);
    os_event_t* event = nullptr;
    LRESULT result = 0;
    
    switch (msg) {
        
        case WM_CLOSE: {
            event = (os_event_t*)arena_calloc(os_state.event_list_arena, sizeof(os_event_t));
            event->window = window_handle;
            event->type = os_event_type_window_close;
            break;
        }
        
        case WM_SIZE:
        case WM_PAINT: {
            if (window != nullptr) {
                UINT width = LOWORD(lparam);
                UINT height = HIWORD(lparam);
                window->resolution = uvec2(width, height);
                PAINTSTRUCT ps = { 0 };
                BeginPaint(handle, &ps);
                if (window->frame_func != nullptr) {
                    window->frame_func();
                }
                EndPaint(handle, &ps);
            }
            break;
        }
        
        case WM_ERASEBKGND: {
            result = 1;
            break;
        }
        
        case WM_ENTERSIZEMOVE: {
            window->is_moving = true;
            break;
        }
        
        case WM_EXITSIZEMOVE: {
            window->is_moving = false;
            QueryPerformanceCounter(&window->tick_current);
            window->tick_previous = window->tick_current;
            break;
        }
        
        case WM_NCACTIVATE: {
            if (!os_state.new_borderless_window && (window == nullptr || window->borderless == 0)) {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            } else {
                result = DefWindowProcA(handle, msg, wparam, -1);
            }
            break;
        }
        
        case WM_NCCALCSIZE: {
            if (os_state.new_borderless_window || (window != nullptr && window->borderless)) {
                f32 dpi = (f32)GetDpiForWindow(handle);
                i32 frame_x = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
                i32 frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
                i32 padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
                if (wparam) {
                    NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lparam;
                    RECT* rect = params->rgrc;
                    rect->right -= frame_x + padding;
                    rect->left += frame_x + padding;
                    rect->bottom -= frame_y + padding;
                    if (IsZoomed(handle)) {
                        rect->top += frame_y + padding;
                        rect->bottom -= 1;
                    }
                } else {
                    RECT* rect = (RECT*)lparam;
                    rect->right -= frame_x + padding;
                    rect->left += frame_x + padding;
                    rect->bottom -= frame_y + padding;
                }
            } else {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            }
            break;
        }
        
        case WM_NCHITTEST: {
            DWORD window_style = window ? GetWindowLong(window->handle, GWL_STYLE) : 0;
            b8 is_fullscreen = !(window_style & WS_OVERLAPPEDWINDOW);
            if (window == nullptr || is_fullscreen || (window != nullptr && !window->borderless)) {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            } else {
                b8 is_default_handled = 0;
                
                result = DefWindowProcA(handle, msg, wparam, lparam);
                switch (result) {
                    case HTNOWHERE:
                    case HTRIGHT:
                    case HTLEFT:
                    case HTTOPLEFT:
                    case HTTOPRIGHT:
                    case HTBOTTOMRIGHT:
                    case HTBOTTOM:
                    case HTBOTTOMLEFT: {
                        is_default_handled = 1;
                        break;
                    }
                }
                
                if (!is_default_handled) {
                    POINT pos_monitor;
                    pos_monitor.x = LOWORD(lparam);
                    pos_monitor.y = HIWORD(lparam);
                    POINT pos_client = pos_monitor;
                    ScreenToClient(handle, &pos_client);
                    
                    f32 dpi = (f32)GetDpiForWindow(handle);
                    i32 frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
                    i32 padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
                    
                    b8 is_over_top_resize = pos_client.y >= 0 && pos_client.y < frame_y + padding;
                    b8 is_over_title_bar = pos_client.y >= 0 && pos_client.y < 30;
                    
                    b8 is_over_title_bar_client_area = 0;
                    for (os_title_bar_client_area_t* area = window->title_bar_client_area_first; area != 0; area = area->next) {
                        if (rect_contains(area->area, vec2(pos_client.x, pos_client.y))) {
                            is_over_title_bar_client_area = 1;
                        }
                    }
                    
                    if (IsZoomed(handle)) {
                        if (is_over_title_bar_client_area) {
                            result = HTCLIENT;
                        } else if (is_over_title_bar) {
                            result = HTCAPTION;
                        } else {
                            result = HTCLIENT;
                        }
                    } else {
                        if (is_over_title_bar_client_area) {
                            result = HTCLIENT;
                        } else if (is_over_top_resize) {
                            result = HTTOP;
                        } else if (is_over_title_bar) {
                            result = HTCAPTION;
                        } else {
                            result = HTCLIENT;
                        }
                    }
                }
            }
            break;
        }
        
        
        case WM_NCPAINT: {
            if (os_state.new_borderless_window || (window != nullptr && window->borderless && !window->composition_enabled)) {
                result = 0;
            } else {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            }
            break;
        }
        
        case WM_DWMCOMPOSITIONCHANGED: {
            if ((window != nullptr && window->borderless)) {
                BOOL enabled = 0;
                DwmIsCompositionEnabled(&enabled);
                window->composition_enabled = enabled;
                if (enabled) {
                    MARGINS m = { 0, 0, 1, 0 };
                    DwmExtendFrameIntoClientArea(handle, &m);
                    DWORD dwmncrp_enabled = DWMNCRP_ENABLED;
                    DwmSetWindowAttribute(handle, DWMWA_NCRENDERING_POLICY, &enabled, sizeof(dwmncrp_enabled));
                }
            } else {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            }
            break;
        }
        
        case WM_WINDOWPOSCHANGED: { 
            result = 0;
            break;
        }
        
        case WM_NCUAHDRAWCAPTION:
        case WM_NCUAHDRAWFRAME: {
            if (os_state.new_borderless_window || (window != nullptr && window->borderless)) {
                result = 0;
            } else {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            }
            break;
        }
        
        case WM_SETICON:
        case WM_SETTEXT: {
            if (os_state.new_borderless_window || (window != nullptr && window->borderless && !window->composition_enabled)) {
                LONG_PTR old_style = GetWindowLongPtrW(handle, GWL_STYLE);
                SetWindowLongPtrW(handle, GWL_STYLE, old_style & ~WS_VISIBLE);
                result = DefWindowProcA(handle, msg, wparam, lparam);
                SetWindowLongPtrW(handle, GWL_STYLE, old_style);
            } else {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            }
            break;
        }
        
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {
            u32 key = (u32)wparam;
            event = (os_event_t*)arena_calloc(os_state.event_list_arena, sizeof(os_event_t));
            event->window = window_handle;
            event->type = os_event_type_key_press;
            event->key = (os_key)key;
            break;
        }
        
        case WM_SYSKEYUP:
        case WM_KEYUP: {
            u32 key = (u32)wparam;
            event = (os_event_t*)arena_calloc(os_state.event_list_arena, sizeof(os_event_t));
            event->window = window_handle;
            event->type = os_event_type_key_release;
            event->key = (os_key)key;
            break;
        }
        
        
        //case WM_SYSCHAR:
        case WM_CHAR: {
            u32 key = (u32)wparam;
            
            if (key == '\r') { key = '\n'; }
            
            if ((key >= 32 && key != 127) || key == '\t' || key == '\n') {
                event = (os_event_t*)arena_calloc(os_state.event_list_arena, sizeof(os_event_t));
                event->window = window_handle;
                event->type = os_event_type_text;
                event->character = key;
            }
            
            break;
        }
        
        // mouse input
        
        case WM_NCMOUSEMOVE:
        case WM_MOUSEMOVE: {
            f32 mouse_x = (f32)(i16)LOWORD(lparam);
            f32 mouse_y = (f32)(i16)HIWORD(lparam);
            event = (os_event_t*)arena_calloc(os_state.event_list_arena, sizeof(os_event_t));
            event->window = window_handle;
            event->type = os_event_type_mouse_move;
            event->position = { mouse_x, mouse_y };
            break;
        }
        
        case WM_MOUSEWHEEL: {
            f32 delta = (f32)GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
            event = (os_event_t*)arena_calloc(os_state.event_list_arena, sizeof(os_event_t));
            event->window = window_handle;
            event->type = os_event_type_mouse_scroll;
            event->scroll = { 0.0f, delta };
            break;
        }
        
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            SetCapture(handle);
            event = (os_event_t*)arena_calloc(os_state.event_list_arena, sizeof(os_event_t));
            event->window = window_handle;
            event->type = os_event_type_mouse_press;
            event->mouse = os_mouse_button_left;
            switch (msg) {
                case WM_RBUTTONDOWN: event->mouse = os_mouse_button_right; break;
                case WM_MBUTTONDOWN: event->mouse = os_mouse_button_middle; break;
            }
            break;
        }
        
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            ReleaseCapture();
            event = (os_event_t*)arena_calloc(os_state.event_list_arena, sizeof(os_event_t));
            event->window = window_handle;
            event->type = os_event_type_mouse_release;
            event->mouse = os_mouse_button_left;
            switch (msg) {
                case WM_RBUTTONUP: event->mouse = os_mouse_button_right; break;
                case WM_MBUTTONUP: event->mouse = os_mouse_button_middle; break;
            }
            break;
        }
        
        default: {
            result = DefWindowProcA(handle, msg, wparam, lparam);
            break;
        }
        
    }
    
    // add event to event list
    if (event) {
        event->modifiers = os_get_modifiers();
        event->position = window->mouse_pos;
        dll_push_back(os_state.event_list.first, os_state.event_list.last, event);
        os_state.event_list.count++;
    }
    
    return result;
    
}


#endif // SORA_OS_WIN32_CPP