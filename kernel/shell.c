#include <pmm.h>
#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include <stdint.h>
#include "fs.h"

typedef enum { PROCESS_READY, PROCESS_RUNNING, PROCESS_BLOCKED, PROCESS_TERMINATED } process_state_t;
typedef struct { uint32_t pid; process_state_t state; uint32_t stack_pointer; uint32_t entry_point; } pcb_t;

extern pcb_t process_table[];
extern uint32_t next_pid;

void cmd_help(char *arg) {
    (void)arg;
    vga_printf("Commands: help, clear, ps, meminfo, ls, touch, cat, write, rm\n");
}

void cmd_clear(char *arg) {
    (void)arg;
    vga_clear(0);
}

void cmd_meminfo(char *arg) {
    (void)arg;
    uint32_t total, used, free;
    pmm_get_memory_stats(&total, &used, &free);
    vga_printf("Total Memory: %d MB\n", total / (1024 * 1024));
    vga_printf("Used Memory: %d KB\n", used / 1024);
    vga_printf("Free Memory: %d KB\n", free / 1024);
}

void cmd_ps(char *arg) {
    (void)arg;
    vga_printf("PID  STATE\n");
    for (uint32_t i = 1; i < next_pid; i++) {
        char pid_str[2] = {i + '0', '\0'};
        vga_printf(pid_str);
        vga_printf("    ");
        if (process_table[i].state == PROCESS_RUNNING) vga_printf("RUNNING\n");
        else vga_printf("READY\n");
    }
}

void cmd_ls(char *arg) {
    (void)arg;
    fs_list();
}

void cmd_touch(char *arg) {
    if (arg[0] == '\0') { vga_printf("Usage: touch <name>\n"); return; }
    if (fs_create(arg) == 0)
        vga_printf("Created: %s\n", arg);
    else
        vga_printf("Error: could not create %s\n", arg);
}

void cmd_cat(char *arg) {
    if (arg[0] == '\0') { vga_printf("Usage: cat <name>\n"); return; }
    char buf[4096];
    int n = fs_read(arg, buf, sizeof(buf) - 1);
    if (n < 0)
        vga_printf("Error: file not found\n");
    else
        vga_printf("%s\n", buf);
}

void cmd_write(char *arg) {
    if (arg[0] == '\0') { vga_printf("Usage: write <name> <text>\n"); return; }

    char *name = arg;
    char *text = arg;
    while (*text && *text != ' ') text++;
    if (*text == ' ') {
        *text = '\0';
        text++;
    } else {
        vga_printf("Usage: write <name> <text>\n");
        return;
    }

    int n = fs_write(name, text, strlen(text));
    if (n < 0)
        vga_printf("Error: file not found (use touch first)\n");
    else
        vga_printf("Wrote %d bytes\n", n);
}

void cmd_rm(char *arg) {
    if (arg[0] == '\0') { vga_printf("Usage: rm <name>\n"); return; }
    if (fs_unlink(arg) == 0)
        vga_printf("Removed: %s\n", arg);
    else
        vga_printf("Error: file not found\n");
}

struct command {
    const char* name;
    void (*handler)(char *arg);
};

struct command commands[] = {
    {"help", cmd_help},
    {"clear", cmd_clear},
    {"ps", cmd_ps},
    {"meminfo", cmd_meminfo},
    {"ls", cmd_ls},
    {"touch", cmd_touch},
    {"cat", cmd_cat},
    {"write", cmd_write},
    {"rm", cmd_rm}
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

        char *cmd_name = input;
        char *arg = input;
        while (*arg && *arg != ' ') arg++;
        if (*arg == ' ') {
            *arg = '\0';
            arg++;
            while (*arg == ' ') arg++;
        }

        int matched = 0;
        for (unsigned int i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
            if (strcmp(cmd_name, commands[i].name) == 0) {
                commands[i].handler(arg);
                matched = 1;
                break;
            }
        }
        if (!matched && cmd_name[0] != '\0') {
            vga_printf("Unknown command: %s\n", cmd_name);
        }
    }
}
