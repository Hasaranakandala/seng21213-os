#include "thread.h"

extern thread_t threads[];
extern int thread_count;
int current_thread = -1;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint32_t schedule(uint32_t esp) {
    if (thread_count == 0) return esp;
    
    if (current_thread != -1) {
        threads[current_thread].esp = esp;
        threads[current_thread].state = THREAD_READY;
    }
    
    current_thread = (current_thread + 1) % thread_count;
    threads[current_thread].state = THREAD_RUNNING;
    
    return threads[current_thread].esp;
}

void init_timer(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}
