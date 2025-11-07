.global sys

.extern read
.extern write
.extern malloc

.text
.intel_syntax noprefix

sys:
    push ebp
    mov ebp, esp
    sub esp, 10
    push ebx
    push esi
    push edi

    # [ebp - 10] - 10 byte array
    # [ebp] - ebp
    # [ebp + 4] - return address
    # [ebp + 8] - adress of a

    mov ebx, esp # keep old esp

    push 1000
    call malloc

    mov esp, eax # make new esp
    add esp, 1000


    # prepare buffer
    mov [ebp - 10], dword PTR 0
    mov [ebp - 6], dword PTR 0
    mov [ebp - 2], word PTR 0

    mov edi, ebp # buffer offset
    sub edi, 10

    # 2 syscalls on the malloc stack
    push dword PTR 10
    push edi
    push dword PTR 0
    call read
    add esp, 12

    push dword PTR 10
    push edi
    push dword PTR 1
    call write
    add esp, 12

    mov esi, [ebp + 8] # add 5 to a
    add [esi], dword PTR 5

    mov esp, ebx # restore esp

    pop edi
    pop esi
    pop ebx
    add esp, 10
    mov esp, ebp
    pop ebp
    ret
