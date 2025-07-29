// sora_font.cpp

#ifndef SORA_FONT_CPP
#define SORA_FONT_CPP

//~ implementation

//- core state functions

function void
_font_core_init() {
    
	font_core_state.arena = arena_create(megabytes(256));
    
    font_core_state.atlas_size = uvec2(2048, 2048);
	
    // atlas nodes
	font_core_state.root_size = vec2((f32)font_core_state.atlas_size.x, (f32)font_core_state.atlas_size.y);
	font_core_state.root = (font_atlas_node_t*)arena_alloc(font_core_state.arena, sizeof(font_atlas_node_t));
	font_core_state.root->max_free_size[0] = vec2_mul(font_core_state.root_size, 0.5f);
	font_core_state.root->max_free_size[1] = vec2_mul(font_core_state.root_size, 0.5f);
	font_core_state.root->max_free_size[2] = vec2_mul(font_core_state.root_size, 0.5f);
	font_core_state.root->max_free_size[3] = vec2_mul(font_core_state.root_size, 0.5f);
    
	// atlas texture
	//font_core_state.atlas_texture = gfx_texture_create(font_core_state.atlas_size);
	
}

function void
_font_core_release() {
    
	// release assets
	//gfx_texture_release(font_core_state.atlas_texture);
    
	// release arena
	arena_release(font_core_state.arena);
    
}

function void
_font_core_reset() {
    
    // clear arenas
    arena_clear(font_core_state.arena);
    
    // glyph cache
	font_core_state.glyph_first = nullptr;
	font_core_state.glyph_last = nullptr;
    
    // atlas nodes
	font_core_state.root = (font_atlas_node_t*)arena_calloc(font_core_state.arena, sizeof(font_atlas_node_t));
	font_core_state.root->max_free_size[0] = vec2_mul(font_core_state.root_size, 0.5f);
	font_core_state.root->max_free_size[1] = vec2_mul(font_core_state.root_size, 0.5f);
	font_core_state.root->max_free_size[2] = vec2_mul(font_core_state.root_size, 0.5f);
	font_core_state.root->max_free_size[3] = vec2_mul(font_core_state.root_size, 0.5f);
    
}

//- handle functions

function b8
font_equals(font_t a, font_t b) {
	return (a.id == b.id);
}

//- interface functions

function font_glyph_t*
font_get_glyph(font_t font, f32 size, u32 codepoint) {
    
	temp_t scratch = scratch_begin();
    
	font_glyph_t* glyph = nullptr;
	u32 hash = _font_glyph_hash(font, size, codepoint);
    
	// try to find glyph in cache
	for (font_glyph_t* current = font_core_state.glyph_first; current != 0; current = current->next) {
        
		// we found a match
		if (current->hash == hash) {
			glyph = current;
			break;
		}
	}
    
	// if we did not find a match, add to cache
	if (glyph == nullptr) {
        
		// raster the glyph on scratch arena
		font_raster_t raster = font_glyph_raster(scratch.arena, font, size, codepoint);
		
		// add to atlas
		vec2_t atlas_glyph_pos = _font_atlas_add(raster.size);
		vec2_t atlas_glyph_size = vec2_add(atlas_glyph_pos, raster.size);
		rect_t region = { atlas_glyph_pos.x, atlas_glyph_pos.y, atlas_glyph_size.x, atlas_glyph_size.y };
		//gfx_texture_fill_region(font_core_state.atlas_texture, region, raster.data);
        
		// add glyph to cache list
		glyph = (font_glyph_t*)arena_calloc(font_core_state.arena, sizeof(font_glyph_t));
		glyph->hash = hash;
		glyph->advance = raster.advance;
		glyph->pos = rect(0.0f, 0.0f, raster.size.x, raster.size.y);
		glyph->uv = rect(region.x0 / (f32)font_core_state.atlas_size.x, 
                         region.y0 / (f32)font_core_state.atlas_size.y, 
                         region.x1 / (f32)font_core_state.atlas_size.x,
                         region.y1 / (f32)font_core_state.atlas_size.y);
		dll_push_back(font_core_state.glyph_first, font_core_state.glyph_last, glyph);
        
	}
    
	scratch_end(scratch);
    
	return glyph;
}

function f32 
font_text_get_width(font_t font, f32 size, str_t string) {
	f32 width = 0.0f;
	for (u32 offset = 0; offset < string.size; offset++) {
		char c = *(string.data + offset);
		font_glyph_t* glyph = font_get_glyph(font, size, (u8)c);
		width += glyph->advance;
	}
	return width;
}

function f32 
font_text_get_height(font_t font, f32 size, str_t string) {
	font_metrics_t metrics = font_get_metrics(font, size);
	f32 h = (metrics.ascent + metrics.descent);
	return h;
}


function vec2_t 
font_align(str_t text, font_t font, f32 size, rect_t rect) {
    
    vec2_t result = { 0 };
    
    font_metrics_t font_metrics = font_get_metrics(font, size);
    f32 text_height = font_text_get_height(font, size, text);
    result.y = roundf(rect.y0 + (rect.y1 - rect.y0 - (text_height)) / 2.0f);
    
    f32 text_width = font_text_get_width(font, size, text);
    result.x = roundf((rect.x0 + rect.x1 - text_width) * 0.5f);
    result.x = max(result.x, rect.x0 + 4.0f);
    
    result.x = floorf(result.x);
    return result;
}

//function str_t 
//font_text_truncate(arena_t* arena, font_t font, f32 font_size, str_t string, f32 max_width, str_t trail_string) {
//
// copy original string
//char* buffer = (char*)arena_alloc(arena, sizeof(char) * string.size);
//memcpy(buffer, string.data, string.size);
//str_t result = { 0 };
//
//f32 trail_width = font_text_get_width(font, font_size, trail_string);
//
//f32 width = 0.0f;
//i32 glyph_count = 0;
//b8 trail_needed = false;
//
//for (; glyph_count < string.size; glyph_count++) {
//char c = *(string.data + glyph_count);
//font_glyph_t* glyph = font_get_glyph(font, font_size, (u8)c);
//width += glyph->advance;
//
//if (width > max_width) {
//trail_needed = true;
//break;
//}
//}
//
//u32 needed_size = string.size;
//
//if (trail_needed) {
//glyph_count -= trail_string.size;
//needed_size = glyph_count + trail_string.size;
//
// copy string
//memcpy(buffer + glyph_count, trail_string.data, sizeof(char) * trail_string.size);
//
//}
//
//result = str(buffer, needed_size);
//
//return result;
//}

//~ internal functions

function u32
_font_glyph_hash(font_t font, f32 size, u32 codepoint) {
	u32 hash = 2166136261u;
	hash ^= font.id;
	hash *= 16777619u;
	hash ^= *(u32*)(&size);
	hash *= 16777619u;
	hash ^= codepoint;
	hash *= 16777619u;
	return hash;
}

function vec2_t
_font_atlas_add(vec2_t needed_size) {
    
	// find node with best-fit size
	vec2_t region_p0 = { 0.0f, 0.0f };
	vec2_t region_size = { 0.0f, 0.0f };
    
	font_atlas_node_t* node = nullptr;
	i32 node_corner = -1;
    
	vec2_t n_supported_size = font_core_state.root_size;
    
	const vec2_t corner_vertices[4] = {
		vec2(0.0f, 0.0f),
		vec2(0.0f, 1.0f),
		vec2(1.0f, 0.0f),
		vec2(1.0f, 1.0f),
	};
    
	for (font_atlas_node_t* n = font_core_state.root, *next = 0; n != 0; n = next, next = 0) {
        
		if (n->taken) { break; }
        
		b8 n_can_be_allocated = (n->child_count == 0);
        
		if (n_can_be_allocated) {
			region_size = n_supported_size;
		}
        
		vec2_t child_size = vec2_mul(n_supported_size, 0.5f);
        
		font_atlas_node_t* best_child = nullptr;
		if (child_size.x >= needed_size.x && child_size.y >= needed_size.y) {
			for (i32 i = 0; i < 4; i++) {
                
				if (n->children[i] == 0) {
					n->children[i] = (font_atlas_node_t*)arena_calloc(font_core_state.arena, sizeof(font_atlas_node_t));
					n->children[i]->parent = n;
					n->children[i]->max_free_size[0] = vec2_mul(child_size, 0.5f);
					n->children[i]->max_free_size[1] = vec2_mul(child_size, 0.5f);
					n->children[i]->max_free_size[2] = vec2_mul(child_size, 0.5f);
					n->children[i]->max_free_size[3] = vec2_mul(child_size, 0.5f);
				}
                
				if (n->max_free_size[i].x >= needed_size.x && n->max_free_size[i].y >= needed_size.y) {
					best_child = n->children[i];
					node_corner = i;
					vec2_t side_vertex = corner_vertices[i];
					region_p0.x += side_vertex.x * child_size.x;
					region_p0.y += side_vertex.y * child_size.y;
					break;
				}
			}
		}
        
		if (n_can_be_allocated && best_child == 0) {
			node = n;
		} else {
			next = best_child;
			n_supported_size = child_size;
		}
        
	}
    
	if (node != 0 && node_corner != -1) {
		node->taken = true;
        
		if (node->parent != nullptr) {
			memset(&node->parent->max_free_size[node_corner], 0, sizeof(vec2_t));
		} 
        
        for (font_atlas_node_t* p = node->parent; p != nullptr; p = p->parent) {
			p->child_count += 1;
			font_atlas_node_t* parent = p->parent;
			if (parent != 0) {
				i32 p_corner = 
                (
                 p == parent->children[0] ? 0 :
                 p == parent->children[1] ? 1 :
                 p == parent->children[2] ? 2 :
                 p == parent->children[3] ? 3 :
                 -1
                 );
                
				parent->max_free_size[p_corner].x = 
					max(max(p->max_free_size[0].x,
					        p->max_free_size[1].x),
                        max(p->max_free_size[2].x,
                            p->max_free_size[3].x));
				parent->max_free_size[p_corner].y = 
					max(max(p->max_free_size[0].y,
					        p->max_free_size[1].y),
                        max(p->max_free_size[2].y,
                            p->max_free_size[3].y));
			}
		}
	}
    
	vec2_t result = region_p0;
    
	return result;
    
}

//~ per backend includes

#if FNT_BACKEND_DWRITE
#    include "backends/font/sora_font_dwrite.cpp"
#elif FNT_BACKEND_CORE_TEXT
// not implemented
#elif FNT_BACKEND_FREETYPE
// not implemented
#endif 

#endif // SORA_FONT_CPP
