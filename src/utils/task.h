// task.h

#ifndef TASK_H
#define TASK_H

//~ typedefs 

typedef void task_function(void*);

//~ structs

// queue

struct task_queue_slot_t {
    atomic_i64 data;
    atomic_i64 sequence;
};

struct task_queue_t {
    __declspec(align(64)) atomic_i64 head;
    __declspec(align(64)) atomic_i64 tail;
    u32 capacity;
    u32 mask;
    __declspec(align(64)) task_queue_slot_t* slots;
};

// tasks 

struct task_desc_t {
    task_function* func;
    void* data;
};

struct task_t {
    task_desc_t desc;
    atomic_i64* counter;
};

struct task_sleeping_fiber_t {
    i64 fiber_index;
    atomic_i64* counter;
    i64 wait_condition;
};

// state

struct task_state_t {
    
    arena_t* arena;
    arena_t* queue_arena;
    
    u32 worker_thread_count;
    os_handle_t* worker_threads;
    
    u32 fiber_count;
    os_handle_t* fibers;
    task_sleeping_fiber_t* sleeping_fibers;
    
    u32 task_count;
    task_t* tasks;
    
    u32 counter_count;
    atomic_i64* counters;
    
    // queues
    task_queue_t task_queue;
    task_queue_t free_task_queue;
    task_queue_t sleeping_fiber_queue;
    task_queue_t free_sleeping_fiber_queue;
    task_queue_t free_fiber_queue;
    task_queue_t free_counter_queue;
    
    atomic_i64 state;
    
};

global task_state_t task_state;
thread_global os_handle_t task_current_thread_fiber = { 0 };
thread_global i64 task_current_fiber_index = -1;

//~ functions 

function void task_init(u32 worker_thread_count, u32 fiber_count);
function void task_release();

function atomic_i64* task_run(task_desc_t* descs, i64 count);
function void task_wait_for_counter(atomic_i64* counter, i64 value);

// internal

function void _task_queue_init(task_queue_t* queue, u32 capacity);
function b8 _task_queue_enqueue(task_queue_t* queue, i64 value);
function b8 _task_queue_dequeue(task_queue_t* queue, i64* value);

function void _task_thread_function(void* params);
function void _task_fiber_function(void* params);

#endif // TASK_H