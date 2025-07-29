// sora_os_win32.cpp

#ifndef SORA_OS_WIN32_CPP
#define SORA_OS_WIN32_CPP

//~ libs

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "advapi32.lib")

//~ implementation

//- state functions 

function void
os_init() {
    
    // allocate event arena and events
    _os_init_core();
    
    // get system and process info
    os_core_state.system_info = os_get_system_info();
    os_core_state.process_info = os_get_process_info(os_process_get_current());
    
    // init w32 state
    os_w32_state.arena = arena_create(gigabytes(1));
    os_w32_state.hinstance = GetModuleHandle(NULL);
    
    // win32 timing
    timeBeginPeriod(1);
	QueryPerformanceFrequency((LARGE_INTEGER*)&os_core_state.time_frequency);
    
    // register window class
	WNDCLASS window_class = { 0 };
	window_class.lpfnWndProc = os_w32_window_procedure;
	window_class.lpszClassName = "sora_window_class";
	window_class.hInstance = GetModuleHandle(NULL);
	window_class.hCursor = LoadCursorA(0, IDC_ARROW);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
	RegisterClass(&window_class);
    
    // init entity lock
    InitializeCriticalSection(&os_w32_state.entity_mutex);
    
    // set dpi awareness
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    
}

function void
os_release() {
	DeleteCriticalSection(&os_w32_state.entity_mutex);
    UnregisterClass("sora_window_class", os_w32_state.hinstance);
    arena_release(os_w32_state.arena);
    _os_release_core();
}

function void 
os_poll_events() {
    
    os_events_clear();
    
    // dispatch win32 messages
    for (MSG message; PeekMessageA(&message, 0, 0, 0, PM_REMOVE);) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
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

//- info functions 

function os_system_info_t 
os_get_system_info() {
    
    temp_t scratch = scratch_begin();
    
    // TODO: maybe we want to pass the system info by reference instead.
    os_system_info_t system_info = { 0 };
    
    // get win32 system info
    SYSTEM_INFO w32_system_info = { 0 };
    GetSystemInfo(&w32_system_info);
    
    { // get host name and user name
        
        DWORD host_name_size = sizeof(system_info.host_name);
        GetComputerNameExA(ComputerNameDnsHostname, system_info.host_name, &host_name_size);
        
        DWORD user_name_size = sizeof(system_info.user_name);
        GetUserNameA(system_info.user_name, &user_name_size);
        
    }
    
    // get platform name from registry. NOTE: using the registry is not recommended by Microsoft.
    {
        b8 success = false;
        HKEY reg_key;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",0, KEY_READ, &reg_key) == ERROR_SUCCESS) {
            DWORD reg_type = REG_SZ;
            DWORD reg_size = 64;
            if (RegQueryValueEx(reg_key, "ProductName", NULL, &reg_type, (LPBYTE)system_info.platform_name, &reg_size) == ERROR_SUCCESS) {
                success = true;
            };
            RegCloseKey(reg_key);
        }
        
        if (!success) {
            strcpy_s(system_info.platform_version, 64, "Unknown Platform Name");
        }
        
    }
    
    // get version using RtlGetVersion() NOTE: supposedly this is an undocumented function?
    {
        b8 success = false;
        HMODULE handle_ntdll = GetModuleHandle("ntdll.dll");
        if (handle_ntdll) {
            RtlGetVersionPtr rtl_get_version_func = (RtlGetVersionPtr)GetProcAddress(handle_ntdll, "RtlGetVersion");
            if (rtl_get_version_func) {
                RTL_OSVERSIONINFOW os_version_info = { 0 };
                os_version_info.dwOSVersionInfoSize = sizeof(os_version_info);
                if (rtl_get_version_func(&os_version_info) == 0) {
                    //printf("%d.%d.%d\n", os_version_info.dwMajorVersion, os_version_info.dwMinorVersion, os_version_info.dwBuildNumber);
                    str_t version_string = str_format(scratch.arena, "%d.%d.%d", os_version_info.dwMajorVersion, os_version_info.dwMinorVersion, os_version_info.dwBuildNumber);
                    memcpy(system_info.platform_version, version_string.data, sizeof(char) * version_string.size + 1);
                    success = true;
                }
            }
        }
        
        if (!success) {
            strcpy_s(system_info.platform_version, 64, "Unknown Platform Version");
        }
        
    }
    
    // get cpu vendor from cpuid.
    {
        i32 cpu_info[4];
        __cpuid(cpu_info, 0);
        
        // NOTE: vendor string is in EBX, EDX, ECX
        memcpy(system_info.cpu_vendor, &cpu_info[1], 4);      // EBX
        memcpy(system_info.cpu_vendor + 4, &cpu_info[3], 4);  // EDX
        memcpy(system_info.cpu_vendor + 8, &cpu_info[2], 4);  // ECX
        system_info.cpu_vendor[12] = '\0';
    }
    
    // get cpu name from cpuid
    {
        i32 cpu_info[4];
        
        // check if brand string is supported.
        __cpuid(cpu_info, 0x80000000);
        if (cpu_info[0] >= 0x80000004) {
            
            // get brand string
            __cpuid(cpu_info, 0x80000002);
            memcpy(system_info.cpu_name, cpu_info, 16);
            __cpuid(cpu_info, 0x80000003);
            memcpy(system_info.cpu_name + 16, cpu_info, 16);
            __cpuid(cpu_info, 0x80000004);
            memcpy(system_info.cpu_name + 32, cpu_info, 16);
            system_info.cpu_name[48] = '\0';
            
        }
    }
    
    // get cpu architecture
    {
        switch (w32_system_info.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64: { strcpy_s(system_info.cpu_architecture, 64, "x86_64"); break; }
            case PROCESSOR_ARCHITECTURE_ARM64: { strcpy_s(system_info.cpu_architecture, 64, "ARM64"); break; }
            case PROCESSOR_ARCHITECTURE_ARM: { strcpy_s(system_info.cpu_architecture, 64, "ARM"); break; }
            case PROCESSOR_ARCHITECTURE_INTEL: { strcpy_s(system_info.cpu_architecture, 64, "x86"); break; }
            case PROCESSOR_ARCHITECTURE_IA64: { strcpy_s(system_info.cpu_architecture, 64, "IA64"); break; }
            default: { strcpy_s(system_info.cpu_architecture, 64, "Unknown"); break; }
        }
    }
    
    // get cpu core count
    {
        
        // logical core count
        system_info.cpu_logical_core_count = (u32)w32_system_info.dwNumberOfProcessors;
        
        // physical core count
        DWORD buffer_size = 0;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
        u32 physical_cores = 0;
        u32 cache_line_size = 64; // NOTE: assume 64 at start
        
        GetLogicalProcessorInformation(NULL, &buffer_size);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)arena_alloc(scratch.arena, buffer_size);
            if (GetLogicalProcessorInformation(buffer, &buffer_size)) {
                DWORD count = buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
                for (DWORD i = 0; i < count; i++) {
                    
                    // physical core count
                    if (buffer[i].Relationship == RelationProcessorCore) {
                        physical_cores++;
                    }
                    
                    // cache line size
                    if (buffer[i].Relationship == RelationCache) {
                        PCACHE_DESCRIPTOR cache = &buffer[i].Cache;
                        if (cache->Level == 1) {  // L1 cache
                            cache_line_size = cache->LineSize;
                        }
                    }
                    
                }
            }
        }
        
        system_info.cpu_physical_core_count = physical_cores > 0 ? physical_cores : system_info.cpu_logical_core_count;
        system_info.cpu_cache_line_size = cache_line_size;
    }
    
    // get memory info
    {
        system_info.memory_page_size = w32_system_info.dwPageSize;
        system_info.memory_allocation_granularity = w32_system_info.dwAllocationGranularity;
        
        
        MEMORYSTATUSEX mem_status;
        mem_status.dwLength = sizeof(MEMORYSTATUSEX);
        
        if (GlobalMemoryStatusEx(&mem_status)) {
            
            // physical memory
            system_info.memory_total = mem_status.ullTotalPhys;
            system_info.memory_available = mem_status.ullAvailPhys;
            
            // virtual memory (physical + page file)
            system_info.virtual_memory_total = mem_status.ullTotalVirtual;
            system_info.virtual_memory_available = mem_status.ullAvailVirtual;
        }
        
    }
    
    scratch_end(scratch);
    
    return system_info;
    
}


function os_process_info_t
os_get_process_info(os_process_t process_handle) {
    
    temp_t scratch = scratch_begin();
    
    os_process_info_t process_info = { 0 };
    
    // get current and parent process id 
    {
        
        // get pid
        u64 pid = os_process_get_id(process_handle);
        process_info.process_id = pid;
        
        // parent pid
        HANDLE handle_snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(pe32);
        DWORD ppid = 0;
        
        if(Process32First(handle_snapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == pid) {
                    ppid = pe32.th32ParentProcessID;
                    strcpy_s(process_info.exe_name, sizeof(process_info.exe_name), pe32.szExeFile);
                    break;
                }
            } while( Process32Next(handle_snapshot, &pe32));
        }
        CloseHandle(handle_snapshot);
        process_info.parent_process_id = ppid;
    }
    
    // get paths
    GetModuleFileNameA(NULL, process_info.exe_path, sizeof(process_info.exe_path));
    GetCurrentDirectoryA(sizeof(process_info.working_directory), process_info.working_directory);
    
    // get start timestamp
    {
        HANDLE handle_process = GetCurrentProcess();
        FILETIME create_time, exit_time, kernel_time, user_time;
        if (GetProcessTimes(handle_process, &create_time, &exit_time, &kernel_time, &user_time)) {
            ULARGE_INTEGER li;
            li.LowPart = create_time.dwLowDateTime;
            li.HighPart = create_time.dwHighDateTime;
            process_info.start_timestamp = (li.QuadPart - 116444736000000000ULL) / 10000;
        }
        
        // get memory usage
        PROCESS_MEMORY_COUNTERS mem;
        if (GetProcessMemoryInfo(handle_process, &mem, sizeof(mem))) {
            process_info.memory_usage = mem.WorkingSetSize;
        }
    }
    
    // get thread count
    {
        
        HANDLE handle_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);
        DWORD thread_count = 0;
        
        if (Thread32First(handle_snapshot, &te32)) {
            do {
                if (te32.th32OwnerProcessID == process_info.process_id) {
                    thread_count++;
                }
            } while (Thread32Next(handle_snapshot, &te32));
        }
        CloseHandle(handle_snapshot);
        process_info.thread_count = thread_count;
    }
    
    scratch_end(scratch);
    
    return process_info;
}


//- timing functions 

function u64
os_get_time_ms() {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (u64)((counter.QuadPart * 1000) / os_core_state.time_frequency);
}

function u64
os_get_time_freq() {
    return os_core_state.time_frequency;
}

//- memory functions 

function u64 
os_page_size() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}

//- window functions 

function os_window_t 
os_window_open(str_t title, u32 width, u32 height, os_window_flags flags) {
    
    // pull from free list or allocate one
    os_w32_window_t* window = os_w32_state.window_free;
    if (window != nullptr) {
        stack_pop(os_w32_state.window_free);
    } else {
        window = (os_w32_window_t*)arena_alloc(os_w32_state.arena, sizeof(os_w32_window_t));
    }
    memset(window, 0, sizeof(os_w32_window_t));
    dll_push_back(os_w32_state.window_first, os_w32_state.window_last, window);
    
    // fill struct
    window->title = title;
    window->flags = flags;
    window->size = uvec2(width, height);
    
    // NOTE: for fullscreen
    window->last_window_placement.length = sizeof(WINDOWPLACEMENT);
    
    // determine style and correct sizing.
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD ex_flags = 0;
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, style, FALSE);
    i32 adjusted_width = rect.right - rect.left;
    i32 adjusted_height = rect.bottom - rect.top;
    
    if (flags & os_window_flag_always_on_top) {
        ex_flags |= WS_EX_TOPMOST;
    }
    
    if (flags & os_window_flag_borderless) {
        os_w32_state.new_borderless_window = true;
    }
    
    // create window
    window->handle = CreateWindowExA(ex_flags, "sora_window_class", (char*)title.data, style, 
                                     CW_USEDEFAULT, CW_USEDEFAULT, adjusted_width, adjusted_height, 
                                     nullptr, nullptr, GetModuleHandle(NULL), nullptr);
    os_window_t window_handle = os_window_handle_from_w32_window(window);
    
    if (flags & os_window_flag_borderless) {
        os_w32_state.new_borderless_window = false;
        //MARGINS margins = { -1, -1, -1, -1 };
        //DwmExtendFrameIntoClientArea(window->handle, &margins);
        
    }
    
    // NOTE: this allows the client space to be transparent.
    // it does not support the acrylic blur or for the 
    // titlebar to be transparent.
    // TODO: investigate a way to support acrylic blur.
    if (flags & os_window_flag_transparent) {
        //DWM_BLURBEHIND bb = {};
        //bb.dwFlags = DWM_BB_ENABLE;
        //bb.fEnable = TRUE;
        //bb.hRgnBlur = nullptr;
        //DwmEnableBlurBehindWindow(window->handle, &bb);
    }
    
    // handle window flags
    i32 show_command = SW_SHOW;
    if (flags& os_window_flag_fullscreen) {
        os_window_set_fullscreen(window_handle, true);
    } else if (flags & os_window_flag_maximized) {
        show_command = SW_MAXIMIZE;
    } else if  (flags & os_window_flag_minimized) {
        show_command = SW_MINIMIZE;
    }
    
    ShowWindow(window->handle, show_command);
    UpdateWindow(window->handle);
    
    return window_handle;
}

function void
os_window_close(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    
    if (window != nullptr) {
        
        // remove from list and push to free list
        dll_remove(os_w32_state.window_first, os_w32_state.window_last, window);
        stack_push(os_w32_state.window_free, window);
        
        // destroy window
        if (window->handle != nullptr) { DestroyWindow(window->handle);  }
        
    }
    
}

function void
os_window_set_resize_callback(os_window_t window_handle, os_resize_callback_func* callback_func) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    
    if (window != nullptr) {
        window->resize_callback_func = callback_func;
    }
    
}

function b8
os_window_is_focused(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return window->focused;
}

function b8
os_window_is_minimized(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return window->mode == os_w32_window_mode_minimized;
}

function b8
os_window_is_maximized(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return window->mode == os_w32_window_mode_maximized;
}

function b8
os_window_is_fullscreen(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return window->mode == os_w32_window_mode_fullscreen;
}

function str_t 
os_window_get_title(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return window->title;
}

function uvec2_t
os_window_get_position(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return window->position;
}

function uvec2_t
os_window_get_size(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return window->size;
}

function uvec2_t
os_window_get_mouse_position(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return window->mouse_position;
}

function f32
os_window_get_dpi(os_window_t window_handle) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    return GetDpiForWindow(window->handle);
}


function b8
os_window_set_focus(os_window_t window_handle, b8 focus) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    b8 result = false;
    if (focus) {
        result = SetForegroundWindow(window->handle);
    } else {
        HWND hwnd_desktop = GetDesktopWindow();
        result = SetForegroundWindow(hwnd_desktop); 
    }
    return result;
}

function b8
os_window_set_minimize(os_window_t window_handle, b8 minimize) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    b8 result = false;
    if (minimize) {
        result = ShowWindow(window->handle, SW_MINIMIZE);
    } else {
        result = ShowWindow(window->handle, SW_RESTORE);
    }
    return result;
}

function b8
os_window_set_maximize(os_window_t window_handle, b8 maximize) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    b8 result = false;
    if (maximize) {
        result = ShowWindow(window->handle, SW_MAXIMIZE);
    } else {
        result = ShowWindow(window->handle, SW_RESTORE);
    }
    return result;
}

function b8
os_window_set_fullscreen(os_window_t window_handle, b8 fullscreen) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    DWORD window_style = GetWindowLong(window->handle, GWL_STYLE);
    b8 is_fullscreen = !(window_style & WS_OVERLAPPEDWINDOW);
    b8 result =  false;
    
    if (fullscreen && !is_fullscreen) {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if (GetWindowPlacement(window->handle, &window->last_window_placement) &&
            GetMonitorInfo(MonitorFromWindow(window->handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) {
            SetWindowLong(window->handle, GWL_STYLE, window_style & ~WS_OVERLAPPEDWINDOW);
            result = SetWindowPos(window->handle, HWND_TOP,
                                  monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                                  monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                                  monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                                  SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            window->mode = os_w32_window_mode_fullscreen;
        }
        
    } else if (!fullscreen && is_fullscreen){
        SetWindowLong(window->handle, GWL_STYLE, window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window->handle, &window->last_window_placement);
        result = SetWindowPos(window->handle, 0, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                              SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        window->mode = os_w32_window_mode_normal;
        
    }
    
    return result;
}

function b8
os_window_set_title(os_window_t window_handle, str_t title) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    b8 result = SetWindowTextA(window->handle, (char*)title.data);
    window->title = title;
    return result;
}

function b8
os_window_set_position(os_window_t window_handle, uvec2_t position) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    b8 result = SetWindowPos(window->handle, HWND_TOP, position.x, position.y, 0, 0, SWP_NOSIZE);
    return result;
}

function b8
os_window_set_size(os_window_t window_handle, uvec2_t size) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    b8 result = SetWindowPos(window->handle, NULL, 0, 0, size.x, size.y, SWP_NOMOVE | SWP_NOZORDER);
    return result;
}

function b8
os_window_set_mouse_position(os_window_t window_handle, uvec2_t position) {
    os_w32_window_t* window = os_w32_window_from_window_handle(window_handle);
    POINT p = { position.x, position.y };
    ClientToScreen(window->handle, &p);
    b8 result = SetCursorPos(p.x, p.y);
    return result;
}

//- file functions 

function os_file_t
os_file_open(str_t filepath, os_file_access_flags flags) {
    
    os_file_t file_handle = { 0 };
    
    DWORD access_flags = 0;
    DWORD share_mode = 0;
    DWORD creation_disposition = OPEN_EXISTING;
    
    if (flags & os_file_access_flag_read) { access_flags |= GENERIC_READ; }
    if (flags & os_file_access_flag_write) { access_flags |= GENERIC_WRITE; }
    if (flags & os_file_access_flag_execute) { access_flags |= GENERIC_EXECUTE; }
    if (flags & os_file_access_flag_shared_read) { share_mode |= FILE_SHARE_READ; }
    if (flags & os_file_access_flag_shared_write) { share_mode |= FILE_SHARE_WRITE; }
    if (flags & os_file_access_flag_write) { creation_disposition = CREATE_ALWAYS; }
    if (flags & os_file_access_flag_append) { creation_disposition = OPEN_ALWAYS; }
    if (flags & os_file_access_flag_attribute) { access_flags = READ_CONTROL | FILE_READ_ATTRIBUTES;  share_mode = FILE_SHARE_READ; }
    
    HANDLE handle = CreateFileA((char*)filepath.data, access_flags, share_mode, NULL, creation_disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    file_handle.id = (u64)handle;
    
    return file_handle;
}

function b8
os_file_close(os_file_t file_handle) {
    HANDLE handle = (HANDLE)file_handle.id;
    b8 result = false;
    if (handle != 0) {
        result = CloseHandle(handle);
    }
    return result;
}

function str_t 
os_file_read_range(arena_t* arena, os_file_t file_handle, u32 start, u32 length) {
    HANDLE handle = (HANDLE)file_handle.id;
    str_t result = { 0 };
    if (handle != 0) {
        LARGE_INTEGER off_li = { 0 };
        off_li.QuadPart = start;
        
        if (SetFilePointerEx(handle, off_li, 0, FILE_BEGIN)) {
            u32 bytes_to_read = length;
            u32 bytes_actually_read = 0;
            result.data = (u8*)arena_alloc(arena, sizeof(u8) * bytes_to_read + 1);
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
    }
    return result;
}

function str_t 
os_file_read_all(arena_t* arena, os_file_t file) {
    os_file_info_t info = os_file_get_info(file);
    str_t data = os_file_read_range(arena, file, 0, info.size);
    return data;
    
}

function str_t 
os_file_read_all(arena_t* arena, str_t filepath) {
    str_t data = str("");
    os_file_t file = os_file_open(filepath);
    data = os_file_read_all(arena, file);
    os_file_close(file);
    return data;
}

function u32
os_file_read(os_file_t file_handle, u32 index, void* data, u32 size) {
    
    HANDLE handle = (HANDLE)file_handle.id;
    u32 amount_read = 0;
    
    if (handle != 0) {
        
        u32 dst_off = 0;
        u32 src_off = index;
        
        for (;;) {
            void* bytes_dst = (u8*)data + dst_off;
            u32 bytes_left = size - dst_off;
            DWORD read_size = min(megabytes(1), bytes_left);
            DWORD bytes_read = 0;
            
            OVERLAPPED overlapped = {0};
            overlapped.Offset = index;
            overlapped.OffsetHigh = 0;
            
            BOOL success = ReadFile(handle, bytes_dst, read_size, &bytes_read, &overlapped);
            if (!success || bytes_read == 0) {
                break;
            }
            
            dst_off += bytes_read;
            src_off += bytes_read;
            
            if (dst_off >= size) {
                break;
            }
        }
        
        amount_read = dst_off;
    }
    
    return amount_read;
}

function u32 
os_file_write(os_file_t file_handle, u32 index, void* data, u32 size) {
    
    HANDLE handle = (HANDLE)file_handle.id;
    u32 amount_written = 0;
    
    if (handle != 0) {
        
        u32 src_off = 0;
        u32 dst_off = index;
        
        for(;;) {
            void *bytes_src = (u8*)data + src_off;
            u32 bytes_left = size - src_off;
            DWORD write_size = min(megabytes(1), bytes_left);
            DWORD bytes_written = 0;
            OVERLAPPED overlapped = {0};
            overlapped.Offset = (dst_off & 0x00000000ffffffffull);
            overlapped.OffsetHigh = (dst_off & 0xffffffff00000000ull) >> 32;
            BOOL success = WriteFile(handle, bytes_src, write_size, &bytes_written, &overlapped);
            if(success == false) {
                break;
            }
            src_off += bytes_written;
            dst_off += bytes_written;
            
            if(bytes_left == 0) {
                break;
            }
        }
        
        amount_written = src_off;
    }
    
    return amount_written;
}

function b8 
os_file_copy(str_t src_filepath, str_t dst_filepath) {
    return CopyFile((char*)src_filepath.data, (char*)dst_filepath.data, FALSE);
}

function b8 
os_file_move(str_t filepath, str_t new_filepath) {
    // TODO: investigate
    return MoveFileEx((char*)filepath.data, (char*)new_filepath.data, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
}

function b8 
os_file_rename(str_t filepath, str_t new_filepath) {
    return MoveFileEx((char*)filepath.data, (char*)new_filepath.data, MOVEFILE_REPLACE_EXISTING);
}

function b8 
os_file_delete(str_t filepath) {
    return DeleteFile((char*)filepath.data);
}

function b8 
os_file_exists(str_t filepath) {
    DWORD attributes = GetFileAttributes((char*)filepath.data);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

// file info functions 

function os_file_info_t
os_file_get_info(os_file_t file_handle) {
    
    os_file_info_t result = { 0 };
    HANDLE handle = (HANDLE)file_handle.id;
    
    if (handle != 0) {
        
        BY_HANDLE_FILE_INFORMATION file_info;
        GetFileInformationByHandle(handle, &file_info);
        
        result.flags = os_w32_file_flags_from_attributes(file_info.dwFileAttributes);
        result.size = ((u64)file_info.nFileSizeLow) | (((u64)file_info.nFileSizeHigh) << 32);
        result.creation_time = ((u64)file_info.ftCreationTime.dwLowDateTime) | (((u64)file_info.ftCreationTime.dwHighDateTime) << 32);
        result.last_accessed_time = ((u64)file_info.ftLastAccessTime.dwLowDateTime) | (((u64)file_info.ftLastAccessTime.dwHighDateTime) << 32);
        result.last_modified_time = ((u64)file_info.ftLastWriteTime.dwLowDateTime) | (((u64)file_info.ftLastWriteTime.dwHighDateTime) << 32);
        
    }
    
}

function os_file_info_t
os_file_get_info(str_t filepath) {
    os_file_t file_handle = os_file_open(filepath);
    os_file_info_t result = os_file_get_info(file_handle); 
    os_file_close(file_handle);
    return result;
}

//- process functions 

// TODO:
function os_process_t
os_process_create(str_t executable_path) {
    
}

function b8 
os_process_join(os_process_t process_handle, u64 timeout_us) {
    
}

function b8
os_process_detach(os_process_t process_handle) {
    
}

function b8 
os_process_kill(os_process_t process_handle, u32 exit_code) {
    HANDLE process = (HANDLE)process_handle.id;
    return TerminateProcess(process, exit_code);
}

function os_process_t 
os_process_get_current() {
    HANDLE process = GetCurrentProcess();
    os_process_t process_handle = {(u64)process};
    return process_handle;
}

function u64 
os_process_get_current_id() {
    return GetCurrentProcessId();
}

function u64 
os_process_get_id(os_process_t process_handle) {
    HANDLE process = (HANDLE)process_handle.id;
    u64 process_id = GetProcessId(process);
    return process_id;
}

//- thread functions 

function os_thread_t
os_thread_create(os_thread_func* thread_func, void* params, os_thread_priority priority) {
    os_w32_entity_t* thread_entity = os_w32_entity_alloc(os_w32_entity_thread);
    
    thread_entity->thread.func = thread_func;
    thread_entity->thread.handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)os_w32_thread_entry_point, thread_entity, 0, &thread_entity->thread.tid);
    thread_entity->thread.params = params;
    os_thread_t thread_handle = { (u64)thread_entity };
    
    // set priority
    if (priority != os_thread_priority_normal) {
        os_thread_set_priority(thread_handle, priority);
    }
    
    return thread_handle;
}

function b8 
os_thread_join(os_thread_t thread_handle, u64 timeout_us) {
    os_w32_entity_t* thread_entity = (os_w32_entity_t*)thread_handle.id;
    
    DWORD sleep_ms = os_w32_sleep_ms_from_timeout_us(timeout_us);
    DWORD wait_result = WAIT_OBJECT_0;
    
    if (thread_entity != nullptr) {
        
        wait_result = WaitForSingleObject(thread_entity->thread.handle, sleep_ms);
        
        // release entity
        CloseHandle(thread_entity->thread.handle);
        os_w32_entity_release(thread_entity);
    }
    
    return (wait_result == WAIT_OBJECT_0);
}

function b8 
os_thread_detach(os_thread_t thread_handle) {
    os_w32_entity_t* thread_entity = (os_w32_entity_t*)thread_handle.id;
    
    // release entity
    if (thread_entity != nullptr) {
        CloseHandle(thread_entity->thread.handle);
        os_w32_entity_release(thread_entity);
    }
    
}

function void
os_thread_yield() {
    // if we can't yield, just sleep
    if (!SwitchToThread()) {
        Sleep(1);
    }
}

function os_thread_t 
os_thread_get_current() {
    return os_current_thread;
}

function u64 
os_thread_get_current_id() {
    return (u64)GetCurrentThreadId();
}

function u64 
os_thread_get_id(os_thread_t thread_handle) {
    os_w32_entity_t* thread_entity = (os_w32_entity_t*)thread_handle.id;
    u64 tid = 0;
    if (thread_entity != nullptr) {
        tid = (u64)GetThreadId(thread_entity->thread.handle);
    }
    return tid;
}

function b8 
os_thread_set_name(os_thread_t thread_handle, str_t name) {
    os_w32_entity_t* thread_entity = (os_w32_entity_t*)thread_handle.id;
    b8 result = false;
    if (thread_entity != nullptr) {
        temp_t scratch = scratch_begin();
        str16_t thread_wide = str16_from_str(scratch.arena, name);
        result = SetThreadDescription(thread_entity->thread.handle, (WCHAR*)thread_wide.data);
        scratch_end(scratch);
    }
    return result;
}

function b8
os_thread_set_priority(os_thread_t thread_handle, os_thread_priority priority) {
    os_w32_entity_t* thread_entity = (os_w32_entity_t*)thread_handle.id;
    b8 result = false;
    if (thread_entity != nullptr) {
        
        i32 w32_priority = THREAD_PRIORITY_NORMAL;
        switch (priority) {
            case os_thread_priority_idle: { w32_priority = THREAD_PRIORITY_IDLE; break; }
            case os_thread_priority_low: { w32_priority = THREAD_PRIORITY_LOWEST; break; }
            case os_thread_priority_normal: { w32_priority = THREAD_PRIORITY_NORMAL; break; }
            case os_thread_priority_high: { w32_priority = THREAD_PRIORITY_HIGHEST; break; }
            case os_thread_priority_time_critical: { w32_priority = THREAD_PRIORITY_TIME_CRITICAL; break; }
        }
        
        b8 result = SetThreadPriority(thread_entity->thread.handle, w32_priority);
    }
    return result;
}

//- fiber functions 

function os_fiber_t 
os_fiber_create(os_fiber_func* fiber_func, void* params, u32 stack_size) {
    os_w32_entity_t* fiber_entity = os_w32_entity_alloc(os_w32_entity_fiber);
    
    fiber_entity->fiber = CreateFiber(stack_size, (LPFIBER_START_ROUTINE)fiber_func, params);
    
    os_fiber_t fiber_handle = { (u64)fiber_entity };
    return fiber_handle;
}

function os_fiber_t
os_fiber_from_thread() {
    os_w32_entity_t* fiber_entity = os_w32_entity_alloc(os_w32_entity_fiber);
    fiber_entity->fiber = ConvertThreadToFiber(nullptr);
    os_fiber_t fiber_handle = { (u64)fiber_entity };
    return fiber_handle;
}

function void
os_fiber_release(os_fiber_t fiber_handle) {
    os_w32_entity_t* fiber_entity = (os_w32_entity_t*)fiber_handle.id;
    if (fiber_entity != nullptr) {
        DeleteFiber(fiber_entity->fiber);
        os_w32_entity_release(fiber_entity);
    }
}

function void
os_fiber_switch(os_fiber_t fiber_handle) {
    os_w32_entity_t* fiber_entity = (os_w32_entity_t*)fiber_handle.id;
    if (fiber_entity != nullptr) {
        SwitchToFiber(fiber_entity->fiber);
    }
}

//- mutex functions 

function os_mutex_t 
os_mutex_create() {
    os_w32_entity_t* mutex_entity = os_w32_entity_alloc(os_w32_entity_mutex);
    InitializeCriticalSection(&mutex_entity->mutex);
    os_mutex_t mutex_handle = { (u64)mutex_entity};
    return mutex_handle;
}

function void
os_mutex_release(os_mutex_t mutex_handle) {
    os_w32_entity_t* mutex_entity = (os_w32_entity_t*)mutex_handle.id;
    if (mutex_entity != nullptr) {
        DeleteCriticalSection(&mutex_entity->mutex);
        os_w32_entity_release(mutex_entity);
    }
}

function void 
os_mutex_lock(os_mutex_t mutex_handle) {
    os_w32_entity_t* mutex_entity = (os_w32_entity_t*)(mutex_handle.id);
    if (mutex_entity != nullptr) {
        EnterCriticalSection(&mutex_entity->mutex);
    }
}

function void 
os_mutex_unlock(os_mutex_t mutex_handle) {
    os_w32_entity_t* mutex_entity = (os_w32_entity_t*)(mutex_handle.id);
    if (mutex_entity != nullptr) {
        LeaveCriticalSection(&mutex_entity->mutex);
    }
}

//- rw mutex functions 



function os_rw_mutex_t
os_rw_mutex_create() {
    os_w32_entity_t* rw_mutex_entity = os_w32_entity_alloc(os_w32_entity_rw_mutex);
    InitializeSRWLock(&rw_mutex_entity->rw_mutex);
    os_rw_mutex_t rw_mutex_handle = { (u64)rw_mutex_entity };
    return rw_mutex_handle;
}

function void 
os_rw_mutex_release(os_rw_mutex_t rw_mutex_handle) {
    os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex_handle.id);
    if (rw_mutex_entity != nullptr) {
        os_w32_entity_release(rw_mutex_entity);
    }
}

function void 
os_rw_mutex_lock_r(os_rw_mutex_t rw_mutex_handle) {
    os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex_handle.id);
    if (rw_mutex_entity != nullptr) {
        AcquireSRWLockShared(&rw_mutex_entity->rw_mutex);
    }
}

function void 
os_rw_mutex_unlock_r(os_rw_mutex_t rw_mutex_handle) {
    os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex_handle.id);
    if (rw_mutex_entity != nullptr) {
        ReleaseSRWLockShared(&rw_mutex_entity->rw_mutex);
    }
}

function void 
os_rw_mutex_lock_w(os_rw_mutex_t rw_mutex_handle) {
    os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex_handle.id);
    if (rw_mutex_entity != nullptr) {
        AcquireSRWLockExclusive(&rw_mutex_entity->rw_mutex);
    }
}

function void 
os_rw_mutex_unlock_w(os_rw_mutex_t rw_mutex_handle) {
    os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex_handle.id);
    if (rw_mutex_entity != nullptr) {
        ReleaseSRWLockExclusive(&rw_mutex_entity->rw_mutex);
    }
}

//- condition variable functions 

function os_condition_variable_t 
os_condition_variable_create() {
    os_w32_entity_t* entity = os_w32_entity_alloc(os_w32_entity_condition_variable);
    InitializeConditionVariable(&entity->cv);
    os_condition_variable_t handle = { (u64)entity };
    return handle;
}

function void 
os_condition_variable_release(os_condition_variable_t condition_variable_handle) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(condition_variable_handle.id);
    os_w32_entity_release(entity);
}

function b8 
os_condition_variable_wait(os_condition_variable_t condition_variable_handle, os_mutex_t mutex_handle, u64 timeout_us) {
    DWORD sleep_ms = os_w32_sleep_ms_from_timeout_us(timeout_us);
    b8 result = 0;
    if (sleep_ms > 0) {
        os_w32_entity_t* entity = (os_w32_entity_t*)(condition_variable_handle.id);
        os_w32_entity_t* mutex_entity = (os_w32_entity_t*)(mutex_handle.id);
        result = SleepConditionVariableCS(&entity->cv, &mutex_entity->mutex, sleep_ms);
    }
    return result;
}

function b8 
os_condition_variable_wait_rw_r(os_condition_variable_t condition_variable_handle, os_rw_mutex_t rw_mutex, u64 timeout_us) {
    DWORD sleep_ms = os_w32_sleep_ms_from_timeout_us(timeout_us);
    b8 result = 0;
    if (sleep_ms > 0) {
        os_w32_entity_t* entity = (os_w32_entity_t*)(condition_variable_handle.id);
        os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex.id);
        result = SleepConditionVariableSRW(&entity->cv, &rw_mutex_entity->rw_mutex, sleep_ms, CONDITION_VARIABLE_LOCKMODE_SHARED);
    }
    return result;
}

function b8 
os_condition_variable_wait_rw_w(os_condition_variable_t condition_variable_handle, os_rw_mutex_t rw_mutex, u64 timeout_us) {
    DWORD sleep_ms = os_w32_sleep_ms_from_timeout_us(timeout_us);
    b8 result = 0;
    if (sleep_ms > 0) {
        os_w32_entity_t* entity = (os_w32_entity_t*)(condition_variable_handle.id);
        os_w32_entity_t* rw_mutex_entity = (os_w32_entity_t*)(rw_mutex.id);
        result = SleepConditionVariableSRW(&entity->cv, &rw_mutex_entity->rw_mutex, sleep_ms, 0);
    }
    return result;
}

function void 
os_condition_variable_signal(os_condition_variable_t condition_variable_handle) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(condition_variable_handle.id);
    WakeConditionVariable(&entity->cv);
}

function void 
os_condition_variable_broadcast(os_condition_variable_t condition_variable_handle) {
    os_w32_entity_t* entity = (os_w32_entity_t*)(condition_variable_handle.id);
    WakeAllConditionVariable(&entity->cv);
}

//- semaphore functions 

function os_semaphore_t
os_semaphore_create(u32 initial_count, u32 max_count, str_t name) {
    HANDLE semaphore = CreateSemaphoreA(0, initial_count, max_count, (LPCSTR)name.data);
    os_semaphore_t semaphore_handle = { (u64)semaphore };
    return semaphore_handle;
}

function os_semaphore_t
os_semaphore_open(str_t name) {
    HANDLE semaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS , 0, (LPCSTR)name.data);
    os_semaphore_t semaphore_handle = { (u64)semaphore };
    return semaphore_handle;
}

function void
os_semaphore_release(os_semaphore_t semaphore_handle) {
    HANDLE semaphore = (HANDLE)semaphore_handle.id;
    CloseHandle(semaphore);
}

function void
os_semaphore_close(os_semaphore_t semaphore_handle) {
    HANDLE semaphore = (HANDLE)semaphore_handle.id;
    CloseHandle(semaphore);
}

function b8
os_semaphore_wait(os_semaphore_t semaphore_handle, u64 timeout_us) {
    HANDLE semaphore = (HANDLE)semaphore_handle.id;
    DWORD sleep_ms = os_w32_sleep_ms_from_timeout_us(timeout_us);
    DWORD wait_result = WaitForSingleObject(semaphore, sleep_ms);
    b8 result = (wait_result == WAIT_OBJECT_0);
    return result;
}

function void
os_semaphore_signal(os_semaphore_t semaphore_handle) {
    HANDLE semaphore = (HANDLE)semaphore_handle.id;
    ReleaseSemaphore(semaphore, 1, 0);
}

//- socket functions 

// TODO:



//~ win32 specific functions 

//- key functions 

function os_modifiers 
os_w32_get_modifiers() {
    os_modifiers modifiers = 0;
    if (GetKeyState(VK_CONTROL) & 0x8000) { modifiers |= os_modifier_ctrl; }
    if (GetKeyState(VK_SHIFT) & 0x8000) { modifiers |= os_modifier_shift; }
    if (GetKeyState(VK_MENU) & 0x8000) { modifiers |= os_modifier_alt; }
    return modifiers;
}

function os_key 
os_w32_os_key_from_vkey(WPARAM vkey) {
    os_key result = os_key_null;
    switch (vkey) {
        default: { result = os_key_null; break; }
#define os_w32_key_map(vk, osk) case vk: { result = osk; break; }
        os_w32_key_mappings
#undef os_w32_key_map
    }
    return result;
}

function WPARAM
os_w32_vkey_from_os_key(os_key key) {
    WPARAM result;
    switch (key) {
        default: { result = 0; break; }
#define os_w32_key_map(vk, osk) case osk: { result = vk; break; }
        os_w32_key_mappings
#undef os_w32_key_map
    }
    return result;
}


//- time functions 

function DWORD
os_w32_sleep_ms_from_timeout_us(u64 timeout_us) {
    DWORD sleep_ms = 0;
    if (timeout_us == u64_max) {
        sleep_ms = INFINITE;
    } else {
        u64 begin_ms = os_get_time_ms();
        if (begin_ms < timeout_us) {
            u64 sleep_us = timeout_us - begin_ms;
            sleep_ms = (u32)((sleep_us + 999) / 1000);
        }
    }
    return sleep_ms;
}


//- memory functions 

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

function b8
os_w32_mem_commit(void* ptr, u64 size) {
    u64 page_snapped = size;
    page_snapped += os_page_size() - 1;
    page_snapped -= page_snapped % os_page_size();
    b8 result = VirtualAlloc(ptr, page_snapped, MEM_COMMIT, PAGE_READWRITE) != 0;
    return result;
}

function void
os_w32_mem_decommit(void* ptr, u64 size) {
    //# pragma warning( push )
    //# pragma warning( disable : 6250 )
    VirtualFree(ptr, size, MEM_DECOMMIT);
    //# pragma warning( pop )
}

//- entity functions 

function os_w32_entity_t*
os_w32_entity_alloc(os_w32_entity_type type) {
    
    os_w32_entity_t* entity = nullptr;
    EnterCriticalSection(&os_w32_state.entity_mutex);
    {
        
        // pull from free list or allocate a new one
        entity = os_w32_state.entity_free;
        if (entity != nullptr) {
            stack_pop(os_w32_state.entity_free);
        } else {
            entity = (os_w32_entity_t*)arena_alloc(os_w32_state.arena, sizeof(os_w32_entity_t));
        }
        memset(entity, 0, sizeof(os_w32_entity_t));
    }
    LeaveCriticalSection(&os_w32_state.entity_mutex);
    
    if (entity != nullptr) {
        entity->type = type;
    }
    
    return entity;
}

function void
os_w32_entity_release(os_w32_entity_t* entity) {
    entity->type = os_w32_entity_null;
    EnterCriticalSection(&os_w32_state.entity_mutex);
    {
        // push to free stack
        stack_push(os_w32_state.entity_free, entity);
    }
    LeaveCriticalSection(&os_w32_state.entity_mutex);
}


//- window functions 

function os_window_t 
os_window_handle_from_w32_window(os_w32_window_t* window) {
    os_window_t window_handle = {(u64) window};
    return window_handle;
}

function os_w32_window_t* 
os_w32_window_from_window_handle(os_window_t window_handle) {
    os_w32_window_t* window = (os_w32_window_t*)(window_handle.id);
    return window;
}

function os_w32_window_t* 
os_w32_window_from_hwnd(HWND handle) {
    os_w32_window_t* result = 0;
    for (os_w32_window_t* window = os_w32_state.window_first; window != nullptr; window = window->next) {
        if (window->handle == handle) {
            result = window;
            break;
        }
    }
    return result;
}

function void 
os_w32_window_enable_acrylic_blur(HWND handle, BYTE opacity, COLORREF col) {
    
    // get proc address
    typedef BOOL (WINAPI *set_window_comp_attrib_t)(HWND, WINCOMPATTR_DATA*);
    HMODULE handle_user = LoadLibrary("user32.dll");
    set_window_comp_attrib_t set_window_comp_attrib_func = (set_window_comp_attrib_t)GetProcAddress(handle_user, "SetWindowCompositionAttribute");
    
    ACCENT_POLICY policy = {};
    policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
    policy.GradientColor = (opacity << 24) | (GetBValue(col) << 16) | (GetGValue(col) << 8) | (GetRValue(col)); // AABBGGRR
    
    WINCOMPATTR_DATA data;
    data.Attribute = WCA_ACCENT_POLICY;
    data.Data = &policy;
    data.SizeOfData = sizeof(policy);
    
    set_window_comp_attrib_func(handle, &data);
    
    FreeLibrary(handle_user);
}

//- file functions 

function os_file_flags 
os_w32_file_flags_from_attributes(DWORD attributes) {
    
    os_file_flags flags = 0;
    
    if (attributes & FILE_ATTRIBUTE_READONLY) { flags |= os_file_flag_is_read_only; }
    if (attributes & FILE_ATTRIBUTE_HIDDEN) { flags |= os_file_flag_is_hidden; }
    if (attributes & FILE_ATTRIBUTE_DIRECTORY) { flags |= os_file_flag_is_directory; }
    
    return flags;
}

//- entry points 

function DWORD
os_w32_thread_entry_point(void* ptr) {
    os_w32_entity_t* thread_entity = (os_w32_entity_t*)ptr;
    
    // set the current thread
    os_current_thread = {(u64)thread_entity};
    
    thread_context_create();
    DWORD result = thread_entity->thread.func(thread_entity->thread.params);
    thread_context_release();
    
    os_current_thread = {0};
    
    return result;
}

//- window procedure 

LRESULT CALLBACK
os_w32_window_procedure(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
    
    // TODO: this searches the window list every time
    //we have an event...  lets not do that.
    os_w32_window_t* window = os_w32_window_from_hwnd(handle);
    os_window_t window_handle = os_window_handle_from_w32_window(window);
    
    LRESULT result = 0;
    switch (msg) {
        
        case WM_CLOSE: 
        case WM_DESTROY: {
            os_event_t* event = os_events_push(window_handle, os_event_window_close);
            break;
        }
        
        case WM_PAINT:
        case WM_SIZE: {
            if (window != nullptr) {
                
                // update window size
                UINT width = LOWORD(lparam);
                UINT height = HIWORD(lparam);
                window->size = uvec2(width, height);
                
                // push event
                os_event_t* event = os_events_push(window_handle, os_event_window_resize);
                event->window_position = window->position;
                event->window_size = window->size;
                
                switch (wparam) {
                    
                    // if window was not minimized before and now it is, send minimized event
                    case SIZE_MINIMIZED: {
                        if (window->mode != os_w32_window_mode_minimized && window->mode != os_w32_window_mode_fullscreen) {
                            window->mode = os_w32_window_mode_minimized;
                            os_event_t* event = os_events_push(window_handle, os_event_window_minimize);
                            event->window_position = window->position;
                            event->window_size = window->size;
                        }
                        break;
                    }
                    
                    // if window was not maximized before and now it is, send maximized event
                    case SIZE_MAXIMIZED: {
                        if (window->mode != os_w32_window_mode_maximized && window->mode != os_w32_window_mode_fullscreen) {
                            window->mode = os_w32_window_mode_maximized;
                            os_event_t* event = os_events_push(window_handle, os_event_window_maximize);
                            event->window_position = window->position;
                            event->window_size = window->size;
                        }
                        break;
                    }
                    
                    // if window was maximized or minimized before and now it isn't, send restored event
                    case SIZE_RESTORED: {
                        if (window->mode != os_w32_window_mode_normal) {
                            window->mode = os_w32_window_mode_normal;
                            os_event_t* event = os_events_push(window_handle, os_event_window_restore);
                            event->window_position = window->position;
                            event->window_size = window->size;
                        }
                        break;
                    }
                }
                
                // redraw window
                PAINTSTRUCT ps = { 0 };
                HDC hdc = BeginPaint(handle, &ps);
                if (window->resize_callback_func != nullptr) {
                    window->resize_callback_func();
                }
                
                // draw border
                if (window->flags & os_window_flag_borderless) {
                    //RECT client_rect;
                    //GetClientRect(handle, &client_rect);
                    //RECT titlebar_rect = { 0, 0, client_rect.right, 30 };
                    //FillRect(hdc, &titlebar_rect, (HBRUSH)(COLOR_MENU + 1));
                }
                
                EndPaint(handle, &ps);
                
            }
            break;
        }
        
        case WM_WINDOWPOSCHANGED: {
            if (window != nullptr) {
                
                // get new position and size
                WINDOWPOS* pos = (WINDOWPOS*)lparam;
                window->position = uvec2((u32)pos->x, (u32)pos->y);
                window->size = uvec2((u32)pos->cx, (u32)pos->cy);
                
                os_event_t* event = os_events_push(window_handle, os_event_window_move);
                event->window_position = window->position;
                event->window_size = window->size;
            }
            
            // NOTE: for some reason: "The WM_SIZE and WM_MOVE messages
            // are not sent if an application handles the WM_WINDOWPOSCHANGED 
            // message without calling DefWindowProc."
            // see: https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-windowposchanged#remarks
            result = DefWindowProcA(handle, msg, wparam, lparam);
            
            break;
        }
        
        case WM_SETFOCUS: {
            if (window != nullptr) {
                os_events_push(window_handle, os_event_window_focus);
                window->focused = true;
            }
            break;
        }
        
        case WM_KILLFOCUS: {
            if (window != nullptr) {
                os_events_push(window_handle, os_event_window_unfocus);
                window->focused = false;
            }
            break;
        }
        
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {
            
            os_key key = os_w32_os_key_from_vkey(wparam);
            
            os_event_t* event = os_events_push(window_handle, os_event_key_press);
            event->key = key;
            
            // update key state
            os_core_state.keys[key] = true;
            
            break;
        }
        
        case WM_SYSKEYUP:
        case WM_KEYUP: {
            
            u32 vkey = (u32)wparam;
            os_key key = os_w32_os_key_from_vkey(vkey);
            
            os_event_t* event = os_events_push(window_handle, os_event_key_release);
            event->key = key;
            
            // update key state
            os_core_state.keys[key] = false;
            
            break;
        }
        
        
        case WM_CHAR: {
            
            u32 key = (u32)wparam;
            if (key == '\r') { key = '\n'; }
            if ((key >= 32 && key != 127) || key == '\t' || key == '\n') {
                os_event_t* event = os_events_push(window_handle, os_event_key_text);
                event->keycode = key;
            }
            
            break;
        }
        
        case WM_NCMOUSEMOVE:
        case WM_MOUSEMOVE: {
            
            f32 mouse_x = (f32)(i16)LOWORD(lparam);
            f32 mouse_y = (f32)(i16)HIWORD(lparam);
            
            os_event_t* event = os_events_push(window_handle, os_event_mouse_move);
            event->mouse_position = vec2(mouse_x, mouse_y);
            
            // TODO: figure out mouse enter
            
            break;
        }
        
        case WM_MOUSELEAVE: {
            os_events_push(window_handle, os_event_mouse_leave);
            break;
        }
        
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            SetCapture(handle);
            
            os_event_t* event = os_events_push(window_handle, os_event_mouse_button_press);
            event->mouse_button = os_mouse_button_left;
            switch (msg) {
                case WM_RBUTTONDOWN: event->mouse_button = os_mouse_button_right; break;
                case WM_MBUTTONDOWN: event->mouse_button = os_mouse_button_middle; break;
            }
            
            // update mouse state
            os_core_state.mouse_buttons[event->mouse_button] = true;
            
            break;
        }
        
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            ReleaseCapture();
            
            os_event_t* event = os_events_push(window_handle, os_event_mouse_button_release);
            event->mouse_button = os_mouse_button_left;
            switch (msg) {
                case WM_RBUTTONDOWN: event->mouse_button = os_mouse_button_right; break;
                case WM_MBUTTONDOWN: event->mouse_button = os_mouse_button_middle; break;
            }
            
            // update mouse state
            os_core_state.mouse_buttons[event->mouse_button] = false;
            
            break;
        }
        
        case WM_MOUSEWHEEL: {
            f32 delta = (f32)GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
            os_event_t* event = os_events_push(window_handle, os_event_mouse_scroll);
            event->mouse_scroll = vec2(0.0f, delta);
            break;
        }
        
        //- borderless window messages 
        
        case WM_NCCALCSIZE: {
            if (os_w32_state.new_borderless_window || (window != nullptr && window->flags & os_window_flag_borderless)) {
                
                u32 dpi = GetDpiForWindow(handle);
                i32 frame_x = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
                i32 frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
                i32 padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
                
                DWORD window_style = GetWindowLong(handle, GWL_STYLE);
                b8 is_fullscreen = !(window_style & WS_OVERLAPPEDWINDOW);
                if (!is_fullscreen) {
                    NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lparam;
                    RECT* window_rect = (wparam == 0) ? (RECT*)lparam : params->rgrc;
                    window_rect->right -= frame_x + padding;
                    window_rect->left += frame_x + padding;
                    window_rect->bottom -= frame_y + padding;
                    if (IsZoomed(handle)) {
                        window_rect->top += frame_y + padding;
                        //window_rect->bottom -= 1;
                    }
                }
                
            } else {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            }
            break;
        }
        
        case WM_NCHITTEST: {
            if (os_w32_state.new_borderless_window || (window != nullptr && window->flags & os_window_flag_borderless)) {
                
                b8 is_default_handled = false;
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
                        is_default_handled = true;
                        break;
                    }
                }
                
                if (!is_default_handled) {
                    
                    u32 dpi = GetDpiForWindow(handle);
                    i32 frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
                    i32 padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
                    
                    POINT pos_monitor;
                    pos_monitor.x = LOWORD(lparam);
                    pos_monitor.y = HIWORD(lparam);
                    
                    POINT pos_client = pos_monitor;
                    ScreenToClient(handle, &pos_client);
                    
                    const u32 title_bar_height = 30;
                    
                    if (!IsZoomed(handle) && (pos_client.y > 0) && pos_client.y < frame_y + padding) {
                        result = HTTOP;
                    } else if (pos_client.y >= 0 && pos_client.y < title_bar_height) {
                        result = HTCAPTION;
                    }
                    
                }
                
            } else {
                result = DefWindowProcA(handle, msg, wparam, lparam);
            }
            break;
        }
        
        //- 
        
        default: {
            result = DefWindowProcA(handle, msg, wparam, lparam);
            break;
        }
    }
    
    return result;
}


#endif // SORA_OS_WIN32_CPP