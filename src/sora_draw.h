// sora_draw.h

#ifndef SORA_DRAW_H
#define SORA_DRAW_H

//- defines

#define draw_max_clip_rects 128
#define draw_max_textures 16

//- enums

enum draw_shape {
	draw_shape_none,
	draw_shape_rect,
	draw_shape_quad,
	draw_shape_line,
	draw_shape_circle,
	draw_shape_ring,
	draw_shape_tri,
};

//- structs

struct draw_constants_t {
	vec2_t window_size;
	vec2_t padding;
	rect_t clip_masks[draw_max_clip_rects];
};

struct draw_instance_t {
	rect_t bbox;
	rect_t tex;
	vec2_t point0;
	vec2_t point1;
	vec2_t point2;
	vec2_t point3;
	vec4_t color0;
	vec4_t color1;
	vec4_t color2;
	vec4_t color3;
	vec4_t radii;
	f32 thickness;
	f32 softness;
	u32 indices;
};

struct draw_batch_t {
	draw_batch_t* next;
	draw_batch_t* prev;
	
	draw_instance_t* instances;
	u32 instance_count;
};

// stacks
struct draw_color0_node_t { draw_color0_node_t* next; color_t v; };
struct draw_color0_stack_t { draw_color0_node_t* top; draw_color0_node_t* free; b8 auto_pop; };

struct draw_color1_node_t { draw_color1_node_t* next; color_t v; };
struct draw_color1_stack_t { draw_color1_node_t* top; draw_color1_node_t* free; b8 auto_pop; };

struct draw_color2_node_t { draw_color2_node_t* next; color_t v; };
struct draw_color2_stack_t { draw_color2_node_t* top; draw_color2_node_t* free; b8 auto_pop; };

struct draw_color3_node_t { draw_color3_node_t* next; color_t v; };
struct draw_color3_stack_t { draw_color3_node_t* top; draw_color3_node_t* free; b8 auto_pop; };

struct draw_radius0_node_t { draw_radius0_node_t* next; f32 v; };
struct draw_radius0_stack_t { draw_radius0_node_t* top; draw_radius0_node_t* free; b8 auto_pop; };

struct draw_radius1_node_t { draw_radius1_node_t* next; f32 v; };
struct draw_radius1_stack_t { draw_radius1_node_t* top; draw_radius1_node_t* free; b8 auto_pop; };

struct draw_radius2_node_t { draw_radius2_node_t* next; f32 v; };
struct draw_radius2_stack_t { draw_radius2_node_t* top; draw_radius2_node_t* free; b8 auto_pop; };

struct draw_radius3_node_t { draw_radius3_node_t* next; f32 v; };
struct draw_radius3_stack_t { draw_radius3_node_t* top; draw_radius3_node_t* free; b8 auto_pop; };

struct draw_thickness_node_t { draw_thickness_node_t* next; f32 v; };
struct draw_thickness_stack_t { draw_thickness_node_t* top; draw_thickness_node_t* free; b8 auto_pop; };

struct draw_softness_node_t { draw_softness_node_t* next; f32 v; };
struct draw_softness_stack_t { draw_softness_node_t* top; draw_softness_node_t* free; b8 auto_pop; };

struct draw_font_node_t { draw_font_node_t* next; font_handle_t v; };
struct draw_font_stack_t { draw_font_node_t* top; draw_font_node_t* free; b8 auto_pop; };

struct draw_font_size_node_t { draw_font_size_node_t* next; f32 v; };
struct draw_font_size_stack_t { draw_font_size_node_t* top; draw_font_size_node_t* free; b8 auto_pop; };

struct draw_clip_mask_node_t { draw_clip_mask_node_t* next; rect_t v; };
struct draw_clip_mask_stack_t { draw_clip_mask_node_t* top; draw_clip_mask_node_t* free; b8 auto_pop; };

struct draw_texture_node_t { draw_texture_node_t* next; gfx_handle_t v; };
struct draw_texture_stack_t { draw_texture_node_t* top; draw_texture_node_t* free; b8 auto_pop; };

struct draw_state_t {
    
	// assets
	gfx_handle_t instance_buffer;
	gfx_handle_t constant_buffer;
	draw_constants_t constants;
	i32 clip_mask_count;
	gfx_pipeline_t pipeline;
	gfx_handle_t shader;
	gfx_handle_t texture;
	font_handle_t font;	
	
	gfx_handle_t texture_list[draw_max_textures];
	u32 texture_count;
    
	// batches
	arena_t* batch_arena;
	draw_batch_t* batch_first;
	draw_batch_t* batch_last;
    
	// stacks
	draw_color0_stack_t color0_stack;
	draw_color1_stack_t color1_stack;
	draw_color2_stack_t color2_stack;
	draw_color3_stack_t color3_stack;
    
	draw_radius0_stack_t radius0_stack;
	draw_radius1_stack_t radius1_stack;
	draw_radius2_stack_t radius2_stack;
	draw_radius3_stack_t radius3_stack;
    
	draw_thickness_stack_t thickness_stack;
	draw_softness_stack_t softness_stack;
    
	draw_font_stack_t font_stack;
	draw_font_size_stack_t font_size_stack;
    
	draw_clip_mask_stack_t clip_mask_stack;
    
	draw_texture_stack_t texture_stack;
    
	// stack defaults
	draw_color0_node_t color0_default_node;
	draw_color1_node_t color1_default_node;
	draw_color2_node_t color2_default_node;
	draw_color3_node_t color3_default_node;
    
	draw_radius0_node_t radius0_default_node;
	draw_radius1_node_t radius1_default_node;
	draw_radius2_node_t radius2_default_node;
	draw_radius3_node_t radius3_default_node;
    
	draw_thickness_node_t thickness_default_node;
	draw_softness_node_t softness_default_node;
    
	draw_font_node_t font_default_node;
	draw_font_size_node_t font_size_default_node;
    
	draw_clip_mask_node_t clip_mask_default_node;
    
	draw_texture_node_t texture_default_node;
    
};

//- globals

global draw_state_t draw_state;

//- functions

function void draw_init();
function void draw_release();
function void draw_begin(gfx_handle_t);
function void draw_end(gfx_handle_t);

function draw_instance_t* draw_get_instance();

function i32 draw_get_texture_index(gfx_handle_t texture);
function i32 draw_get_clip_mask_index(rect_t rect);

function void draw_rect(rect_t);
function void draw_image(rect_t);
function void draw_quad(vec2_t, vec2_t, vec2_t, vec2_t);
function void draw_line(vec2_t, vec2_t);
function void draw_circle(vec2_t, f32, f32, f32);
function void draw_tri(vec2_t, vec2_t, vec2_t);
function void draw_text(str_t, vec2_t);

function void draw_bezier(vec2_t p0, vec2_t p1, vec2_t c0, vec2_t c1);

// stacks
function void draw_auto_pop_stacks();

function color_t draw_top_color0();
function color_t draw_push_color0(color_t);
function color_t draw_pop_color0();
function color_t draw_set_next_color0(color_t);

function color_t draw_top_color1();
function color_t draw_push_color1(color_t); 
function color_t draw_pop_color1(); 
function color_t draw_set_next_color1(color_t);

function color_t draw_top_color2(); 
function color_t draw_push_color2(color_t); 
function color_t draw_pop_color2(); 
function color_t draw_set_next_color2(color_t);

function color_t draw_top_color3(); 
function color_t draw_push_color3(color_t); 
function color_t draw_pop_color3(); 
function color_t draw_set_next_color3(color_t);

function f32 draw_top_radius0(); 
function f32 draw_push_radius0(f32); 
function f32 draw_pop_radius0(); 
function f32 draw_set_next_radius0(f32);

function f32 draw_top_radius1();
function f32 draw_push_radius1(f32);
function f32 draw_pop_radius1(); 
function f32 draw_set_next_radius1(f32);

function f32 draw_top_radius2(); 
function f32 draw_push_radius2(f32);
function f32 draw_pop_radius2();
function f32 draw_set_next_radius2(f32);

function f32 draw_top_radius3(); 
function f32 draw_push_radius3(f32);
function f32 draw_pop_radius3(); 
function f32 draw_set_next_radius3(f32);

function f32 draw_top_thickness(); 
function f32 draw_push_thickness(f32);
function f32 draw_pop_thickness(); 
function f32 draw_set_next_thickness(f32);

function f32 draw_top_softness(); 
function f32 draw_push_softness(f32); 
function f32 draw_pop_softness();
function f32 draw_set_next_softness(f32);

function font_handle_t draw_top_font(); 
function font_handle_t draw_push_font(font_handle_t); 
function font_handle_t draw_pop_font(); 
function font_handle_t draw_set_next_font(font_handle_t);

function f32 draw_top_font_size(); 
function f32 draw_push_font_size(f32); 
function f32 draw_pop_font_size(); 
function f32 draw_set_next_font_size(f32);

function rect_t draw_top_clip_mask();
function rect_t draw_push_clip_mask(rect_t); 
function rect_t draw_pop_clip_mask();
function rect_t draw_set_next_clip_mask(rect_t);

function gfx_handle_t draw_top_texture();
function gfx_handle_t draw_push_texture(gfx_handle_t);
function gfx_handle_t draw_pop_texture();
function gfx_handle_t draw_set_next_texture(gfx_handle_t);

// group stacks
function void draw_push_color(color_t);
function void draw_set_next_color(color_t);
function void draw_pop_color();

function vec4_t draw_top_rounding();
function void draw_push_rounding(f32);
function void draw_push_rounding(vec4_t);
function void draw_set_next_rounding(f32);
function void draw_set_next_rounding(vec4_t);
function void draw_pop_rounding();

#endif // SORA_DRAW_H