/* =============================================================================
 * SENG21213-OS :: Main Kernel  (Stage 0 – Foundations)
 * File   : kernel/kernel.c
 *
 * PURPOSE
 *   This is the heart of your operating system. Right now it:
 *     1. Initialises VGA text-mode display
 *     2. Initialises the keyboard driver
 *     3. Prints a splash screen
 *     4. Runs a minimal interactive shell ("ksh")
 *
 * ASSIGNMENT MILESTONES  (what YOU will add in later lectures)
 *   Lecture  9  – Process Management  →  process.h / process.c / scheduler.c
 *   Lecture 10  – Threads             →  thread.h  / thread.c
 *   Lecture 11  – Memory Management   →  pmm.h     / pmm.c / vmm.c
 *   Lecture 12  – File System         →  fs.h      / fs.c
 *
 * CODING CONVENTION
 *   - Prefix kernel-internal functions with k_ (e.g. k_strcmp)
 *   - All driver APIs live in their own .h/.c pair
 *   - NEVER call malloc – use the PMM you build in Lecture 11
 * ============================================================================*/
#include "vga.h"
#include <stdint.h>
#include "process.h"
#include "keyboard.h"
extern void keyboard_init();
extern void shell_run();

// io.h ෆයිල් එක නැති නිසා outb function එක කෙලින්ම මෙතන ලියමු
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

extern void schedule();

void process_a() {
    while (1) {
        vga_printf("A");
        for (volatile int i = 0; i < 10000000; i++); 
    }
}

void process_b() {
    while (1) {
        vga_printf("B");
        for (volatile int i = 0; i < 20000000; i++);
    }
}

void timer_init() {
    outb(0x43, 0x36);
    int divisor = 11931; 
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void kernel_main() {
    vga_init();
   
   // keyboard_init();
    
    create_process(process_a,"Process A");
    create_process(process_b,"Process B");

    timer_init();

    shell_run();
}
