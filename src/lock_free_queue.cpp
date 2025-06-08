// lock_free_queue.cpp

//~ includes

#include "core/sora_inc.h"
#include "core/sora_inc.cpp"

#include "ui/sora_ui_inc.h"
#include "ui/sora_ui_inc.cpp"

//~ defines 

#define num_threads 10
#define items_per_producer 1000
#define items_total (num_threads * items_per_producer)

//~ structs 

struct slot_t {
    void* data;
    atomic_i64 sequence;
};

struct queue_t {
    
    arena_t* arena;
    
    __declspec(align(64)) atomic_i64 head;
    __declspec(align(64)) atomic_i64 tail;
    
    u32 capacity;
    u32 mask;
    
    __declspec(align(64)) slot_t* slots;
};


struct profile_slot_t {
    cstr name;
    i32 depth;
    u64 start;
    u64 end;
};

struct profile_graph_t {
    
    profile_slot_t slots[1024];
    u32 slot_count;
    
    u32 slot_indices[512];
    u32 top_slot_index;
    
    u32 current_depth;
    
    u64 cpu_freq;
};

//~ globals

global arena_t* arena;
global os_handle_t window;
global gfx_handle_t context;
global ui_context_t* ui;
global b8 quit = false;

global queue_t* queue;
global atomic_i32 produced; 
global atomic_i32 consumed;

global profile_graph_t profile_graph;

//~ functions

// queue

function void queue_init(queue_t* queue, u32 capacity);
function b8 queue_enqueue(queue_t* queue, void* data);
function b8 queue_dequeue(queue_t* queue, void** data);

function b8 queue_empty(queue_t* queue);
function b8 queue_full(queue_t* queue);

// thread
function void thread_producer(void* params);
function void thread_consumer(void* params);


// profiler

function void profile_init();
function void profile_release();
function void profile_begin();
function void profile_end();

function void ui_profile_graph(str_t label);
function void ui_profile_slot_draw(ui_node_t* node);

// app
function void app_init();
function void app_release();
function void app_frame();

//~ implementation 

//- queue functions 

function queue_t* 
queue_create(u32 capacity) {
    
    arena_t* arena = arena_create_aligned(megabytes(64), 64);
    
    queue_t* queue = (queue_t*)arena_calloc(arena, sizeof(queue_t));
    
    queue->arena = arena;
    
    queue->capacity = capacity;
    queue->mask = capacity - 1;
    atomic_i64_assign(&queue->head, 0);
    atomic_i64_assign(&queue->tail, 0);
    
    queue->slots = (slot_t*)arena_calloc(arena, sizeof(slot_t) * capacity);
    
    for (i32 i = 0; i < capacity; i++) {
        atomic_ptr_assign(&queue->slots[i].data, nullptr);
        atomic_i64_assign(&queue->slots[i].sequence, (i64)i);
    }
    
    return queue;
}


function b8
queue_enqueue(queue_t* queue, void* data) {
    
    i64 tail;
    slot_t* slot;
    i64 sequence;
    i64 diff;
    b8 result = false;
    u32 backoff = 1;
    
    for (;;) {
        
        tail = atomic_i64_load(&queue->tail);
        slot = &queue->slots[tail & queue->mask];
        sequence = atomic_i64_load(&slot->sequence);
        
        diff = sequence - tail;
        if (diff == 0) {
            if (atomic_i64_cond_assign(&queue->tail, tail + 1, tail)) {
                atomic_ptr_assign(&slot->data, data);
                atomic_memory_barrier();
                atomic_i64_assign(&slot->sequence, tail + 1);
                result = true;
                break;
            } else {
                for (u32 i = 0; i < backoff; i++) {
                    _mm_pause();
                }
                backoff = min(backoff * 2, 64);
            }
        } else if (diff < 0) {
            // queue is full
            break;
        }
    }
    
    return result;
}

function b8
queue_dequeue(queue_t* queue, void** data) {
    
    i64 head;
    slot_t* slot;
    i64 sequence;
    i64 diff;
    b8 result = false;
    u32 backoff = 1;
    
    for (;;) {
        
        head = atomic_i64_load(&queue->head);
        slot = &queue->slots[head & queue->mask];
        sequence = atomic_i64_load(&slot->sequence);
        
        diff = sequence - (head + 1);
        if (diff == 0) {
            if (atomic_i64_cond_assign(&queue->head, head + 1, head)) {
                *data = atomic_ptr_load(&slot->data);
                atomic_memory_barrier();
                atomic_i64_assign(&slot->sequence, head + queue->capacity);
                result = true;
                break;
            } else {
                for (u32 i = 0; i < backoff; i++) {
                    _mm_pause();
                }
                backoff = min(backoff * 2, 64);
            }
        } else if (diff < 0) {
            break;
        }
    }
    
    return result;
}

function b8 
queue_empty(queue_t* queue) {
    i64 head = queue->head;
    i64 tail = queue->tail;
    return (head >= tail);
}

function b8 
queue_full(queue_t* queue) {
    i64 head = queue->head;
    i64 tail = queue->tail;
    return (tail - head) >= (i64)queue->capacity;
}

//- thread functions 

function void 
thread_producer(void* params) {
    
    for (i32 i = 0; i < items_per_producer; i++) {
        i32 value = i;
        while (!queue_enqueue(queue, &value)) {
            os_sleep(0);
        }
        atomic_i32_inc(&produced);
    }
    
}

function void
thread_consumer(void* params) {
    
    while (true) {
        if (consumed >= items_total) {
            break;
        }
        
        u32* value = nullptr;
        if (queue_dequeue(queue, &(void*)value)) {
            atomic_i32_inc(&consumed);
        } else {
            os_sleep(0);
        }
    }
    
}

//- profile functions 


function void 
profile_init() {
    profile_graph.cpu_freq = os_get_cpu_freq();
}

function void 
profile_release() {
    
}

function void 
profile_begin(cstr name) {
    
    // get index
    u32 index = profile_graph.slot_count;
    
    profile_slot_t* slot = &profile_graph.slots[index];
    profile_graph.slot_count++;
    
    // push this index to the stack
    profile_graph.slot_indices[++profile_graph.top_slot_index] = index;
    
    slot->name = name;
    slot->depth = profile_graph.current_depth;
    slot->start =  os_get_cpu_time();
    
    profile_graph.current_depth++;
}

function void 
profile_end() {
    
    u64 end_time = os_get_cpu_time();
    
    // pop index from stack
    u32 index = profile_graph.slot_indices[profile_graph.top_slot_index--];
    
    // get slot
    profile_slot_t* slot = &profile_graph.slots[index];
    profile_graph.current_depth--;
    slot->end = end_time;
}

function void 
ui_profile_graph(str_t label) {
    
    ui_node_flags graph_flags = 
        ui_flag_mouse_interactable |
        ui_flag_draw_background |
        ui_flag_draw_border |
        ui_flag_clip;
    
    ui_node_t* profile_graph_node = ui_node_from_string(graph_flags, label);
    
    u64 start_ms = profile_graph.slots[0].start;
    f64 total_ms = 0.0f;
    for (i32 i = 0; i < profile_graph.slot_count; i++) {
        profile_slot_t* slot = &profile_graph.slots[i];
        if (slot->depth == 0) {
            total_ms += (f64)(slot->end - slot->start) * 1000.0f / (f64)(profile_graph.cpu_freq);
        }
    }
    
    ui_push_parent(profile_graph_node);
    
    f32 graph_width = rect_width(profile_graph_node->rect);
    f32 graph_height = rect_height(profile_graph_node->rect);
    
    for (i32 i = 0; i < profile_graph.slot_count; i++) {
        profile_slot_t* slot = &profile_graph.slots[i];
        
        // calculate rect
        f64 min_ms =  (f64)(slot->start - start_ms) * 1000.0f / (f64)(profile_graph.cpu_freq);
        f64 max_ms =  (f64)(slot->end - start_ms) * 1000.0f / (f64)(profile_graph.cpu_freq);
        vec2_t min_pos = vec2((min_ms / total_ms) * graph_width, 25.0f * slot->depth); 
        vec2_t max_pos = vec2((max_ms / total_ms) * graph_width, 25.0f * (slot->depth + 1)); 
        
        f32 slot_ms = max_ms - min_ms;
        
        // build ui node
        ui_set_next_rect(rect(min_pos, max_pos));
        ui_node_t* slot_node = ui_node_from_stringf(ui_flag_mouse_interactable | ui_flag_draw_border | ui_flag_draw_custom, "%s_%p", label.data, slot);
        ui_node_set_display_string(slot_node, str_format(ui_build_arena(), "%s : %.2f ms", slot->name, slot_ms));
        ui_node_set_custom_draw(slot_node, ui_profile_slot_draw, nullptr);
        
        ui_interaction interaction = ui_interaction_from_node(slot_node);
        
        if (interaction & ui_hovered) {
            ui_tooltip_begin();
            
            ui_set_next_size(ui_size_by_text(2.0f), ui_size_by_text(2.0f));
            ui_labelf("%s : %.2f ms", slot->name, slot_ms);
            
            ui_tooltip_end();
        }
        
        
    }
    
    ui_pop_parent();
    
    // interaction
    ui_interaction interaction = ui_interaction_from_node(profile_graph_node);
    
}

function void
ui_profile_slot_draw(ui_node_t* node) {
    
    f32 hue = (f32)((u32)node % 360) / 360.0f;
    
    color_t col = color_rgb_from_hsv(color(hue, 0.4f, 0.5f, 1.0f));
    
    // draw background
    ui_r_set_next_color(col );
    ui_r_draw_rect(node->rect, 0.0f, 0.33f, node->rounding);
    
    // draw text
    ui_r_push_clip_mask(node->rect);
    
    color_t text_color = ui_color_from_key_name(node->tags_key, str("text"));
    color_t shadow_color = ui_color_from_key_name(node->tags_key, str("text shadow"));
    vec2_t text_pos = ui_text_align(node->font, node->font_size, node->label, node->rect, node->text_alignment);
    
    ui_r_set_next_color(shadow_color);
    ui_r_draw_text(node->label, vec2_add(text_pos, 1.0f), node->font, node->font_size);
    
    ui_r_set_next_color(text_color);
    ui_r_draw_text(node->label, text_pos, node->font, node->font_size);
    
    ui_r_pop_clip_mask();
}

//- app functions 

function void 
app_init() {
    
    profile_begin("app_init");
    
    os_sleep(5);
    
    // create arena
    arena = arena_create(gigabytes(2));
    
    // open window and create renderer
    profile_begin("window_create");
    window = os_window_open(str("lock free queue"), 1280, 720);
    os_window_set_frame_function(window, app_frame);
    profile_end();
    
    os_sleep(1);
    
    // create graphics context
    profile_begin("graphics_create");
    context = gfx_context_create(window);
    profile_end();
    
    // create ui context
    ui = ui_context_create(window, context);
    ui_context_default_theme(ui);
    
    // queue testing
    profile_begin("queue_create");
    queue = queue_create(1024);
    profile_end();
    
    profile_end();
    
    atomic_i32_assign(&produced, 0);
    atomic_i32_assign(&consumed, 0);
    
    os_handle_t producer_threads[num_threads];
    os_handle_t consumer_threads[num_threads];
    
    for (i32 i = 0; i < num_threads; i++) {
        producer_threads[i] = os_thread_create(thread_producer, nullptr);
        consumer_threads[i] = os_thread_create(thread_consumer, nullptr);
    }
    
    for (i32 i = 0; i < num_threads; i++) {
        os_thread_join(producer_threads[i]);
        os_thread_join(consumer_threads[i]);
    }
    
    printf("produced: %u\n", produced);
    printf("consumed: %u\n", consumed);
    
}


function void 
app_release() {
    gfx_context_release(context);
    os_window_close(window);
    arena_release(arena);
}

function void
app_frame() {
    os_get_events();
    
    // update window and context
    os_window_update(window);
    gfx_context_update(context);
    
    // full screen
    if (os_key_press(window, os_key_F11)) {
        os_window_fullscreen(window);
    }
    
    // close app
    if (os_key_press(window, os_key_esc) || (os_event_get(os_event_type_window_close) != 0)) {
        quit = true;
    }
    
    
    // build ui
    ui_begin(ui);
    
    ui_set_next_rect(rect(25.0f, 25.0f, 1025.0f, 525.0f));
    ui_profile_graph(str("profile_graph"));
    
    ui_end(ui);
    
    
    // render
    gfx_set_context(context);
    gfx_context_clear(context, color(0x1D1F20ff));
    
    ui_render(ui);
    
    gfx_context_present(context);
}

//- entry point

function i32 
app_entry_point(i32 argc, char** argv) {
    
    // init layers
    os_init();
    gfx_init();
    font_init();
    ui_init();
    profile_init();
    
    app_init();
    
    // main loop
    while (!quit) {
        app_frame();
    }
    
    // release
    app_release();
    
    profile_release();
    ui_release();
    font_release();
    gfx_release();
    os_release();
    
    return 0;
}