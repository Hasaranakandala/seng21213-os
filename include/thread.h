#ifndef THREAD_H
#define THREAD_H

#include "process.h"
typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_DEAD
} thread_state_t;
typedef struct {
    int thread_id;
    int parent_pid;       
    thread_state_t state;
    uint32_t esp;        
    uint32_t stack[1024]; 
} thread_t;
void thread_create(void (*fn)(void*), void* arg);

#endif
