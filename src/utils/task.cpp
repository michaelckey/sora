// task.cpp

#ifndef TASK_CPP
#define TASK_CPP

//~ implementation 

function void
task_init(u32 worker_thread_count, u32 fiber_count) {
    
    const u32 max_task_count = 1024;
    
    // allocate arena
    task_state.arena = arena_create(megabytes(256));
    task_state.queue_arena = arena_create_aligned(megabytes(64), 64);
    
    // create queues
    _task_queue_init(&task_state.task_queue, max_task_count + 1);
    _task_queue_init(&task_state.free_task_queue, max_task_count + 1);
    _task_queue_init(&task_state.sleeping_fiber_queue, fiber_count + 1);
    _task_queue_init(&task_state.free_sleeping_fiber_queue, fiber_count + 1);
    _task_queue_init(&task_state.free_fiber_queue, fiber_count + 1);
    _task_queue_init(&task_state.free_counter_queue, fiber_count + 1);
    
    // allocate everything
    task_state.counter_count = fiber_count;
    task_state.task_count = max_task_count;
    task_state.fiber_count = fiber_count;
    task_state.worker_thread_count = worker_thread_count;
    
    task_state.counters = (atomic_i64*)arena_calloc(task_state.arena, sizeof(atomic_i64) * task_state.counter_count);
    task_state.tasks = (task_t*)arena_calloc(task_state.arena, sizeof(task_t) * task_state.task_count);
    task_state.fibers = (os_handle_t*)arena_calloc(task_state.arena, sizeof(os_handle_t) * task_state.fiber_count);
    task_state.sleeping_fibers = (task_sleeping_fiber_t*)arena_calloc(task_state.arena, sizeof(task_sleeping_fiber_t) * task_state.fiber_count);
    task_state.worker_threads = (os_handle_t*)arena_calloc(task_state.arena, sizeof(os_handle_t) * task_state.worker_thread_count);
    
    // set state to active
    atomic_i64_assign(&task_state.state, 1);
    
    // initialize everything 
    for (i64 i = 0; i < task_state.counter_count; i++) { _task_queue_enqueue(&task_state.free_counter_queue, i); }
    for (i64 i = 0; i < task_state.task_count; i++) { _task_queue_enqueue(&task_state.free_task_queue, i); }
    for (i64 i = 0; i < task_state.fiber_count; i++) { _task_queue_enqueue(&task_state.free_sleeping_fiber_queue, i); }
    
    for (i64 i = 0; i < task_state.fiber_count; i++) {
        task_state.fibers[i] = os_fiber_create(0, _task_fiber_function, nullptr);
        _task_queue_enqueue(&task_state.free_fiber_queue, i);
    }
    
    for (i32 i = 0; i < task_state.worker_thread_count; i++) { 
        task_state.worker_threads[i] = os_thread_create(_task_thread_function, nullptr);
    }
    
}

function void
task_release() {
    
    // set state to unactive
    atomic_i64_assign(&task_state.state, 0);
    
    // close threads
    for (i32 i = 0; i < task_state.worker_thread_count; i++) {
        os_thread_join(task_state.worker_threads[i]);
    }
    
    // release fibers
    for (i32 i = 0; i < task_state.fiber_count; i++) {
        os_fiber_release(task_state.fibers[i]);
    }
    
    // release arena
    arena_release(task_state.queue_arena);
    arena_release(task_state.arena);
    
}

function atomic_i64*
task_run(task_desc_t* descs, i64 count) {
    
    u32 tid = os_get_thread_id();
    
    // get a counter 
    i64 counter_index = -1;
    atomic_i64* counter = nullptr; 
    if (_task_queue_dequeue(&task_state.free_counter_queue, &counter_index)) {
        counter = &task_state.counters[counter_index];
        atomic_i64_assign(counter, count);
    } else {
        printf("failed to get a counter!\n");
    }
    
    //printf("got counter! c: %i\n", atomic_i64_load(counter));
    
    // create task
    for (i32 i = 0; i < count; i++) {
        
        i64 task_index;
        if (_task_queue_dequeue(&task_state.free_task_queue, &task_index)) {
            
            // fill task info
            task_t* task = &task_state.tasks[task_index];
            task->desc = descs[i];
            task->counter = counter;
            
            // push this index to task queue
            // TODO: maybe we don't push this here
            _task_queue_enqueue(&task_state.task_queue, task_index);
            
        }
    }
    
    return counter;
}

function void
task_wait_for_counter(atomic_i64* counter, i64 value) {
    u32 tid = os_get_thread_id();
    
    // if we aren't in a worker thread
    if (task_current_fiber_index == -1) {
        while (atomic_i64_load(counter) > value) {
            //os_sleep(1);
        }
    } else {
        
        // pull index for a sleeping fiber queue
        i64 free_sleeping_fiber_index;
        if (_task_queue_dequeue(&task_state.free_sleeping_fiber_queue, &free_sleeping_fiber_index)) {
            task_sleeping_fiber_t* sleeping_fiber = &task_state.sleeping_fibers[free_sleeping_fiber_index];
            
            // fill info
            sleeping_fiber->fiber_index = task_current_fiber_index;
            sleeping_fiber->counter = counter;
            sleeping_fiber->wait_condition = value;
            
            
            // get a unsued fiber to switch to. 
            i64 free_fiber_index;
            _task_queue_dequeue(&task_state.free_fiber_queue, &free_fiber_index);
            
            // push to sleeping fiber queue
            while (!_task_queue_enqueue(&task_state.sleeping_fiber_queue, free_sleeping_fiber_index)) {
                //os_sleep(1);
            }
            
            //printf("[tid: %u fiber_index: %i : task_wait_for_counter] \n   counter: %p (%i)\n   wait_condition: %i\n   fiber_switch: %i\n\n", tid, task_current_fiber_index, counter, atomic_i64_load(counter), value, free_fiber_index);
            
            // switch to unused fiber
            task_current_fiber_index = free_fiber_index;
            os_handle_t fiber = task_state.fibers[free_fiber_index];
            os_fiber_switch(task_state.fibers[free_fiber_index]);
            
        }
    }
    
    // once we are done, add the counter index back into the queue
    i64 counter_index = (i64)(counter - task_state.counters);
    while (!_task_queue_enqueue(&task_state.free_counter_queue, counter_index)) {
        //os_sleep(1);
    }
    
}

//- internal functions 


function void
_task_thread_function(void* params) {
    
    // turns the current thread into a fiber
    task_current_thread_fiber = os_fiber_from_thread();
    
    // get a free fiber from the fiber list 
    i64 free_fiber_index;
    _task_queue_dequeue(&task_state.free_fiber_queue, &free_fiber_index);
    
    // switch to that fiber
    task_current_fiber_index = free_fiber_index;
    os_fiber_switch(task_state.fibers[free_fiber_index]);
    
    // we are back
    u32 tid = os_get_thread_id();
    //printf("successfully returned from thread %x.\n", tid);
}

function void
_task_fiber_function(void* params) {
    
    u32 tid = os_get_thread_id();
    
    while (atomic_i64_load(&task_state.state)) {
        
        // check sleeping fibers and see if they can be swapped back in.
        i64 sleeping_fiber_index;
        if (_task_queue_dequeue(&task_state.sleeping_fiber_queue, &sleeping_fiber_index)) {
            
            task_sleeping_fiber_t* sleeping_fiber = &task_state.sleeping_fibers[sleeping_fiber_index]; 
            
            // check wait condition
            if (atomic_i64_load(sleeping_fiber->counter) <= sleeping_fiber->wait_condition) {
                
                // put current fiber back onto free fiber index
                while (!_task_queue_enqueue(&task_state.free_fiber_queue, task_current_fiber_index)) {
                    //os_sleep(1);
                }
                
                // switch to sleeping fiber
                task_current_fiber_index = sleeping_fiber->fiber_index;
                os_fiber_switch(task_state.fibers[sleeping_fiber->fiber_index]);
                
                // we got switched back here
                
            } else {
                // sleeping fiber wait condition was not met. add back to queue
                while (!_task_queue_enqueue(&task_state.sleeping_fiber_queue, sleeping_fiber_index)) {
                    //os_sleep(1);
                }
            }
        }
        
        // pull a task from the task queue
        i64 task_index;
        if (_task_queue_dequeue(&task_state.task_queue, &task_index)) {
            
            task_t* task = &task_state.tasks[task_index];
            
            // execute work
            if (task->desc.func != nullptr) {
                task->desc.func(task->desc.data);
            }
            
            // decrement the counter
            atomic_i64_dec(task->counter);
            
            // add task back to queue
            while (!_task_queue_enqueue(&task_state.free_task_queue, task_index)) {
                //os_sleep(1);
            }
            
        } else {
            // there was no task.
            os_sleep(1);
        }
    }
    
    // switch back to thread
    os_fiber_switch(task_current_thread_fiber);
    
}

//- internal queue functions 

function void 
_task_queue_init(task_queue_t* queue, u32 capacity) {
    
    u32 fixed_capacity = capacity;
    fixed_capacity--;
    fixed_capacity |= fixed_capacity >> 1; 
    fixed_capacity |= fixed_capacity >> 2; 
    fixed_capacity |= fixed_capacity >> 4; 
    fixed_capacity |= fixed_capacity >> 8; 
    fixed_capacity |= fixed_capacity >> 16;
    fixed_capacity = fixed_capacity + 1;
    
    queue->capacity = fixed_capacity;
    queue->mask = queue->capacity - 1;
    atomic_i64_assign(&queue->head, 0);
    atomic_i64_assign(&queue->tail, 0);
    
    queue->slots = (task_queue_slot_t*)arena_calloc(task_state.queue_arena, sizeof(task_queue_slot_t) * queue->capacity);
    
    for (i32 i = 0; i < queue->capacity; i++) {
        atomic_ptr_assign(&queue->slots[i].data, nullptr);
        atomic_i64_assign(&queue->slots[i].sequence, (i64)i);
    }
    
}

function b8
_task_queue_enqueue(task_queue_t* queue, i64 data) {
    
    i64 tail;
    task_queue_slot_t* slot;
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
                atomic_i64_assign(&slot->data, data);
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
_task_queue_dequeue(task_queue_t* queue, i64* data) {
    
    i64 head;
    task_queue_slot_t* slot;
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
                *data = atomic_i64_load(&slot->data);
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
            // queue is empty
            break;
        }
    }
    
    return result;
}

#endif // TASK_CPP