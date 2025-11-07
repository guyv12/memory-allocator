.global switch_stack

.extern malloc

.data

.text
.intel_syntax noprefix

switch_stack:
    mov ebx, [esp] # return adress

    push dword PTR 10000
    call malloc

    mov esp, eax
    add esp, 10000
    
    mov [esp], ebx # return adress

    ret