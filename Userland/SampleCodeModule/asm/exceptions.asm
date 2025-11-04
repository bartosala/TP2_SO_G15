GLOBAL invalid_opcode
GLOBAL div_zero

section .text

invalid_opcode:
    mov cr6, rax
    ret

div_zero:
    mov rax, 0
    div rax
    ret