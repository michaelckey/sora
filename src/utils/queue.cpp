// queue.cpp

#ifndef QUEUE_CPP
#define QUEUE_CPP

//~ implementation

function queue_t*
queue_create(u32 capacity) {
    
    // round capacity up to the nearest power of 2
    u32 fixed_capacity = capacity;
    fixed_capacity--;
    fixed_capacity |= fixed_capacity >> 1; 
    fixed_capacity |= fixed_capacity >> 2; 
    fixed_capacity |= fixed_capacity >> 4; 
    fixed_capacity |= fixed_capacity >> 8; 
    fixed_capacity |= fixed_capacity >> 16;
    fixed_capacity = fixed_capacity + 1;
    
    u32 arena_size = (sizeof(queue_t) + (sizeof(queue_slot_t) * fixed_capacity));
    arena_t* arena = arena_create_aligned(arena_size + kilobytes(4), 64);
    queue_t* queue = (queue_t*)arena_calloc(arena, sizeof(queue_t));
    
    queue->arena = arena;
    queue->capacity = fixed_capacity;
    queue->mask = queue->capacity - 1;
    atomic_i64_assign(&queue->head, 0);
    atomic_i64_assign(&queue->tail, 0);
    
    queue->slots = (queue_slot_t*)arena_calloc(arena, sizeof(queue_slot_t) * queue->capacity);
    
    for (i32 i = 0; i < queue->capacity; i++) {
        atomic_ptr_assign(&queue->slots[i].data, nullptr);
        atomic_i64_assign(&queue->slots[i].sequence, (i64)i);
    }
    
    return queue;
}

function void
queue_release(queue_t* queue) {
    arena_release(queue->arena);
}

function b8
queue_enqueue(queue_t* queue, void* data) {
    
    i64 tail;
    queue_slot_t* slot;
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
    queue_slot_t* slot;
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


#endif // QUEUE_CPP