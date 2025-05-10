// sora_ui_core.h

#ifndef SORA_UI_CORE_H
#define SORA_UI_CORE_H

// TODO:
//
// [x] - font alignment.
// [x] - hover cursor.
// [x] - animations.
// [x] - scrolling.
// [x] - text input.
// [x] - key bindings.
// [ ] - icons.
// [ ] - navigation.
// [ ] - focus. 
// [ ] - animation sizing.
// [ ] - tooltips/popups.
// [ ] - panels/docking.
// [ ] - mouse interaction for text edit widget.
// [ ] - tables.
// [ ] - 

//- defines 

// ui element building stack
#define ui_stack_list \
ui_stack(parent, ui_node_t*, nullptr)\
ui_stack(flags, ui_node_flags, 0)\
ui_stack(omit_flags, ui_node_flags, 0)\
ui_stack(seed_key, ui_key_t, { 0 })\
ui_stack(group_key, ui_key_t, { 0 })\
ui_stack(tag, str_t, { 0 })\
ui_stack(fixed_size_x, f32, 0.0f)\
ui_stack(fixed_size_y, f32, 0.0f)\
ui_stack(fixed_pos_x, f32, 0.0f)\
ui_stack(fixed_pos_y, f32, 0.0f)\
ui_stack(size_x, ui_size_t, { 0 })\
ui_stack(size_y, ui_size_t, { 0 })\
ui_stack(padding_x, f32, 0.0f)\
ui_stack(padding_y, f32, 0.0f)\
ui_stack(layout_dir, ui_dir, ui_dir_down)\
ui_stack(text_alignment, ui_text_alignment, ui_text_align_left)\
ui_stack(hover_cursor, os_cursor, os_cursor_null)\
ui_stack(rounding_00, f32, 4.0f)\
ui_stack(rounding_01, f32, 4.0f)\
ui_stack(rounding_10, f32, 4.0f)\
ui_stack(rounding_11, f32, 4.0f)\
ui_stack(border_size, f32, 0.0f)\
ui_stack(shadow_size, f32, 0.0f)\
ui_stack(texture, gfx_handle_t, { 0 })\
ui_stack(font, font_handle_t, ui_font_default)\
ui_stack(font_size, f32, 9.0f)\

// ui rendering stack
#define ui_r_stack_list \
ui_r_stack(color0, color_t, { 0 })\
ui_r_stack(color1, color_t, { 0 })\
ui_r_stack(color2, color_t, { 0 })\
ui_r_stack(color3, color_t, { 0 })\
ui_r_stack(radius0, f32, 0.0f)\
ui_r_stack(radius1, f32, 0.0f)\
ui_r_stack(radius2, f32, 0.0f)\
ui_r_stack(radius3, f32, 0.0f)\
ui_r_stack(thickness, f32, 0.0f)\
ui_r_stack(softness, f32, 0.0f)\
ui_r_stack(font, font_handle_t, ui_font_default)\
ui_r_stack(font_size, f32, 12.0f)\
ui_r_stack(clip_mask, rect_t, { 0 })\
ui_r_stack(texture, gfx_handle_t, { 0 })\

#define ui_r_max_clip_rects 128
#define ui_r_max_textures 16

//- enums 

enum ui_icon {
    ui_icon_user = ' ',
    ui_icon_checkmark = '!',
    ui_icon_cancel = '"',
    ui_icon_plus = '#',
    ui_icon_minus = '$',
    ui_icon_help = '%',
    ui_icon_info = '&',
    ui_icon_home = '\'',
    ui_icon_lock = '(',
    ui_icon_lock_open = ')',
    ui_icon_eye = '*',
    ui_icon_eye_off = '+',
    ui_icon_tag = ',',
    ui_icon_tags = '-',
    ui_icon_bookmark = '.',
    ui_icon_bookmark_empty = '/',
    ui_icon_flag = '0',
    ui_icon_flag_empty = '1',
    ui_icon_reply = '2',
    ui_icon_reply_all = '3',
    ui_icon_forward = '4',
    ui_icon_pencil = '5',
    ui_icon_repeat = '6',
    ui_icon_attention = '7',
    ui_icon_trash = '8',
    ui_icon_document = '9',
    ui_icon_document_text = ':',
    ui_icon_folder = ';',
    ui_icon_folder_open = '<',
    ui_icon_box = '=',
    ui_icon_menu = '>',
    ui_icon_cog = '?',
    ui_icon_cog_alt = '@',
    ui_icon_wrench = 'A',
    ui_icon_sliders = 'B',
    ui_icon_block = 'C',
    ui_icon_resize_full = 'D',
    ui_icon_resize_full_alt = 'E',
    ui_icon_resize_small = 'F',
    ui_icon_resize_vertical = 'G',
    ui_icon_resize_horizontal = 'H',
    ui_icon_move = 'I',
    ui_icon_zoom_in = 'J',
    ui_icon_zoom_out = 'K',
    ui_icon_down = 'L',
    ui_icon_up = 'M',
    ui_icon_left = 'N',
    ui_icon_right = 'O',
    ui_icon_down_open = 'P',
    ui_icon_left_open = 'Q',
    ui_icon_right_open = 'R',
    ui_icon_up_open = 'S',
    ui_icon_arrow_cw = 'T',
    ui_icon_arrow_ccw = 'U',
    ui_icon_arrows_cw = 'V',
    ui_icon_shuffle = 'W',
    ui_icon_play = 'X',
    ui_icon_stop = 'Y',
    ui_icon_pause = 'Z',
    ui_icon_to_end = '[',
    ui_icon_to_end_alt = '\\',
    ui_icon_to_start = ']',
    ui_icon_to_start_alt = '^',
    ui_icon_fast_foward = '_',
    ui_icon_fast_backward = '`',
    ui_icon_desktop = 'a',
    ui_icon_align_left = 'b',
    ui_icon_align_center = 'c',
    ui_icon_align_right = 'd',
    ui_icon_align_justify = 'e',
    ui_icon_list = 'f',
    ui_icon_indent_left = 'g',
    ui_icon_indent_right = 'h',
    ui_icon_list_bullet = 'i',
    ui_icon_ellipsis = 'j',
    ui_icon_ellipsis_vertical = 'k',
    ui_icon_off = 'l',
    ui_icon_circle_fill = 'm',
    ui_icon_circle = 'n',
    ui_icon_sort = 'o',
    ui_icon_sort_down = 'p',
    ui_icon_sort_up = 'q',
    ui_icon_sort_up_alt = 'r',
    ui_icon_sort_down_alt = 's',
    ui_icon_sort_name_up = 't',
    ui_icon_sort_name_down = 'u',
    ui_icon_sort_number_up = 'v',
    ui_icon_sort_number_down = 'w',
    ui_icon_sitemap = 'x',
    ui_icon_cube = 'y',
    ui_icon_cubes = 'z',
    ui_icon_database = '{',
    ui_icon_eyecropper = '|',
    ui_icon_brush = '}',
};

enum ui_size_type {
    ui_size_type_null,
    ui_size_type_pixel,
    ui_size_type_percent,
    ui_size_type_by_children,
    ui_size_type_by_text,
};

typedef u32 ui_axis;
enum {
    ui_axis_x = 0,
    ui_axis_y = 1,
    ui_axis_count,
};

typedef u32 ui_side;
enum {
    ui_side_min = 0,
    ui_side_max = 1,
    ui_side_count,
};

typedef u32 ui_dir;
enum {
    ui_dir_null = -1,
    ui_dir_left = 0,
    ui_dir_up = 1,
    ui_dir_right = 2,
    ui_dir_down = 3,
    ui_dir_count = 4,
};

typedef u32 ui_text_alignment;
enum {
    ui_text_align_left,
    ui_text_align_center,
    ui_text_align_right,
};

typedef u32 ui_text_op_flags;
enum {
    ui_text_op_flag_none = 0,
    ui_text_op_flag_invalid = (1 << 0),
    ui_text_op_flag_copy = (1 << 1),
};

enum ui_event_type {
    ui_event_type_null,
    ui_event_type_key_press,
    ui_event_type_key_release,
    ui_event_type_mouse_press,
    ui_event_type_mouse_release,
    ui_event_type_text,
    ui_event_type_navigate,
    ui_event_type_edit,
    ui_event_type_mouse_move,
    ui_event_type_mouse_scroll,
    ui_event_type_count,
};

typedef u32 ui_event_flags;
enum {
    ui_event_flag_null = 0,
    ui_event_flag_keep_mark = (1 << 0),
    ui_event_flag_delete = (1 << 1),
    ui_event_flag_copy = (1 << 2),
    ui_event_flag_paste = (1 << 3),
    ui_event_flag_zero_delta = (1 << 4),
    ui_event_flag_pick_side = (1 << 5),
};

enum ui_event_delta_unit {
    ui_event_delta_unit_null,
    ui_event_delta_unit_char,
    ui_event_delta_unit_word,
    ui_event_delta_unit_line,
    ui_event_delta_unit_page,
    ui_event_delta_unit_whole,
    ui_event_delta_unit_count,
};

enum ui_drag_state {
    ui_drag_state_null,
    ui_drag_state_dragging,
    ui_drag_state_dropping,
    ui_drag_state_count,
};

typedef u64 ui_node_flags;
enum {
    ui_flag_null = (0),
    ui_flag_transient = (1 << 0),
    
    // interactions
    ui_flag_mouse_interactable = (1 << 1),
    ui_flag_keyboard_interactable = (1 << 2),
    ui_flag_draggable = (1 << 3),
    ui_flag_scrollable = (1 << 4),
    ui_flag_view_scroll_x = (1 << 5),
    ui_flag_view_scroll_y = (1 << 6),
    ui_flag_view_clamp_x = (1 << 7),
    ui_flag_view_clamp_y = (1 << 8),
    
    // layout
    ui_flag_fixed_size_x = (1 << 9),
    ui_flag_fixed_size_y = (1 << 10),
    ui_flag_fixed_pos_x = (1 << 11),
    ui_flag_fixed_pos_y = (1 << 12),
    ui_flag_overflow_x = (1 << 13),
    ui_flag_overflow_y = (1 << 14),
    ui_flag_ignore_view_offset_x = (1 << 14),
    ui_flag_ignore_view_offset_y = (1 << 15),
    
    // appearance
    ui_flag_draw_background = (1 << 16),
    ui_flag_draw_text = (1 << 17),
    ui_flag_draw_border = (1 << 18),
    ui_flag_draw_shadow = (1 << 19),
    ui_flag_draw_hover_effects = (1 << 20),
    ui_flag_draw_active_effects = (1 << 21),
    ui_flag_draw_custom = (1 << 22),
    ui_flag_anim_pos_x = (1 << 23),
    ui_flag_anim_pos_y = (1 << 24),
    ui_flag_clip = (1 << 25),
    
    // groups
    ui_flag_interactable = ui_flag_mouse_interactable | ui_flag_keyboard_interactable,
    ui_flag_view_scroll = ui_flag_view_scroll_x | ui_flag_view_scroll_y,
    ui_flag_view_clamp = ui_flag_view_clamp_x | ui_flag_view_clamp_y,
    ui_flag_fixed_size = ui_flag_fixed_size_x | ui_flag_fixed_size_y,
    ui_flag_fixed_pos = ui_flag_fixed_pos_x | ui_flag_fixed_pos_y,
    ui_flag_overflow = ui_flag_overflow_x | ui_flag_overflow_y, 
    ui_flag_ignore_view_offset = ui_flag_ignore_view_offset_x | ui_flag_ignore_view_offset_y,
    ui_flag_anim_pos = ui_flag_anim_pos_x | ui_flag_anim_pos_y,
    
};

typedef u32 ui_interaction;
enum {
    ui_null = (0),
    
    ui_hovered = (1 << 1),
    ui_mouse_over = (1 << 2),
    
    ui_left_pressed = (1 << 3),
    ui_middle_pressed = (1 << 4),
    ui_right_pressed = (1 << 5),
    
    ui_left_released = (1 << 6),
    ui_middle_released = (1 << 7),
    ui_right_released = (1 << 8),
    
    ui_left_clicked = (1 << 9),
    ui_middle_clicked = (1 << 10),
    ui_right_clicked = (1 << 11),
    
    ui_left_double_clicked = (1 << 12),
    ui_middle_double_clicked = (1 << 13),
    ui_right_double_clicked = (1 << 14),
    
    ui_left_triple_clicked = (1 << 15),
    ui_middle_triple_clicked = (1 << 16),
    ui_right_triple_clicked = (1 << 17),
    
    ui_left_dragging = (1 << 18),
    ui_middle_dragging = (1 << 19),
    ui_right_dragging = (1 << 20),
    
    ui_left_double_dragging = (1 << 21),
    ui_middle_double_dragging = (1 << 22),
    ui_right_double_dragging = (1 << 23),
    
    ui_left_triple_dragging = (1 << 24),
    ui_middle_triple_dragging = (1 << 25),
    ui_right_triple_dragging = (1 << 26),
    
    // TODO: unsure about keyboard interactions.
    ui_keyboard_pressed = (1 << 27),
    ui_keyboard_released = (1 << 28),
    ui_keyboard_clicked = (1 << 29),
    
};

// rendering enums 

enum ui_r_shape{
	ui_r_shape_none,
	ui_r_shape_rect,
	ui_r_shape_quad,
	ui_r_shape_line,
	ui_r_shape_circle,
	ui_r_shape_ring,
	ui_r_shape_tri,
};

//- typedefs 

struct ui_node_t;
typedef void ui_node_custom_draw_func(ui_node_t*);

//- structs 

// key
struct ui_key_t {
    u64 data[1];
};

// size
struct ui_size_t {
    ui_size_type type;
    f32 value;
    f32 strictness;
};

// theme
struct ui_theme_pattern_t {
    ui_theme_pattern_t* next;
    ui_theme_pattern_t* prev;
    ui_key_t key;
    color_t color;
};

struct ui_theme_pattern_hash_list_t {
    ui_theme_pattern_t* first;
    ui_theme_pattern_t* last;
};

struct ui_theme_t {
    ui_theme_pattern_hash_list_t* theme_pattern_hash_list;
    u32 theme_pattern_hash_list_count;
};


struct ui_text_point_t {
    i32 line;
    i32 column;
};

struct ui_text_range_t {
    ui_text_point_t min;
    ui_text_point_t max;
};

// text op
struct ui_text_op_t {
    ui_text_op_flags flags;
    str_t replace;
    str_t copy;
    ui_text_range_t range;
    ui_text_point_t cursor;
    ui_text_point_t mark;
};

// key binding
struct ui_key_binding_t {
    ui_key_binding_t* next;
    ui_key_binding_t* prev;
    
    os_key key;
    os_modifiers modifiers;
    
    ui_event_type result_type;
    ui_event_flags result_flags;
    ui_event_delta_unit result_delta_unit;
    ivec2_t result_delta;
};

struct ui_key_binding_list_t {
    ui_key_binding_t* first;
    ui_key_binding_t* last;
};

// events
struct ui_event_t {
    ui_event_t* next;
    ui_event_t* prev;
    
    ui_event_type type;
    ui_event_flags flags;
    ui_event_delta_unit delta_unit;
    os_event_t* os_event;
    ivec2_t delta;
};

struct ui_event_list_t {
    ui_event_t* first;
    ui_event_t* last;
};

// node 
struct ui_node_t {
    
    // list
    ui_node_t* list_next;
    ui_node_t* list_prev;
    
    // tree
    ui_node_t* tree_next;
    ui_node_t* tree_prev;
    ui_node_t* tree_parent;
    ui_node_t* tree_first;
    ui_node_t* tree_last;
    
    // info
    ui_key_t key;
    ui_key_t group_key;
    ui_key_t tags_key;
    str_t label;
    ui_node_flags flags;
    
    // layout
    vec2_t pos_fixed;
    vec2_t pos_target;
    vec2_t pos;
    ui_size_t size_wanted[2];
    vec2_t size_fixed;
    vec2_t size;
    vec2_t padding;
    ui_dir layout_dir;
    ui_text_alignment text_alignment;
    vec2_t view_bounds;
    vec2_t view_offset_target;
    vec2_t view_offset;
    vec2_t view_offset_prev;
    rect_t rect;
    
    // appearance
    f32 hover_t;
    f32 active_t;
    vec4_t rounding;
    f32 border_size;
    f32 shadow_size;
    gfx_handle_t texture;
    font_handle_t font;
    f32 font_size;
    os_cursor hover_cursor;
    ui_node_custom_draw_func* custom_draw_func;
    void* custom_draw_data;
    
    u64 first_build_index;
    u64 last_build_index;
};

struct ui_node_rec_t {
    ui_node_t* next;
    i32 push_count;
    i32 pop_count;
};

struct ui_node_hash_list_t {
    ui_node_t* first;
    ui_node_t* last;
    u32 count;
};

struct ui_node_list_item_t {
    ui_node_list_item_t* next;
    ui_node_t* node;
};

struct ui_node_list_t {
    ui_node_list_item_t* first;
    ui_node_list_item_t* last;
    u32 count;
};

// animation 

struct ui_anim_params_t {
    f32 initial;
    f32 target;
    f32 rate;
};

struct ui_anim_node_t {
    ui_anim_node_t* list_next;
    ui_anim_node_t* list_prev;
    
    ui_anim_node_t* lru_next;
    ui_anim_node_t* lru_prev;
    
    u64 first_build_index;
    u64 last_build_index;
    
    ui_key_t key;
    ui_anim_params_t params;
    
    f32 current;
};

struct ui_anim_hash_list_t {
    ui_anim_node_t* first;
    ui_anim_node_t* last;
    u32 count;
};

// tags

struct ui_tags_node_t {
    ui_tags_node_t* next;
    ui_key_t key;
};

struct ui_tags_hash_list_t {
    ui_tags_node_t* first;
    ui_tags_node_t* last;
};

struct ui_tags_stack_node_t {
    ui_tags_stack_node_t* next;
    ui_key_t key;
};

// rendering

struct ui_r_constants_t {
    vec2_t window_size;
    vec2_t padding;
    rect_t clip_masks[ui_r_max_clip_rects];
};

struct ui_r_instance_t {
    rect_t bbox;
	rect_t tex;
	vec2_t point0;
	vec2_t point1;
	vec2_t point2;
	vec2_t point3;
	color_t color0;
	color_t color1;
	color_t color2;
	color_t color3;
	vec4_t radii;
	f32 thickness;
	f32 softness;
	u32 indices;
};

struct ui_r_batch_t {
	ui_r_batch_t* next;
	ui_r_batch_t* prev;
	
	ui_r_instance_t* instances;
	u32 instance_count;
};

// stacks

// element building stacks
#define ui_stack(name, type) \
struct ui_##name##_node_t { ui_##name##_node_t* next; type v; };\
struct ui_##name##_stack_t { ui_##name##_node_t* top; ui_##name##_node_t* free; b8 auto_pop; };
ui_stack_list
#undef ui_stack

// rendering stacks
#define ui_r_stack(name, type) \
struct ui_r_##name##_node_t { ui_r_##name##_node_t* next; type v; };\
struct ui_r_##name##_stack_t { ui_r_##name##_node_t* top; ui_r_##name##_node_t* free; b8 auto_pop; };
ui_r_stack_list
#undef ui_r_stack

// context

struct ui_context_t {
    
    // arenas
    arena_t* arena;
    arena_t* build_arenas[2];
    arena_t* drag_state_arena;
    arena_t* event_arena;
    
    // context
    os_handle_t window;
    gfx_handle_t renderer;
    
    // build state
    u64 build_index;
    
    // keys
    ui_key_t key_hovered;
    ui_key_t key_active[os_mouse_button_count];
    ui_key_t key_focused;
    ui_key_t key_popup;
    ui_key_t key_drag;
    
    ui_key_t key_hovered_prev;
    ui_key_t key_active_prev[os_mouse_button_count];
    ui_key_t key_focused_prev;
    ui_key_t key_popup_prev;
    ui_key_t key_drag_prev;
    
    // nodes
    ui_node_hash_list_t* node_hash_list;
    u32 node_hash_list_count;
    ui_node_t* node_free;
    ui_node_t* node_root;
    ui_node_t* node_tooltip_root;
    ui_node_t* node_popup_root;
    
    // animation cache
    ui_anim_hash_list_t* anim_hash_list;
    u32 anim_hash_list_count;
    ui_anim_node_t* anim_node_free;
    ui_anim_node_t* anim_node_lru;
    ui_anim_node_t* anim_node_mru;
    
    f32 anim_rapid_rate;
    f32 anim_fast_rate;
    f32 anim_slow_rate;
    
    // theme pattern hash list
    ui_theme_pattern_hash_list_t* theme_pattern_hash_list;
    u32 theme_pattern_hash_list_count;
    
    // tags
    ui_tags_hash_list_t* tags_hash_list;
    u32 tags_hash_list_count;
    ui_tags_stack_node_t* tags_stack_top;
    ui_tags_stack_node_t* tags_stack_free;
    
    // events
    ui_event_list_t event_list;
    u32 click_counter[os_mouse_button_count];
    u64 last_click_time[os_mouse_button_count];
    vec2_t mouse_pos;
    vec2_t mouse_delta;
    ui_key_binding_list_t key_binding_list;
    
    // drag state
    ui_drag_state drag_state;
    void* drag_state_data;
    u32 drag_state_size;
    vec2_t drag_start_pos;
    
    // renderer
    gfx_handle_t vertex_shader;
    gfx_handle_t pixel_shader;
    gfx_handle_t texture;
    
    gfx_handle_t instance_buffer;
    gfx_handle_t constant_buffer;
    ui_r_constants_t constants;
    
    i32 clip_mask_count;
    gfx_handle_t texture_list[ui_r_max_textures];
	u32 texture_count;
    
    // renderer batches
	arena_t* batch_arena;
	ui_r_batch_t* batch_first;
	ui_r_batch_t* batch_last;
    
    // stacks
#define ui_stack(name, type)\
ui_##name##_node_t name##_default_node;\
ui_##name##_stack_t name##_stack;
    ui_stack_list
#undef ui_stack
    
#define ui_r_stack(name, type)\
ui_r_##name##_node_t name##_r_default_node;\
ui_r_##name##_stack_t name##_r_stack;
    ui_r_stack_list
#undef ui_r_stack
    
};

//- globals 

global font_handle_t ui_font_icon;
global font_handle_t ui_font_default;
thread_global ui_context_t* ui_active_context = nullptr; 

//- functions 

// state
function void ui_init();
function void ui_release();
function void ui_begin(ui_context_t* context);
function void ui_end(ui_context_t* context);
function arena_t* ui_build_arena();

// context
function ui_context_t* ui_context_create(os_handle_t window, gfx_handle_t renderer);
function void ui_context_release(ui_context_t* context);
function void ui_context_set_active(ui_context_t* context);
function void ui_context_add_color(ui_context_t* context, str_t tags, color_t color);

// keys
function ui_key_t ui_key_from_string(ui_key_t seed, str_t string);
function ui_key_t ui_key_from_stringf(ui_key_t seed, char* fmt, ...);
function b8 ui_key_equals(ui_key_t a, ui_key_t b);

// size
inlnfunc ui_size_t ui_size(ui_size_type type, f32 value, f32 strictness);
inlnfunc ui_size_t ui_size_pixels(f32 pixels, f32 strictness = 1.0f);
inlnfunc ui_size_t ui_size_percent(f32 percent);
inlnfunc ui_size_t ui_size_by_children(f32 strictness);
inlnfunc ui_size_t ui_size_by_text(f32 padding);

// axis/side/dir
function ui_axis ui_axis_from_dir(ui_dir dir);
function ui_side ui_side_from_dir(ui_dir dir);
function ui_dir ui_dir_from_axis_side(ui_axis axis, ui_side side);

// text alignment
function vec2_t  ui_text_size(font_handle_t font, f32 font_size, str_t text);
function vec2_t ui_text_align(font_handle_t font, f32 font_size, str_t text, rect_t rect, ui_text_alignment alignment);

// text point
function ui_text_point_t ui_text_point(i32 line, i32 column);
function b8 ui_text_point_equals(ui_text_point_t a, ui_text_point_t b);
function b8 ui_text_point_less_than(ui_text_point_t a, ui_text_point_t b);
function ui_text_point_t ui_text_point_less_min(ui_text_point_t a, ui_text_point_t b);
function ui_text_point_t ui_text_point_less_max(ui_text_point_t a, ui_text_point_t b);

// text range
function ui_text_range_t ui_text_range(ui_text_point_t min, ui_text_point_t max);
function ui_text_range_t ui_text_range_intersects(ui_text_range_t a, ui_text_range_t b);
function ui_text_range_t ui_text_range_union(ui_text_range_t a, ui_text_range_t b);
function b8 ui_text_range_contains(ui_text_range_t r, ui_text_point_t p);

// text op
function ui_text_op_t ui_single_line_text_op_from_event(arena_t *arena, ui_event_t* event, str_t string, ui_text_point_t cursor, ui_text_point_t mark);

// events
function void ui_event_push(ui_event_t* event);
function void ui_event_pop(ui_event_t* event);

// keybinding
function void ui_key_binding_add(os_key key, os_modifiers modifiers, ui_event_type result_type, ui_event_flags result_flags, ui_event_delta_unit result_delta_unit, ivec2_t result_delta);
function ui_key_binding_t* ui_key_binding_find(os_key key, os_modifiers modifiers);

// theme
function color_t ui_color_from_key(ui_key_t key);
function color_t ui_color_from_string(str_t string);

// animation
function ui_anim_params_t ui_anim_params_create(f32 initial, f32 target, f32 rate = ui_active_context->anim_fast_rate);
function f32 ui_anim_ex(ui_key_t key, ui_anim_params_t params);
function f32 ui_anim(ui_key_t key, f32 initial, f32 target, f32 rate = ui_active_context->anim_fast_rate);

// drag state
function void ui_drag_store_data(void* data, u32 size);
function void* ui_drag_get_data();
function void ui_drag_clear_data();
function b8 ui_drag_is_active();
function void ui_drag_begin(ui_key_t key = { 0 });
function b8  ui_drag_drop();
function void ui_drag_kill();
function vec2_t ui_drag_delta();

// nodes
function ui_node_t* ui_node_find(ui_key_t key); 
function ui_node_t* ui_node_from_key(ui_node_flags flags, ui_key_t key);
function ui_node_t* ui_node_from_string(ui_node_flags flags, str_t string);
function ui_node_t* ui_node_from_stringf(ui_node_flags flags, char* fmt, ...);
function ui_node_rec_t ui_node_rec_depth_first(ui_node_t* node);

function void ui_node_set_custom_draw(ui_node_t* node, ui_node_custom_draw_func* func, void* data);

// interaction
function ui_interaction ui_interaction_from_node(ui_node_t* node);

// layout
function void ui_layout_solve_independent(ui_node_t* node, ui_axis axis);
function void ui_layout_solve_upward_dependent(ui_node_t* node, ui_axis axis);
function void ui_layout_solve_downward_dependent(ui_node_t* node, ui_axis axis);
function void ui_layout_solve_violations(ui_node_t* node, ui_axis axis);
function void ui_layout_set_positions(ui_node_t* node, ui_axis axis);

// renderer
function ui_r_instance_t* ui_r_get_instance();
function i32 ui_r_get_texture_index(gfx_handle_t texture);
function i32 ui_r_get_clip_mask_index(rect_t rect);

function void ui_r_draw_rect(rect_t);
function void ui_r_draw_image(rect_t);
function void ui_r_draw_quad(vec2_t, vec2_t, vec2_t, vec2_t);
function void ui_r_draw_line(vec2_t, vec2_t);
function void ui_r_draw_circle(vec2_t, f32, f32, f32);
function void ui_r_draw_tri(vec2_t, vec2_t, vec2_t);
function void ui_r_draw_text(str_t, vec2_t);


// stacks
function void ui_auto_pop_stacks();

function ui_node_t* ui_top_parent();
function ui_node_t* ui_push_parent(ui_node_t* v);
function ui_node_t* ui_pop_parent();
function ui_node_t* ui_set_next_parent(ui_node_t* v);

function ui_node_flags ui_top_flags();
function ui_node_flags ui_push_flags(ui_node_flags v);
function ui_node_flags ui_pop_flags();
function ui_node_flags ui_set_next_flags(ui_node_flags v);

function ui_node_flags ui_top_omit_flags();
function ui_node_flags ui_push_omit_flags(ui_node_flags v);
function ui_node_flags ui_pop_omit_flags();
function ui_node_flags ui_set_next_omit_flags(ui_node_flags v);

function ui_key_t ui_top_seed_key();
function ui_key_t ui_push_seed_key(ui_key_t v);
function ui_key_t ui_pop_seed_key();
function ui_key_t ui_set_next_seed_key(ui_key_t v);

function ui_key_t ui_top_group_key();
function ui_key_t ui_push_group_key(ui_key_t v);
function ui_key_t ui_pop_group_key();
function ui_key_t ui_set_next_group_key(ui_key_t v);

function str_t ui_top_tag();
function str_t ui_push_tag(str_t v);
function str_t ui_push_tagf(char* fmt, ...);
function str_t ui_pop_tag();
function str_t ui_set_next_tag(str_t v);
function str_t ui_set_next_tagf(char* fmt, ...);

function f32 ui_top_fixed_size_x();
function f32 ui_push_fixed_size_x(f32 v);
function f32 ui_pop_fixed_size_x();
function f32 ui_set_next_fixed_size_x(f32 v);

function f32 ui_top_fixed_size_y();
function f32 ui_push_fixed_size_y(f32 v);
function f32 ui_pop_fixed_size_y();
function f32 ui_set_next_fixed_size_y(f32 v);

function f32 ui_top_fixed_pos_x();
function f32 ui_push_fixed_pos_x(f32 v);
function f32 ui_pop_fixed_pos_x();
function f32 ui_set_next_fixed_pos_x(f32 v);

function f32 ui_top_fixed_pos_y();
function f32 ui_push_fixed_pos_y(f32 v);
function f32 ui_pop_fixed_pos_y();
function f32 ui_set_next_fixed_pos_y(f32 v);

function ui_size_t ui_top_size_x();
function ui_size_t ui_push_size_x(ui_size_t v);
function ui_size_t ui_pop_size_x();
function ui_size_t ui_set_next_size_x(ui_size_t v);

function ui_size_t ui_top_size_y();
function ui_size_t ui_push_size_y(ui_size_t v);
function ui_size_t ui_pop_size_y();
function ui_size_t ui_set_next_size_y(ui_size_t v);

function f32 ui_top_padding_x();
function f32 ui_push_padding_x(f32 v);
function f32 ui_pop_padding_x();
function f32 ui_set_next_padding_x(f32 v);

function f32 ui_top_padding_y();
function f32 ui_push_padding_y(f32 v);
function f32 ui_pop_padding_y();
function f32 ui_set_next_padding_y(f32 v);

function ui_dir ui_top_layout_dir();
function ui_dir ui_push_layout_dir(ui_dir v);
function ui_dir ui_pop_layout_dir();
function ui_dir ui_set_next_layout_dir(ui_dir v);

function ui_text_alignment ui_top_text_alignment();
function ui_text_alignment ui_push_text_alignment(ui_text_alignment v);
function ui_text_alignment ui_pop_text_alignment();
function ui_text_alignment ui_set_next_text_alignment(ui_text_alignment v);

function os_cursor ui_top_hover_cursor();
function os_cursor ui_push_hover_cursor(os_cursor v);
function os_cursor ui_pop_hover_cursor();
function os_cursor ui_set_next_hover_cursor(os_cursor v);

function f32 ui_top_rounding_00();
function f32 ui_push_rounding_00(f32 v);
function f32 ui_pop_rounding_00();
function f32 ui_set_next_rounding_00(f32 v);

function f32 ui_top_rounding_01();
function f32 ui_push_rounding_01(f32 v);
function f32 ui_pop_rounding_01();
function f32 ui_set_next_rounding_01(f32 v);

function f32 ui_top_rounding_10();
function f32 ui_push_rounding_10(f32 v);
function f32 ui_pop_rounding_10();
function f32 ui_set_next_rounding_10(f32 v);

function f32 ui_top_rounding_11();
function f32 ui_push_rounding_11(f32 v);
function f32 ui_pop_rounding_11();
function f32 ui_set_next_rounding_11(f32 v);

function f32 ui_top_border_size();
function f32 ui_push_border_size(f32 v);
function f32 ui_pop_border_size();
function f32 ui_set_next_border_size(f32 v);

function f32 ui_top_shadow_size();
function f32 ui_push_shadow_size(f32 v);
function f32 ui_pop_shadow_size();
function f32 ui_set_next_shadow_size(f32 v);

function gfx_handle_t ui_top_texture();
function gfx_handle_t ui_push_texture(gfx_handle_t v);
function gfx_handle_t ui_pop_texture();
function gfx_handle_t ui_set_next_texture(gfx_handle_t v);

function font_handle_t ui_top_font();
function font_handle_t ui_push_font(font_handle_t v);
function font_handle_t ui_pop_font();
function font_handle_t ui_set_next_font(font_handle_t v);

function f32 ui_top_font_size();
function f32 ui_push_font_size(f32 v);
function f32 ui_pop_font_size();
function f32 ui_set_next_font_size(f32 v);

// tags
function ui_key_t ui_top_tags_key();
function str_t ui_top_tag();
function str_t ui_push_tag(str_t v);
function str_t ui_pop_tag();
function str_t ui_set_next_tag(str_t v);

// group stacks

function void ui_push_size(ui_size_t size_x, ui_size_t size_y);
function void ui_pop_size();
function void ui_set_next_size(ui_size_t size_x, ui_size_t size_y);

function void ui_push_fixed_size(f32 x, f32 y);
function void ui_pop_fixed_size();
function void ui_set_next_fixed_size(f32 x, f32 y);

function void ui_push_fixed_pos(f32 x, f32 y);
function void ui_pop_fixed_pos();
function void ui_set_next_fixed_pos(f32 x, f32 y);

function void ui_push_rect(rect_t rect);
function void ui_pop_rect();
function void ui_set_next_rect(rect_t rect);

function void ui_push_rounding(vec4_t rounding);
function void ui_pop_rounding();
function void ui_set_next_rounding(vec4_t rounding);

function void ui_push_padding(f32 value);
function void ui_pop_padding();
function void ui_set_next_padding(f32 value);

// rendering stacks

function void ui_r_auto_pop_stacks();

function color_t ui_r_top_color0();
function color_t ui_r_push_color0(color_t);
function color_t ui_r_pop_color0();
function color_t ui_r_set_next_color0(color_t);

function color_t ui_r_top_color1();
function color_t ui_r_push_color1(color_t); 
function color_t ui_r_pop_color1(); 
function color_t ui_r_set_next_color1(color_t);

function color_t ui_r_top_color2(); 
function color_t ui_r_push_color2(color_t); 
function color_t ui_r_pop_color2(); 
function color_t ui_r_set_next_color2(color_t);

function color_t ui_r_top_color3(); 
function color_t ui_r_push_color3(color_t); 
function color_t ui_r_pop_color3(); 
function color_t ui_r_set_next_color3(color_t);

function f32 ui_r_top_radius0(); 
function f32 ui_r_push_radius0(f32); 
function f32 ui_r_pop_radius0(); 
function f32 ui_r_set_next_radius0(f32);

function f32 ui_r_top_radius1();
function f32 ui_r_push_radius1(f32);
function f32 ui_r_pop_radius1(); 
function f32 ui_r_set_next_radius1(f32);

function f32 ui_r_top_radius2(); 
function f32 ui_r_push_radius2(f32);
function f32 ui_r_pop_radius2();
function f32 ui_r_set_next_radius2(f32);

function f32 ui_r_top_radius3(); 
function f32 ui_r_push_radius3(f32);
function f32 ui_r_pop_radius3(); 
function f32 ui_r_set_next_radius3(f32);

function f32 ui_r_top_thickness(); 
function f32 ui_r_push_thickness(f32);
function f32 ui_r_pop_thickness(); 
function f32 ui_r_set_next_thickness(f32);

function f32 ui_r_top_softness(); 
function f32 ui_r_push_softness(f32); 
function f32 ui_r_pop_softness();
function f32 ui_r_set_next_softness(f32);

function font_handle_t ui_r_top_font(); 
function font_handle_t ui_r_push_font(font_handle_t); 
function font_handle_t ui_r_pop_font(); 
function font_handle_t ui_r_set_next_font(font_handle_t);

function f32 ui_r_top_font_size(); 
function f32 ui_r_push_font_size(f32); 
function f32 ui_r_pop_font_size(); 
function f32 ui_r_set_next_font_size(f32);

function rect_t ui_r_top_clip_mask();
function rect_t ui_r_push_clip_mask(rect_t); 
function rect_t ui_r_pop_clip_mask();
function rect_t ui_r_set_next_clip_mask(rect_t);

function gfx_handle_t ui_r_top_texture();
function gfx_handle_t ui_r_push_texture(gfx_handle_t);
function gfx_handle_t ui_r_pop_texture();
function gfx_handle_t ui_r_set_next_texture(gfx_handle_t);

// group stacks
function void ui_r_push_color(color_t);
function void ui_r_set_next_color(color_t);
function void ui_r_pop_color();

function vec4_t ui_r_top_rounding();
function void ui_r_push_rounding(f32);
function void ui_r_push_rounding(vec4_t);
function void ui_r_set_next_rounding(f32);
function void ui_r_set_next_rounding(vec4_t);
function void ui_r_pop_rounding();

#endif // SORA_UI_CORE_H