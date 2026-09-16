#include "thread.h"
#include "vga.h"

#define MAX_THREADS 16

thread_t threads[MAX_THREADS];
int thread_count = 0;

void thread_create(void (*fn)(void*), void* arg) {
    if (thread_count >= MAX_THREADS) {
        vga_printf("Error: Maximum threads reached!\n");
        return;
    }

    thread_t* t = &threads[thread_count];
    t->thread_id = thread_count;
    t->state = THREAD_READY;

    uint32_t* sp = &t->stack[1024];

    *(--sp) = (uint32_t)arg;
    *(--sp) = 0;
    *(--sp) = (uint32_t)fn;

    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;

    t->esp = (uint32_t)sp;
    
    thread_count++;
}
