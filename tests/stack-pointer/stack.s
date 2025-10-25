.global stack_test

.extern malloc
.extern fastcall_adder

.text
.intel_syntax noprefix

stack_test:
    push rbp
    mov rbp, rsp
    push rbx

    mov rbx, rdi # keep &a
    mov r12, rsp # keep old rsp

    mov rdi, 1000
    call malloc

    mov rsp, rax # move rsp to newly allocated stack
    add rsp, 1000 # to the end

    mov rdi, 0
    mov rsi, 0
    mov rdx, 0
    mov rcx, 0
    mov r8, 0
    mov r9, 0
    push rbx # 7 args to have smth to push with fastcall
    call fastcall_adder
    
    mov rsp, r12 # restore old rsp

    pop rbx
    mov rsp, rbp
    pop rbp
    ret
