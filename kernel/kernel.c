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
#include "idt.h"
#include "fs.h"
#include "pmm.h"
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



extern void pic_remap(void);
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
/*
void timer_init() {
    outb(0x43, 0x36);
    int divisor = 11931; 
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}
*/
void timer_init(int freq) {
    int divisor = 1193180 / freq;   // PIT base frequency = 1193180 Hz
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);

}
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

#define COM1_PORT 0x3F8

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Divisor low byte (38400 baud)
    outb(COM1_PORT + 1, 0x00);    // Divisor high byte
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static int serial_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;   // bit 5 = Transmit Holding Register Empty
}

void serial_putc(char c) {
    while (!serial_transmit_empty());   // wait until UART is ready for next byte
    outb(COM1_PORT, c);
}




 void serial_print(const char *s) {
    while (*s) serial_putc(*s++);
}

void kernel_main() {
    serial_init();
    serial_print("1: before vga_init\n");
    vga_init();
    serial_print("2: after vga_init\n");

    vga_printf("Starting Stage 2: Threads & Mutex Demo...\n");
    serial_print("3: after vga_printf\n");

    pmm_init(32 * 1024 * 1024);
    fs_init();
    serial_print("8: after fs_init\n");
    serial_print("4: after pmm_init\n");

    mutex_init(&mymutex);
    serial_print("5: after mutex_init\n");

    pic_remap();
    serial_print("5b: after pic_remap\n");

    idt_init();
    serial_print("5c: after idt_init\n");

    timer_init(100);
    serial_print("6: after timer_init\n");
     
     
      
     __asm__ volatile("cli");
    serial_print("=== Stage 4 Verification: 5 files ===\n");
    fs_create("a.txt"); fs_write("a.txt", "File1", 5);
    fs_create("b.txt"); fs_write("b.txt", "File2", 5);
    fs_create("c.txt"); fs_write("c.txt", "File3", 5);
    fs_create("d.txt"); fs_write("d.txt", "File4", 5);
    fs_create("e.txt"); fs_write("e.txt", "File5", 5);
    serial_print("--- After creating 5 files ---\n");
    fs_list();
    fs_unlink("a.txt");
    fs_unlink("b.txt");
    serial_print("--- After deleting 2 files ---\n");
    fs_list();
    serial_print("=== Verification Complete ===\n");
    __asm__ volatile("sti");
    
    __asm__ volatile("sti");

    serial_print("7: after sti\n");
    
     
    
    while (1) {
        shell_run();
    }
}
