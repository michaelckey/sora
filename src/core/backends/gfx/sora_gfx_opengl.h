// sora_gfx_opengl.h

#ifndef SORA_GFX_OPENGL_H
#define SORA_GFX_OPENGL_H

// NOTE: opengl needs to be loaded differently for each os backend.

//- opengl loader

// win32 loader
#if defined(OS_BACKEND_WIN32)

#define __gl_glcorearb_h_ 1
#define __gl_glext_h_ 1
#define __gl_h_ 1
#define __GL_H__ 1

// defines
typedef unsigned int  GLenum;
typedef unsigned int  GLuint;
typedef int  GLsizei;
typedef char  GLchar;
typedef ptrdiff_t  GLintptr;
typedef ptrdiff_t  GLsizeiptr;
typedef double  GLclampd;
typedef unsigned short  GLushort;
typedef unsigned char  GLubyte;
typedef unsigned char  GLboolean;
typedef u64  GLuint64;
typedef double  GLdouble;
typedef unsigned short  GLhalf;
typedef float  GLclampf;
typedef unsigned int  GLbitfield;
typedef signed char  GLbyte;
typedef short  GLshort;
typedef void  GLvoid;
typedef u64  GLint64;
typedef float  GLfloat;
typedef int  GLint;

// constants
#define GL_POINTS         0x0000
#define GL_LINES          0x0001
#define GL_LINE_LOOP      0x0002
#define GL_LINE_STRIP     0x0003
#define GL_TRIANGLES      0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN   0x0006

#define GL_ZERO                0
#define GL_ONE                 1
#define GL_SRC_COLOR           0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA           0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA           0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR           0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE  0x0308

#define GL_NO_ERROR          0
#define GL_INVALID_ENUM      0x0500
#define GL_INVALID_VALUE     0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY     0x0505

#define GL_BLEND 0x0BE2

#define GL_SCISSOR_BOX  0x0C10
#define GL_SCISSOR_TEST 0x0C11

#define GL_UNPACK_SWAP_BYTES  0x0CF0
#define GL_UNPACK_LSB_FIRST   0x0CF1
#define GL_UNPACK_ROW_LENGTH  0x0CF2
#define GL_UNPACK_SKIP_ROWS   0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_ALIGNMENT   0x0CF5
#define GL_PACK_SWAP_BYTES    0x0D00
#define GL_PACK_LSB_FIRST     0x0D01
#define GL_PACK_ROW_LENGTH    0x0D02
#define GL_PACK_SKIP_ROWS     0x0D03
#define GL_PACK_SKIP_PIXELS   0x0D04
#define GL_PACK_ALIGNMENT     0x0D05

#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1

#define GL_BYTE           0x1400
#define GL_UNSIGNED_BYTE  0x1401
#define GL_SHORT          0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT            0x1404
#define GL_UNSIGNED_INT   0x1405
#define GL_FLOAT          0x1406

#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED   0x1903
#define GL_RGB   0x1907
#define GL_RGBA  0x1908

#define GL_NEAREST 0x2600
#define GL_LINEAR  0x2601

#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S     0x2802
#define GL_TEXTURE_WRAP_T     0x2803

#define GL_REPEAT 0x2901

#define GL_FUNC_ADD              0x8006
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_FUNC_SUBTRACT         0x800A
#define GL_MIN                   0x8007
#define GL_MAX                   0x8008

#define GL_RGB8  0x8051
#define GL_RGBA8 0x8058

#define GL_MULTISAMPLE 0x809D

#define GL_BGR  0x80E0
#define GL_BGRA 0x80E1

#define GL_CLAMP_TO_EDGE   0x812F

#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE       0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE     0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE      0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE     0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE     0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE   0x8217
#define GL_FRAMEBUFFER_DEFAULT   0x8218
#define GL_FRAMEBUFFER_UNDEFINED 0x8219

#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A

#define GL_RG         0x8227
#define GL_RG_INTEGER 0x8228
#define GL_R8         0x8229
#define GL_R16        0x822A
#define GL_RG8        0x822B
#define GL_RG16       0x822C
#define GL_R16F       0x822D
#define GL_R32F       0x822E
#define GL_RG16F      0x822F
#define GL_RG32F      0x8230
#define GL_R8I        0x8231
#define GL_R8UI       0x8232
#define GL_R16I       0x8233
#define GL_R16UI      0x8234
#define GL_R32I       0x8235
#define GL_R32UI      0x8236
#define GL_RG8I       0x8237
#define GL_RG8UI      0x8238
#define GL_RG16I      0x8239
#define GL_RG16UI     0x823A
#define GL_RG32I      0x823B
#define GL_RG32UI     0x823C

#define GL_MIRRORED_REPEAT 0x8370

#define GL_TEXTURE0  0x84C0
#define GL_TEXTURE1  0x84C1
#define GL_TEXTURE2  0x84C2
#define GL_TEXTURE3  0x84C3
#define GL_TEXTURE4  0x84C4
#define GL_TEXTURE5  0x84C5
#define GL_TEXTURE6  0x84C6
#define GL_TEXTURE7  0x84C7
#define GL_TEXTURE8  0x84C8
#define GL_TEXTURE9  0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF

#define GL_DEPTH_STENCIL 0x84F9

#define GL_RGBA32F 0x8814
#define GL_RGB32F  0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F  0x881B

#define GL_ARRAY_BUFFER         0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893

#define GL_STREAM_DRAW  0x88E0
#define GL_STREAM_READ  0x88E1
#define GL_STREAM_COPY  0x88E2
#define GL_STATIC_DRAW  0x88E4
#define GL_STATIC_READ  0x88E5
#define GL_STATIC_COPY  0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER   0x8B31

#define GL_DELETE_STATUS   0x8B80
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84

#define GL_SRGB         0x8C40
#define GL_SRGB8        0x8C41
#define GL_SRGB_ALPHA   0x8C42
#define GL_SRGB8_ALPHA8 0x8C43

#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1

#define GL_FRAMEBUFFER_COMPLETE               0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT  0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED            0x8CDD

#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_COLOR_ATTACHMENT0     0x8CE0
#define GL_COLOR_ATTACHMENT1     0x8CE1
#define GL_COLOR_ATTACHMENT2     0x8CE2
#define GL_COLOR_ATTACHMENT3     0x8CE3
#define GL_COLOR_ATTACHMENT4     0x8CE4
#define GL_COLOR_ATTACHMENT5     0x8CE5
#define GL_COLOR_ATTACHMENT6     0x8CE6
#define GL_COLOR_ATTACHMENT7     0x8CE7
#define GL_COLOR_ATTACHMENT8     0x8CE8
#define GL_COLOR_ATTACHMENT9     0x8CE9
#define GL_COLOR_ATTACHMENT10    0x8CEA
#define GL_COLOR_ATTACHMENT11    0x8CEB
#define GL_COLOR_ATTACHMENT12    0x8CEC
#define GL_COLOR_ATTACHMENT13    0x8CED
#define GL_COLOR_ATTACHMENT14    0x8CEE
#define GL_COLOR_ATTACHMENT15    0x8CEF
#define GL_COLOR_ATTACHMENT16    0x8CF0
#define GL_COLOR_ATTACHMENT17    0x8CF1
#define GL_COLOR_ATTACHMENT18    0x8CF2
#define GL_COLOR_ATTACHMENT19    0x8CF3
#define GL_COLOR_ATTACHMENT20    0x8CF4
#define GL_COLOR_ATTACHMENT21    0x8CF5
#define GL_COLOR_ATTACHMENT22    0x8CF6
#define GL_COLOR_ATTACHMENT23    0x8CF7
#define GL_COLOR_ATTACHMENT24    0x8CF8
#define GL_COLOR_ATTACHMENT25    0x8CF9
#define GL_COLOR_ATTACHMENT26    0x8CFA
#define GL_COLOR_ATTACHMENT27    0x8CFB
#define GL_COLOR_ATTACHMENT28    0x8CFC
#define GL_COLOR_ATTACHMENT29    0x8CFD
#define GL_COLOR_ATTACHMENT30    0x8CFE
#define GL_COLOR_ATTACHMENT31    0x8CFF
#define GL_DEPTH_ATTACHMENT      0x8D00
#define GL_STENCIL_ATTACHMENT    0x8D20

#define GL_FRAMEBUFFER 0x8D40

#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56

#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8

#define GL_FRAMEBUFFER_SRGB 0x8DB9

#define GL_TEXTURE_2D_MULTISAMPLE 0x9100

#define GL_DEPTH_BUFFER_BIT   0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT   0x00004000

// wgl
#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_DRAW_TO_WINDOW_ARB   0x2001
#define WGL_DRAW_TO_BITMAP_ARB   0x2002
#define WGL_ACCELERATION_ARB     0x2003
#define WGL_NEED_PALETTE_ARB     0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB 0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB  0x2006
#define WGL_SWAP_METHOD_ARB      0x2007
#define WGL_NUMBER_OVERLAYS_ARB  0x2008
#define WGL_NUMBER_UNDERLAYS_ARB 0x2009
#define WGL_TRANSPARENT_ARB      0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB   0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB  0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB      0x200C
#define WGL_SHARE_STENCIL_ARB    0x200D
#define WGL_SHARE_ACCUM_ARB      0x200E
#define WGL_SUPPORT_GDI_ARB      0x200F
#define WGL_SUPPORT_OPENGL_ARB   0x2010
#define WGL_DOUBLE_BUFFER_ARB    0x2011
#define WGL_STEREO_ARB           0x2012
#define WGL_PIXEL_TYPE_ARB       0x2013
#define WGL_COLOR_BITS_ARB       0x2014
#define WGL_RED_BITS_ARB         0x2015
#define WGL_RED_SHIFT_ARB        0x2016
#define WGL_GREEN_BITS_ARB       0x2017
#define WGL_GREEN_SHIFT_ARB      0x2018
#define WGL_BLUE_BITS_ARB        0x2019
#define WGL_BLUE_SHIFT_ARB       0x201A
#define WGL_ALPHA_BITS_ARB       0x201B
#define WGL_ALPHA_SHIFT_ARB      0x201C
#define WGL_ACCUM_BITS_ARB       0x201D
#define WGL_ACCUM_RED_BITS_ARB   0x201E
#define WGL_ACCUM_GREEN_BITS_ARB 0x201F
#define WGL_ACCUM_BLUE_BITS_ARB  0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB       0x2022
#define WGL_STENCIL_BITS_ARB     0x2023
#define WGL_AUX_BUFFERS_ARB      0x2024

#define WGL_NO_ACCELERATION_ARB      0x2025
#define WGL_GENERIC_ACCELERATION_ARB 0x2026
#define WGL_FULL_ACCELERATION_ARB    0x2027

#define WGL_SWAP_EXCHANGE_ARB  0x2028
#define WGL_SWAP_COPY_ARB      0x2029
#define WGL_SWAP_UNDEFINED_ARB 0x202A

#define WGL_TYPE_RGBA_ARB       0x202B
#define WGL_TYPE_COLORINDEX_ARB 0x202C

//- WGL_ARB_create_context constants

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB   0x2093
#define WGL_CONTEXT_FLAGS_ARB         0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB              0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define ERROR_INVALID_VERSION_ARB 0x2095
#define ERROR_INVALID_PROFILE_ARB 0x2096

// macos
#elif defined (OS_BACKEND_MACOS)

// NOTE: untested
#    include <TargetConditionals.h>
#    include <OpenGLES/ES3/gl.h>
#    include <OpenGLES/ES3/glext.h>

// linux
#elif defined(OS_BACKEND_LINUX)
// NOTE: untested
#    define GL_GLEXT_PROTOTYPES
#    include <gl/gl.h>
#endif

//- enums 

//- structs 

struct gfx_ogl_buffer_t {
    GLuint id;
};

struct gfx_ogl_texture_t {
    GLuint id;
};

struct gfx_ogl_shader_t {
    GLuint id;
};

struct gfx_ogl_render_target_t {
    GLuint id;
};

struct gfx_ogl_compute_shader_t {
    GLuint id;
};

struct gfx_ogl_resource_t {
    gfx_ogl_resource_t* next;
    gfx_ogl_resource_t* prev;
    
    // resource descriptions
	union {
		gfx_buffer_desc_t buffer_desc;
		gfx_texture_desc_t texture_desc;
		gfx_shader_desc_t shader_desc;
		gfx_compute_shader_desc_t compute_shader_desc;
		gfx_render_target_desc_t render_target_desc;
	};
    
    union {
        gfx_ogl_buffer_t buffer;
        gfx_ogl_texture_t texture;
        gfx_ogl_shader_t shader;
        gfx_ogl_render_target_t render_target;
        gfx_ogl_compute_shader_t compute_shader;
    };
};

struct gfx_ogl_renderer_t {
    gfx_ogl_renderer_t* next;
    gfx_ogl_renderer_t* prev;
    
    // context
    os_handle_t window;
    color_t clear_color;
    uvec2_t resolution;
    
    // win32 
#if OS_BACKEND_WIN32
    HDC dc;
#endif 
    
};

struct gfx_ogl_state_t {
    
    arena_t* resource_arena;
    arena_t* renderer_arena;
    
    // resource list
    gfx_ogl_resource_t* resource_first;
    gfx_ogl_resource_t* resource_last;
    gfx_ogl_resource_t* resource_free;
    
    // renderer list
    gfx_ogl_renderer_t* renderer_first;
    gfx_ogl_renderer_t* renderer_last;
    gfx_ogl_renderer_t* renderer_free;
    
    // win32
#if OS_BACKEND_WIN32
    HGLRC rc;
    i32 format_idx;
#endif 
    
};

//- globals 

global gfx_ogl_state_t gfx_ogl_state;

//- per os specific opengl functions

// win32
#if OS_BACKEND_WIN32

function PROC gfx_ogl_w32_load(HMODULE module, char* name);

function void gfx_ogl_w32_init();
function void gfx_ogl_w32_renderer_create();
function void gfx_ogl_w32_renderer_release();

// macos
#elif OS_BACKEND_MACOS

function void gfx_ogl_macos_init();
function void gfx_ogl_macos_renderer_create();
function void gfx_ogl_macos_renderer_release();

// linux
#elif OS_BACKEND_LINUX

function void gfx_ogl_linux_init();
function void gfx_ogl_linux_renderer_create();
function void gfx_ogl_linux_renderer_release();

#endif

#endif // SORA_GFX_OPENGL_H