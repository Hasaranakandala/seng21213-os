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
#define STACK_SIZE 4096

pcb_t process_table[MAX_PROCESSES];
uint8_t process_stacks[MAX_PROCESSES][STACK_SIZE];
uint32_t next_pid = 1;

int create_process(void (*entry_fn)()) {
    if (next_pid >= MAX_PROCESSES) {
        return -1;
    }

    uint32_t pid = next_pid++;
    pcb_t* pcb = &process_table[pid];

    pcb->pid = pid;
    pcb->state = PROCESS_READY;
    pcb->entry_point = (uint32_t)entry_fn;

    pcb->stack_pointer = (uint32_t)&process_stacks[pid][STACK_SIZE];

    return pid;
}
