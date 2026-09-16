; boot/irq0.asm
global irq0_stub
extern irq0_handler

irq0_stub:
    pusha              ; save all general registers
    call irq0_handler  ; call C handler
    popa               ; restore registers
    iretd               ; return from interrupt
