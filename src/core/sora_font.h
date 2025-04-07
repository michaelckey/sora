// sora_font.h

#ifndef SORA_FONT_H
#define SORA_FONT_H

//- defines 

#define font_atlas_size 2048.0f

//- structs

struct font_handle_t {
	u64 data[1];
};

struct font_metrics_t {
	f32 line_gap;
	f32 ascent;
	f32 descent;
	f32 capital_height;
	f32 x_height;
};

struct font_glyph_t {
	font_glyph_t* next;
	font_glyph_t* prev;
    
	u32 hash;
    
	rect_t pos;
	rect_t uv;
	f32 advance;
	f32 height;
};

struct font_raster_t {
	vec2_t size;
	f32 advance;
	u8* data;
};

struct font_atlas_node_t {
	font_atlas_node_t* parent;
	font_atlas_node_t* children[4];
	u32 child_count;
    
	vec2_t max_free_size[4];
	b8 taken;
};

struct font_state_t {
    
	// arena
	arena_t* atlas_arena;
	arena_t* glyph_arena;
    
	// glyph list
	font_glyph_t* glyph_first;
	font_glyph_t* glyph_last;
    
	// atlas
	vec2_t root_size;
	font_atlas_node_t* root;
	gfx_handle_t atlas_texture;
};

//- globals

global font_state_t font_state;

//- functions

// state
function void font_init();
function void font_release();
function void font_reset();

// handles
function b8 font_handle_equals(font_handle_t a, font_handle_t b);

// interface (implemented per backend)
function font_handle_t  font_open(str_t filepath);
function void           font_close(font_handle_t font);
function font_metrics_t font_get_metrics(font_handle_t font, f32 size);
function font_raster_t  font_glyph_raster(arena_t* arena, font_handle_t font, f32 size, u32 codepoint);

// interface (implemented once)
function font_glyph_t* font_get_glyph(font_handle_t font, f32 size, u32 codepoint);
function f32           font_text_get_width(font_handle_t font, f32 size, str_t string);
function f32           font_text_get_height(font_handle_t font, f32 size, str_t string);
function vec2_t        font_align(str_t text, font_handle_t font, f32 size, rect_t rect);

function str_t font_text_truncate(arena_t* arena, font_handle_t font, f32 font_size, str_t string, f32 max_width, str_t trail_string);

// helpers
function u32    font_glyph_hash(font_handle_t, f32, u32);
function vec2_t font_atlas_add(vec2_t);

//- per backend incldues 

#if OS_BACKEND_WIN32
#    if !defined(FNT_BACKEND_DWRITE) && !defined(FNT_BACKEND_FREETYPE)
#        define FNT_BACKEND_DWRITE 1
#    endif
#elif OS_BACKEND_MACOS
#    if !defined(FNT_BACKEND_CORE_TEXT) && !defined(FNT_BACKEND_FREETYPE)
#        define FNT_BACKEND_CORE_TEXT 1
#    endif
#elif OS_BACKEND_LINUX
#    if !defined(FNT_BACKEND_FREETYPE)
#        define FNT_BACKEND_FREETYPE 1
#    endif
#endif 

#if FNT_BACKEND_DWRITE
#include "backends/font/sora_font_dwrite.h"
#elif FNT_BACKEND_CORE_TEXT
// not implemented
#elif FNT_BACKEND_FREETYPE
// not implemented
#endif 

#endif // SORA_FONT_H