#include <pmm.h>
#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include <stdint.h>

typedef enum { PROCESS_READY, PROCESS_RUNNING, PROCESS_BLOCKED, PROCESS_TERMINATED } process_state_t;
typedef struct { uint32_t pid; process_state_t state; uint32_t stack_pointer; uint32_t entry_point; } pcb_t;

extern pcb_t process_table[];
extern uint32_t next_pid;

void cmd_help() { vga_printf("Commands: help, clear, ps, meminfo\n"); }
void cmd_clear() { vga_clear(0); }

void cmd_meminfo() {
    uint32_t total, used, free;
    pmm_get_memory_stats(&total, &used, &free);

    vga_printf("Total Memory: %d MB\n", total / (1024 * 1024));
    vga_printf("Used Memory: %d KB\n", used / 1024);
    vga_printf("Free Memory: %d KB\n", free / 1024);
}

void cmd_ps() {
    vga_printf("PID  STATE\n");
    for (uint32_t i = 1; i < next_pid; i++) {
        char pid_str[2] = {i + '0', '\0'};
        vga_printf(pid_str);
        vga_printf("    ");
        if (process_table[i].state == PROCESS_RUNNING) vga_printf("RUNNING\n");
        else vga_printf("READY\n");
    }
}

struct command {
    const char* name;
    void (*handler)();
};

struct command commands[] = {
    {"help", cmd_help},
    {"clear", cmd_clear},
    {"ps", cmd_ps},
    {"meminfo", cmd_meminfo}
};

void shell_run() {
    char input[128];
    int pos = 0;

    while (1) {
        vga_printf("kernel> ");
        pos = 0;

        while (1) {
            char c = kb_getchar();
            if (c == '\n') {
                input[pos] = '\0';
                vga_printf("\n");
                break;
            } else if (c != 0) {
                input[pos++] = c;
                char str[2] = {c, '\0'};
                vga_printf(str);
            }
        }

        for (unsigned int i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
            if (strcmp(input, commands[i].name) == 0) {
                commands[i].handler();
            }
        }
    }
}
