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
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Mask all IRQs except IRQ0 (timer) — we haven't installed handlers
    // for keyboard, RTC, floppy, etc. yet, and unhandled spurious IRQs
    // (common in emulated hardware) would jump to an empty IDT entry,
    // causing a #GP fault and corrupting execution. (L09 §3 / L11 §1)
    outb(PIC1_DATA, 0xFE);  // 11111110 -> only IRQ0 unmasked
    outb(PIC2_DATA, 0xFF);  // 11111111 -> all slave IRQs masked
}

// This runs every time the timer fires (100 times/sec if timer_init(100) used)
volatile uint32_t tick_count = 0;

void irq0_handler(void) {
    tick_count++;
    // Stage 1 scheduler logic will go here later (context switch call)
    outb(PIC1_CMD, 0x20);  // send End-Of-Interrupt to PIC — MANDATORY
}
