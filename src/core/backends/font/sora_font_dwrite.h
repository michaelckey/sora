// sora_font_dwrite.h

#ifndef SORA_FONT_DWRITE_H
#define SORA_FONT_DWRITE_H

//~ includes

#include <dwrite_2.h>

//~ structs

struct font_dwrite_font_t {
	font_dwrite_font_t* next;
	font_dwrite_font_t* prev;
    
	IDWriteFontFile* file;
	IDWriteFontFace* face;
};

struct font_dwrite_state_t {
    
	// arena
	arena_t* arena;
	
	// dwrite
	IDWriteFactory2* dwrite_factory2;
	IDWriteRenderingParams* rendering_params;
	IDWriteGdiInterop* gdi_interop;
    
	// font pool
	font_dwrite_font_t* font_first;
	font_dwrite_font_t* font_last;
	font_dwrite_font_t* font_free;
	
};

//~ globals

global font_dwrite_state_t font_dwrite_state;

//~ dwrite specific functions



#endif // SORA_FONT_DWRITE