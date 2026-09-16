#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef enum {
    PROC_FREE,
    PROC_READY,
    PROC_RUNNING,
    PROC_BLOCKED
} process_state_t;

typedef struct {
    uint32_t pid;
    process_state_t state;
    uint32_t esp;
    uint32_t stack[1024];
    char name[32];
} pcb_t;

void process_init();
void create_process(void (*entry_fn)(), const char* name);
uint32_t schedule(uint32_t esp);
void init_timer(uint32_t freq);

#endif
