#include <stdint.h>

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

typedef struct {
    uint32_t pid;
    process_state_t state;
    uint32_t stack_pointer;
    uint32_t entry_point;
} pcb_t;

#define MAX_PROCESSES 16

extern pcb_t process_table[MAX_PROCESSES];
extern uint32_t next_pid;

extern void switch_context(uint32_t* old_sp, uint32_t new_sp);

uint32_t current_pid = 0;

void schedule() {
    if (next_pid <= 1) {
        return;
    }

    uint32_t old_pid = current_pid;
    uint32_t next = (current_pid + 1) % next_pid;

    if (next == 0) {
        next = 1; 
    }

    current_pid = next;
    
    if (process_table[old_pid].state == PROCESS_RUNNING) {
        process_table[old_pid].state = PROCESS_READY;
    }
    process_table[current_pid].state = PROCESS_RUNNING;

    switch_context(&process_table[old_pid].stack_pointer, process_table[current_pid].stack_pointer);
}

