// sora_gfx_opengl.cpp

#ifndef SORA_GFX_OPENGL_CPP
#define SORA_GFX_OPENGL_CPP

//- state functions 

function void
gfx_init() {
    
    // create arenas
    gfx_ogl_state.resource_arena = arena_create(megabytes(256));
    gfx_ogl_state.renderer_arena = arena_create(megabytes(64));
    
    // init resource list
    gfx_ogl_state.resource_first = nullptr;
    gfx_ogl_state.resource_last = nullptr;
    gfx_ogl_state.resource_free = nullptr;
    
    // init renderer list
    gfx_ogl_state.renderer_first = nullptr;
    gfx_ogl_state.renderer_last = nullptr;
    gfx_ogl_state.renderer_free = nullptr;
    
#if OS_BACKEND_WIN32
    gfx_ogl_w32_init();
#elif OS_BACKEND_MACOS
    gfx_ogl_macos_init();
#elif OS_BACKEND_LINUX
    gfx_ogl_linux_init();
#endif
    
}

function void
gfx_release() {
    
    // release arenas
    arena_release(gfx_ogl_state.resource_arena);
    arena_release(gfx_ogl_state.renderer_arena);
    
}

function void
gfx_update() {
    // TODO: maybe we don't need this
}

//- renderer 

function gfx_handle_t
gfx_renderer_create(os_handle_t window, color_t clear_color) {
    
	// get from resource pool or create one
	gfx_ogl_renderer_t* renderer = gfx_ogl_state.renderer_free;
	if (renderer != nullptr) {
		stack_pop(gfx_ogl_state.renderer_free);
	} else {
		renderer = (gfx_ogl_renderer_t*)arena_alloc(gfx_ogl_state.renderer_arena, sizeof(gfx_ogl_renderer_t));
	}
	memset(renderer, 0, sizeof(gfx_ogl_renderer_t));
	dll_push_back(gfx_ogl_state.renderer_first, gfx_ogl_state.renderer_last, renderer);
    
    // fill struct
    renderer->window = window;
    renderer->clear_color = clear_color;
    renderer->resolution = os_window_get_size(window);
    
    os_w32_window_t* w32_window = os_w32_window_from_handle(window);
    
    // set pixel format
    HDC dc = GetDC(w32_window->handle);
    PIXELFORMATDESCRIPTOR format_desc = {0};
    SetPixelFormat(dc, gfx_ogl_state.format_idx, &format_desc);
    ReleaseDC(w32_window->handle, dc);
    
    gfx_handle_t handle = { (u64)renderer };
    
    return handle;
}

function void 
gfx_renderer_release(gfx_handle_t renderer) {
    
}

function void
gfx_renderer_resize(gfx_handle_t renderer, uvec2_t size) {
    
}

function void
gfx_renderer_begin(gfx_handle_t renderer) {
    
    
#if OS_BACKEND_WIN32
    
#elif OS_BACKEND_MACOS
    
#elif OS_BACKEND_LINUX
    
#endif 
    
    gfx_ogl_renderer_t* ogl_renderer = (gfx_ogl_renderer_t*)(renderer.data[0]);
    os_w32_window_t* w32_window = os_w32_window_from_handle( ogl_renderer->window);
    ogl_renderer->dc = GetDC(w32_window->handle);
    w32_wglMakeCurrent(ogl_renderer->dc, gfx_ogl_state.rc);
    
    glClearColor(ogl_renderer->clear_color.r, ogl_renderer->clear_color.g, ogl_renderer->clear_color.b,  1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

function void 
gfx_renderer_end(gfx_handle_t renderer) {
    gfx_ogl_renderer_t* ogl_renderer = (gfx_ogl_renderer_t*)(renderer.data[0]);
    os_w32_window_t* w32_window = os_w32_window_from_handle( ogl_renderer->window);
    SwapBuffers(ogl_renderer->dc);
    ReleaseDC( w32_window->handle, ogl_renderer->dc);
}

//- per os specific opengl function 

// win32
#if OS_BACKEND_WIN32

// opengl procs
#define GL_FUNCS \
X(glGetError, GLenum, (void))\
X(glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height))\
X(glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height))\
X(glClearColor, void, (GLfloat r,GLfloat g,GLfloat b,GLfloat a))\
X(glClear, void, (GLbitfield mask))\
X(glBlendFunc, void, (GLenum sfactor, GLenum dfactor))\
X(glBlendFuncSeparate, void, (GLenum srcRGB,GLenum dstRGB,GLenum srcAlpha,GLenum dstAlpha))\
X(glBlendEquation, void, (GLenum mode))\
X(glBlendEquationSeparate, void, (GLenum modeRGB, GLenum modeAlpha))\
X(glDisable, void, (GLenum cap))\
X(glEnable, void, (GLenum cap))\
X(glPixelStorei, void, (GLenum pname, GLint param))\
X(glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels))\
X(glGenBuffers, void, (GLsizei n, GLuint *buffers))\
X(glDeleteBuffers, void, (GLsizei n, const GLuint *buffers))\
X(glBindBuffer, void, (GLenum target, GLuint buffer))\
X(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))\
X(glBufferSubData, void, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data))\
X(glGenVertexArrays, void, (GLsizei n, GLuint *arrays))\
X(glDeleteVertexArrays, void, (GLsizei n, const GLuint *arrays))\
X(glBindVertexArray, void, (GLuint array))\
X(glActiveTexture, void, (GLenum texture))\
X(glGenTextures, void, (GLsizei n, GLuint *textures))\
X(glDeleteTextures, void, (GLsizei n, const GLuint *textures))\
X(glBindTexture, void, (GLenum target, GLuint texture))\
X(glIsTexture, GLboolean, (GLuint texture))\
X(glTexParameteri, void, (GLenum target, GLenum pname, GLint param))\
X(glTexImage1D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels))\
X(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels))\
X(glTexSubImage1D, void, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels))\
X(glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))\
X(glTexImage2DMultisample, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations))\
X(glAttachShader, void, (GLuint program, GLuint shader))\
X(glCompileShader, void, (GLuint shader))\
X(glCreateProgram, GLuint, (void))\
X(glCreateShader, GLuint, (GLenum type))\
X(glDeleteProgram, void, (GLuint program))\
X(glDeleteShader, void, (GLuint shader))\
X(glGetProgramiv, void, (GLuint program, GLenum pname, GLint *params))\
X(glGetProgramInfoLog, void, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog))\
X(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint *params))\
X(glGetShaderInfoLog, void, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog))\
X(glLinkProgram, void, (GLuint program))\
X(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length))\
X(glUseProgram, void, (GLuint program))\
X(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))\
X(glDrawElements, void, (GLenum mode, GLsizei count, GLenum type, const void *indices))\
X(glDrawArraysInstanced, void, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount))\
X(glDrawElementsInstanced, void, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount))\
X(glEnableVertexAttribArray, void, (GLuint index))\
X(glDisableVertexAttribArray, void, (GLuint index))\
X(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))\
X(glVertexAttribIPointer, void, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer))\
X(glVertexAttribDivisor, void, (GLuint index, GLuint divisor))\
X(glGetUniformLocation, GLint, (GLuint program, const GLchar *name))\
X(glUniform1f, void, (GLint location, GLfloat v0))\
X(glUniform2f, void, (GLint location, GLfloat v0, GLfloat v1))\
X(glUniform3f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2))\
X(glUniform4f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))\
X(glUniform1i, void, (GLint location, GLint v0))\
X(glUniform1fv, void, (GLint location, GLsizei count, const GLfloat *value))\
X(glUniform2fv, void, (GLint location, GLsizei count, const GLfloat *value))\
X(glUniform3fv, void, (GLint location, GLsizei count, const GLfloat *value))\
X(glDrawBuffers, void, (GLsizei n, const GLenum *bufs))\
X(glGenFramebuffers, void, (GLsizei n, GLuint *framebuffers))\
X(glDeleteFramebuffers, void, (GLsizei n, const GLuint *framebuffers))\
X(glBindFramebuffer, void, (GLenum target, GLuint framebuffer))\
X(glIsFramebuffer, GLboolean, (GLuint framebuffer))\
X(glCheckFramebufferStatus, GLenum, (GLenum target))\
X(glFramebufferTexture1D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))\
X(glFramebufferTexture2D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))\
X(glFramebufferTexture3D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))\
X(glFramebufferRenderbuffer, void, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer))\
X(glFramebufferTexture, void, (GLenum target, GLenum attachment, GLuint texture, GLint level))\
X(glBlitFramebuffer, void, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))\
X(glGetFramebufferAttachmentParameteriv, void, (GLenum target, GLenum attachment, GLenum pname, GLint *params))\

// wgl procs
#define WGL_FUNCS \
X(wglCreateContext,  HGLRC, (HDC dc))\
X(wglDeleteContext,  BOOL,  (HGLRC glrc))\
X(wglMakeCurrent,    BOOL,  (HDC dc,HGLRC glrc))\
X(wglGetProcAddress, PROC,  (LPCSTR name))\

// wgl ext provs
#define WGL_EXT_FUNCS \
X(wglChoosePixelFormatARB, BOOL, (HDC dc,const int*atri,const FLOAT*atrf,UINT max_fmts,int*fmts,UINT *num_fmts))\
X(wglCreateContextAttribsARB, HGLRC, (HDC dc,HGLRC share,const int*atri))\

// generate opengl typedef functions
#define X(name, return_type, params) typedef return_type name##_func params;
GL_FUNCS
#undef X

// generate opengl function pointers
#define X(name, return_type, params) global name##_func* name = 0;
GL_FUNCS
#undef X

// generate wgl type functions
#define X(name, return_type, params) typedef return_type name##_func params;
WGL_FUNCS
WGL_EXT_FUNCS
#undef X

// generate wgl function pointers
#define X(name, return_type, parmas) global name##_func* w32_##name = 0;
WGL_FUNCS
WGL_EXT_FUNCS
#undef X


function PROC
gfx_ogl_w32_load(HMODULE module, char *name){
    PROC result = (PROC)GetProcAddress(module, name);
    if (result == nullptr){
        result = (PROC)w32_wglGetProcAddress(name);
    }
    return(result);
}

function void 
gfx_ogl_w32_init() {
    
    // get hinstance
    HINSTANCE hinstance = GetModuleHandle(NULL);
    
    // load opengl module
    HMODULE w32_opengl_module = LoadLibraryA("opengl32.dll");
    if (w32_opengl_module == 0){
        printf("[error] failed to load opengl32.dll!\n");
    }
    
    // load wgl functions
#define X(name, return_type, params) (*(PROC*)(&(w32_##name))) = GetProcAddress((w32_opengl_module),(#name));
    WGL_FUNCS
#undef X
    
    b8 missing_wgl_func = false;
#define X(name, return, params) if (w32_##name == 0){ missing_wgl_func = true; }
    WGL_FUNCS
#undef X
    
    if (missing_wgl_func){
        printf("[error] failed to load wgl functions!\n");
    }
    
    // register bootstrap class
    WNDCLASS bs_window_class = { 0 };
    bs_window_class.lpfnWndProc = DefWindowProc;
    bs_window_class.hInstance = hinstance;
    bs_window_class.lpszClassName = "sora_bootstrap_window_class";
    RegisterClassA(&bs_window_class);
    
    // create bootstrap window
    HWND bootstrap_window = CreateWindow("sora_bootstrap_window_class", "bootstrap_window", 0, 0, 0, 0, 0, 0, 0, hinstance, 0);
    HDC dc = GetDC(bootstrap_window);
    
    // create bootstrap context
    HGLRC hglrc = 0;
    
    PIXELFORMATDESCRIPTOR format_desc = {0};
    format_desc.nSize = sizeof(format_desc);
    format_desc.nVersion = 1;
    format_desc.dwFlags = PFD_SUPPORT_OPENGL;
    format_desc.cColorBits  = 24;
    format_desc.cRedBits    = 8;
    format_desc.cRedShift   = 0;
    format_desc.cGreenBits  = 8;
    format_desc.cGreenShift = 8;
    format_desc.cBlueBits   = 8;
    format_desc.cBlueShift  = 16;
    
    int format_idx = ChoosePixelFormat(dc, &format_desc);
    if (format_idx != 0){
        BOOL spf = SetPixelFormat(dc, format_idx, &format_desc);
        if (spf){
            hglrc = w32_wglCreateContext(dc);
        }
    }
    
    if (hglrc == 0){
        printf("[error] failed to create bootstrap context!\n");
    }
    
    // load wgl extension functions
    w32_wglMakeCurrent(dc, hglrc);
#define X(name, return_type, params) (*(PROC*)(&(w32_##name))) = w32_wglGetProcAddress(#name);
    WGL_EXT_FUNCS
#undef X
    w32_wglMakeCurrent(0, 0);
    
    b8 missing_wgl_ext_func = false;
#define X(name, return, params) if (w32_##name == 0) { missing_wgl_ext_func = true; }
    WGL_EXT_FUNCS
#undef X
    
    if (missing_wgl_ext_func) {
        printf("[error] failed to load wgl extension functions!\n");
    }
    
    // cleanup
    BOOL delete_context = w32_wglDeleteContext(hglrc);
    ReleaseDC(bootstrap_window, dc);
    BOOL destroy_window = DestroyWindow(bootstrap_window);
    BOOL unregister = UnregisterClass("sora_bootstrap_window_class", hinstance);
    
    // dummy window class
    WNDCLASS dummy_window_class = {0};
    dummy_window_class.lpfnWndProc = DefWindowProc;
    dummy_window_class.hInstance = hinstance;
    dummy_window_class.lpszClassName = "dummy_window_class";
    RegisterClass(& dummy_window_class);
    
    gfx_ogl_state.format_idx = 0;
    
    // create dummy window
    HWND dummy_window = CreateWindow("dummy_window_class", "DUMMY", WS_TILEDWINDOW,
                                     CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,
                                     0, 0, hinstance, 0);
    
    
    dc = GetDC(dummy_window);
    
    // choose pixel format
    int format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
        WGL_SUPPORT_OPENGL_ARB, TRUE,
        WGL_DOUBLE_BUFFER_ARB, TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,  24,
        WGL_RED_BITS_ARB,     8,
        WGL_GREEN_BITS_ARB,   8,
        WGL_BLUE_BITS_ARB,    8,
        0
    };
    
    UINT num_formats = 0;
    BOOL cpf = w32_wglChoosePixelFormatARB(dc, format_attribs, 0,
                                           1, &gfx_ogl_state.format_idx, &num_formats);
    if (cpf && num_formats > 0){
        
        // set pixel format
        PIXELFORMATDESCRIPTOR format_desc = {0};
        BOOL spf = SetPixelFormat(dc, gfx_ogl_state.format_idx, &format_desc);
        if (spf){
            
            // create context
            int attribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
            };
            
            gfx_ogl_state.rc = w32_wglCreateContextAttribsARB(dc, 0, attribs);
        }
    }
    
    
    //- load & check gl functions
    w32_wglMakeCurrent(dc, gfx_ogl_state.rc);
#define X(name, return_type, params) (*(PROC*)(&(name))) = gfx_ogl_w32_load((w32_opengl_module),( #name));
    GL_FUNCS
#undef X
    w32_wglMakeCurrent(0, 0);
    
    b8 missing_gl_func = false;
#define X(name, return_type, params) if (name == 0){ missing_gl_func = true; }
    GL_FUNCS
#undef X
    
    if (missing_gl_func){
        printf("[error] failed to load opengl functions!\n");
    }
    
    // cleanup
    ReleaseDC(dummy_window, dc);
    DestroyWindow(dummy_window);
    
    
}


// macos
#elif OS_BACKEND_MACOS


// linux
#elif OS_BACKEND_LINUX


#endif 


#endif // SORA_GFX_OPENGL_CPP
