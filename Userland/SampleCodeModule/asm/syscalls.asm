;syscalls.asm

GLOBAL syscall

section .text
syscall:
    push rbp ; registros a preservar
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r15
 
    mov rax, rdi
    mov rbx, rsi
    mov rdi, rdx 
    mov rdx, rcx
    mov rcx, rdi
    int 80h        ; efectúa la syscall 
    
    pop r15 
    pop r13
    pop r12
    pop rbx
    mov rsp, rbp
    pop rbp
    ret