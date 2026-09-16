[BITS 32]
global switch_context

switch_context:
    pusha

    mov eax, [esp + 36]
    mov [eax], esp

    mov esp, [esp + 40]

    popa
    ret
