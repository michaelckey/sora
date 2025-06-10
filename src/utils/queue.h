// queue.h

#ifndef QUEUE_H
#define QUEUE_H

//~ structs 

struct queue_slot_t {
    void* data;
    atomic_i64 sequence;
};

struct queue_t {
    
    arena_t* arena;
    
    __declspec(align(64)) atomic_i64 head;
    __declspec(align(64)) atomic_i64 tail;
    
    u32 capacity;
    u32 mask;
    
    __declspec(align(64)) queue_slot_t* slots;
    
};

//~ functions 

function queue_t* queue_create(u32 capacity);
function void queue_release(queue_t* queue);

function b8 queue_enqueue(queue_t* queue, void* data);
function b8 queue_dequeue(queue_t* queue, void** data);

#endif // QUEUE_H