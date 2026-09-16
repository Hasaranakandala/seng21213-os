; boot/idt_load.asm
global idt_flush
idt_flush:
    mov eax, [esp+4]   ; get the pointer argument
    lidt [eax]         ; load the IDT pointer
    ret

