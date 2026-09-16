#ifndef IDT_H
#define IDT_H
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} idt_entry_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

void idt_set_gate(int n, uint32_t handler, uint16_t sel, uint8_t flags);
void idt_init(void);

#endif
