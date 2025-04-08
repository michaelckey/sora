// sora_ui_core.h

#ifndef SORA_UI_CORE_H
#define SORA_UI_CORE_H

// TODO:
//
// [x] - font alignment.
// [ ] - hover cursor.
// [ ] - more events
//    [ ] - typing.
//    [ ] - scrolling.
//    [ ] - key bindings.
// [ ] - navigation.
// [ ] - animation sizing.
// 

//- defines 

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
ui_stack(text_alignment, ui_text_alignment, ui_text_alignment_left)\
ui_stack(hover_cursor, os_cursor, os_cursor_null)\
ui_stack(rounding_00, f32, 4.0f)\
ui_stack(rounding_01, f32, 4.0f)\
ui_stack(rounding_10, f32, 4.0f)\
ui_stack(rounding_11, f32, 4.0f)\
ui_stack(border_size, f32, 0.0f)\
ui_stack(shadow_size, f32, 0.0f)\
ui_stack(texture, gfx_handle_t, { 0 })\
ui_stack(font, font_handle_t, draw_state.font)\
ui_stack(font_size, f32, 9.0f)\

//- enums 

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
    ui_text_alignment_left,
    ui_text_alignment_center,
    ui_text_alignment_right,
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

// events
struct ui_event_t {
    ui_event_t* next;
    ui_event_t* prev;
    
    ui_event_type type;
    ui_event_flags flags;
    ui_event_delta_unit delta_unit;
    os_event_t* os_event;
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

// stacks

#define ui_stack(name, type) \
struct ui_##name##_node_t { ui_##name##_node_t* next; type v; };\
struct ui_##name##_stack_t { ui_##name##_node_t* top; ui_##name##_node_t* free; b8 auto_pop; };
ui_stack_list
#undef ui_stack

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
    
    // drag state
    ui_drag_state drag_state;
    void* drag_state_data;
    u32 drag_state_size;
    vec2_t drag_start_pos;
    
    // stacks
#define ui_stack(name, type)\
ui_##name##_node_t name##_default_node;\
ui_##name##_stack_t name##_stack;
    ui_stack_list
#undef ui_stack
    
};

//- globals 

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

// theme
function color_t ui_color_from_key(ui_key_t key);

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

// interaction
function ui_interaction ui_interaction_from_node(ui_node_t* node);

// layout
function void ui_layout_solve_independent(ui_node_t* node, ui_axis axis);
function void ui_layout_solve_upward_dependent(ui_node_t* node, ui_axis axis);
function void ui_layout_solve_downward_dependent(ui_node_t* node, ui_axis axis);
function void ui_layout_solve_violations(ui_node_t* node, ui_axis axis);
function void ui_layout_set_positions(ui_node_t* node, ui_axis axis);

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

#endif // SORA_UI_CORE_H