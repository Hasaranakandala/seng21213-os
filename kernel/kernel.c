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
#include "thread.h"
#include "mutex.h"

int myglobal = 0;
mutex_t mymutex;


void safe_thread_1(void* arg) {
    (void)arg;
    while (1) {
        mutex_lock(&mymutex);
        myglobal++;
        vga_printf("A");
        mutex_unlock(&mymutex);
         
       for(int i=0; i<100000; i++);
    }
}

void safe_thread_2(void* arg) {
    (void)arg;
    while (1) {
        mutex_lock(&mymutex);
        myglobal++;
        vga_printf("B");
        mutex_unlock(&mymutex);
  
        for(int i=0; i<100000; i++);
    }
}




extern void keyboard_init();
extern void shell_run();

// io.h ෆයිල් එක නැති නිසා outb function එක කෙලින්ම මෙතන ලියමු
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}



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
    vga_printf("Starting Stage 2: Threads & Mutex Demo...\n");

    
    mutex_init(&mymutex);
    thread_create(safe_thread_1, 0);
    thread_create(safe_thread_2, 0);


    init_timer(100);
     __asm__ volatile("sti");

    
    while(1) {
        
    }
}
