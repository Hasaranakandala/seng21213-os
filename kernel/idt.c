#include "idt.h"
#include "string.h"

#define IDT_ENTRIES 256

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;

// defined in idt_load.asm
extern void idt_flush(uint32_t);
extern void irq0_stub(void); 

void idt_set_gate(int n, uint32_t handler, uint16_t sel, uint8_t flags) {
    idt[n].base_low  = handler & 0xFFFF;
    idt[n].base_high = (handler >> 16) & 0xFFFF;
    idt[n].sel       = sel;
    idt[n].always0   = 0;
    idt[n].flags     = flags;
}

void idt_init(void) {
    idt_ptr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    idt_ptr.base  = (uint32_t)&idt;

    uint8_t *p = (uint8_t *)&idt;
    for (uint32_t i = 0; i < sizeof(idt_entry_t) * IDT_ENTRIES; i++) {
        p[i] = 0;
    }

    idt_set_gate(0x20, (uint32_t)irq0_stub, 0x08, 0x8E);
    idt_flush((uint32_t)&idt_ptr);


    // IRQ0 (timer) -> vector 0x20, kernel code segment 0x08, present+ring0+32-bit-int-gate
    extern void irq0_stub(void);
    idt_set_gate(0x20, (uint32_t)irq0_stub, 0x08, 0x8E);

    idt_flush((uint32_t)&idt_ptr);
}
