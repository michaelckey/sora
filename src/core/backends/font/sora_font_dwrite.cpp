// sora_font_dwrite.cpp

#ifndef SORA_FONT_DWRITE_CPP
#define SORA_FONT_DWRITE_CPP

//~ includes

#pragma comment(lib, "dwrite")

//~ implementation

//- state functions

function void
font_init() {
    
    _font_core_init();
    
	// allocate arenas
	font_dwrite_state.arena = arena_create(megabytes(64));
    
	HRESULT hr = 0;
    
	// create dwrite factory
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory2), (IUnknown**)&font_dwrite_state.dwrite_factory2);
	
	// create rendering params
	hr = font_dwrite_state.dwrite_factory2->CreateRenderingParams(&font_dwrite_state.rendering_params);
	
	f32 gamma = font_dwrite_state.rendering_params->GetGamma();
	f32 enhanced_contrast = font_dwrite_state.rendering_params->GetEnhancedContrast();
	f32 clear_type_level = font_dwrite_state.rendering_params->GetClearTypeLevel();
	
    hr = font_dwrite_state.dwrite_factory2->CreateCustomRenderingParams(gamma, enhanced_contrast, enhanced_contrast, 0.0f, 
                                                                        DWRITE_PIXEL_GEOMETRY_FLAT, DWRITE_RENDERING_MODE_GDI_NATURAL, DWRITE_GRID_FIT_MODE_ENABLED,
                                                                        (IDWriteRenderingParams2**)&font_dwrite_state.rendering_params);
	
	// create gdi interop
	hr = font_dwrite_state.dwrite_factory2->GetGdiInterop(&font_dwrite_state.gdi_interop);
    
}

function void
font_release() {
    
    // release dwrite
	if (font_dwrite_state.gdi_interop != nullptr) { font_dwrite_state.gdi_interop->Release();  font_dwrite_state.gdi_interop = nullptr; }
	if (font_dwrite_state.rendering_params != nullptr) { font_dwrite_state.rendering_params->Release(); font_dwrite_state.rendering_params = nullptr; }
	if (font_dwrite_state.dwrite_factory2 != nullptr) { font_dwrite_state.dwrite_factory2->Release(); font_dwrite_state.dwrite_factory2 = nullptr; }
    
	// release arenas
	arena_release(font_dwrite_state.arena);
    
    _font_core_release();
}

function void
font_reset() {
    _font_core_reset();
}

//- font functions

function font_t
font_open(str_t filepath) {
    
	temp_t scratch = scratch_begin();
    
	// get from font pool or create one
	font_dwrite_font_t* font = font_dwrite_state.font_free;
	if (font != nullptr) {
		font = stack_pop(font_dwrite_state.font_free);
	} else {
		font = (font_dwrite_font_t*)arena_alloc(font_dwrite_state.arena, sizeof(font_dwrite_font_t));
	}
	memset(font, 0, sizeof(font_dwrite_font_t));
	dll_push_back(font_dwrite_state.font_first, font_dwrite_state.font_last, font);
    
	// convert to wide path
	str16_t wide_filepath = str16_from_str(scratch.arena, filepath);
    
	// create font file and face
	font_dwrite_state.dwrite_factory2->CreateFontFileReference((WCHAR*)wide_filepath.data, 0, &font->file);
	font_dwrite_state.dwrite_factory2->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE, 1, &font->file, 0, DWRITE_FONT_SIMULATIONS_NONE, &(font->face));
    
	str_t font_name = str_get_file_name(filepath);
	log_infof("successfully opened font: '%.*s'", font_name.size, font_name.data);
    
	scratch_end(scratch);
    
	font_t font_handle = { (u64)font };
	return font_handle;
}

function void
font_close(font_t font_handle) {
    
	font_dwrite_font_t* dwrite_font = (font_dwrite_font_t*)(font_handle.id);
    
	// release dwrite
	if (dwrite_font->face != nullptr) { dwrite_font->face->Release(); dwrite_font->face = nullptr; }
	if (dwrite_font->file!= nullptr) { dwrite_font->file->Release(); dwrite_font->file = nullptr; }
    
	// push to free stack
	dll_remove(font_dwrite_state.font_first, font_dwrite_state.font_last, dwrite_font);
	stack_push(font_dwrite_state.font_free, dwrite_font);
}

function font_metrics_t 
font_get_metrics(font_t font_handle, f32 size) {
    
	font_dwrite_font_t* dwrite_font = (font_dwrite_font_t*)(font_handle.id);
    
	DWRITE_FONT_METRICS metrics = { 0 };
	dwrite_font->face->GetMetrics(&metrics);
    
	f32 pixel_per_em = size * (96.0f / 72.0f); // we assume dpi = 96.0f
	f32 pixel_per_design_unit = pixel_per_em / ((f32)metrics.designUnitsPerEm);
    
	font_metrics_t result = { 0 };
	result.line_gap = (f32)metrics.lineGap * pixel_per_design_unit;
	result.ascent = (f32)metrics.ascent * pixel_per_design_unit;
	result.descent = (f32)metrics.descent * pixel_per_design_unit;
	result.capital_height = (f32)metrics.capHeight * pixel_per_design_unit;
	result.x_height =  (f32)metrics.xHeight * pixel_per_design_unit;
    
	return result;
}

function font_raster_t 
font_glyph_raster(arena_t* arena, font_t font_handle, f32 size, u32 codepoint) {
    
	// get font 
	font_dwrite_font_t* dwrite_font = (font_dwrite_font_t*)(font_handle.id);
    
	// get font metrics
	DWRITE_FONT_METRICS font_metrics = { 0 };
	dwrite_font->face->GetMetrics(&font_metrics);
	f32 pixel_per_em = size * (96.0f / 72.0f); // we assume dpi = 96.0f
	f32 pixel_per_design_unit = pixel_per_em / ((f32)font_metrics.designUnitsPerEm);
	f32 ascent = (f32)font_metrics.ascent * pixel_per_design_unit;
	f32 descent = (f32)font_metrics.descent * pixel_per_design_unit;
	f32 capital_height = (f32)font_metrics.capHeight * pixel_per_design_unit;
    
	// get glyph indices
	u16 glyph_index;
	dwrite_font->face->GetGlyphIndicesA(&codepoint, 1, &glyph_index);
    
	// get metrics info
	DWRITE_GLYPH_METRICS glyph_metrics = { 0 };
	dwrite_font->face->GetGdiCompatibleGlyphMetrics(size, 1.0f, 0, 1, &glyph_index, 1, &glyph_metrics, 0);
    
	// determine atlas size
	i32 atlas_dim_x = (i32)(glyph_metrics.advanceWidth * pixel_per_design_unit);
	i32 atlas_dim_y = (i32)((font_metrics.ascent + font_metrics.descent) * pixel_per_design_unit);
	f32 advance = (f32)glyph_metrics.advanceWidth * pixel_per_design_unit + 1.0f;
	atlas_dim_x += 7;
	atlas_dim_x -= atlas_dim_x % 8;
	atlas_dim_x += 4;
	
	// make bitmap for rendering
	IDWriteBitmapRenderTarget* render_target;
	font_dwrite_state.gdi_interop->CreateBitmapRenderTarget(0, atlas_dim_x, atlas_dim_y, &render_target);
	HDC dc = render_target->GetMemoryDC();
    
	// draw glyph
	DWRITE_GLYPH_RUN glyph_run = { 0 };
	glyph_run.fontFace = dwrite_font->face;
	glyph_run.fontEmSize = roundf(size * 96.0f / 72.0f);
	glyph_run.glyphCount = 1;
	glyph_run.glyphIndices = &glyph_index;
    
	RECT bounding_box = { 0 };
	vec2_t draw_pos = { 1.0f, (f32)atlas_dim_y - descent };
	render_target->DrawGlyphRun(draw_pos.x, draw_pos.y, DWRITE_MEASURING_MODE_NATURAL, &glyph_run, font_dwrite_state.rendering_params, RGB(255, 255, 255), &bounding_box);
    
	// get bitmap
	DIBSECTION dib = { 0 };
	HBITMAP bitmap = (HBITMAP)GetCurrentObject(dc, OBJ_BITMAP);
	GetObject(bitmap, sizeof(dib), &dib);
    
	// fill raster result
	font_raster_t raster = { 0 };
	raster.size = vec2((f32)atlas_dim_x, (f32)atlas_dim_y);
	raster.advance = (f32)ceilf(advance);
	raster.data = (u8*)arena_alloc(arena, sizeof(u8) * raster.size.x * raster.size.y * 4);
    
	u8* in_data = (u8*)dib.dsBm.bmBits;
	u32 in_pitch = (u32)dib.dsBm.bmWidthBytes;
	u8* out_data = raster.data;
	u32 out_pitch = (u32)raster.size.x * 4;
	
	u8* in_line = (u8*)in_data;
	u8* out_line = out_data;
	for (u32 y = 0; y < (u32)raster.size.y; y += 1) {
		u8* in_pixel = in_line;
		u8* out_pixel = out_line;
		for (u32 x = 0; x < (u32)raster.size.x; x += 1) {
			//u8 alpha = (in_pixel[0] + in_pixel[1] + in_pixel[2]) / 3;
			out_pixel[0] = 255;
			out_pixel[1] = 255;
			out_pixel[2] = 255;
			out_pixel[3] = in_pixel[1];
			in_pixel += 4;
			out_pixel += 4;
		}
		in_line += in_pitch;
		out_line += out_pitch;
	}
    
	render_target->Release();
    
	return raster;
}

//- dwrite specific functions




#endif // SORA_FONT_DWRITE_CPP