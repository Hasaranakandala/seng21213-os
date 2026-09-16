#include <stdint.h>
   // if you don't have io.h, use the outb() already in kernel.c

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

void pic_remap(void) {
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    outb(PIC1_DATA, 0x20); // IRQ0-7  -> interrupt vectors 0x20-0x27
    outb(PIC2_DATA, 0x28); // IRQ8-15 -> interrupt vectors 0x28-0x2F
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, 0x0);
    outb(PIC2_DATA, 0x0);
}

// This runs every time the timer fires (100 times/sec if timer_init(100) used)
volatile uint32_t tick_count = 0;

void irq0_handler(void) {
    tick_count++;
    // Stage 1 scheduler logic will go here later (context switch call)
    outb(PIC1_CMD, 0x20);  // send End-Of-Interrupt to PIC — MANDATORY
}
