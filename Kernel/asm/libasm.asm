GLOBAL cpuVendor
GLOBAL kb_getKey
GLOBAL getRegisters
EXTERN rsp_aux
GLOBAL outb
GLOBAL inb
GLOBAL regs
GLOBAL callTimerTick
section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid

	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

kb_getKey:
    push rbp
    mov rbp, rsp

.wait_key:
    in al, 64h        
    and al, 1         
    jz .wait_key       


    in al, 60h
	movzx rax, al      
    mov rsp, rbp
    pop rbp
    ret



getRegisters: ; Deja el vector de registros en RAX. 
	push rsi
	mov rsi, [rsp_aux]
	mov rax, [rsi] ; rax
	mov [regs], rax

	mov rax, [rsi + 8*1] ; rbx
	mov [regs + 8*1], rax

	mov rax, [rsi + 8*2] ; rcx
	mov [regs + 8*2], rax

	mov rax, [rsi + 8*3] ; rdx
	mov [regs + 8*3], rax

	mov rax, [rsi + 8*4] ; rsi
	mov [regs + 8*4], rax

	mov rax, [rsi + 8*5] ; rdi
	mov [regs + 8*5], rax

	mov rax, [rsi + 8*6] ; rbp
	mov [regs + 8*6], rax

    mov rax, [rsi + 8*18] ; rsp
	mov [regs + 8*7], rax

	mov rax, [rsi + 8*7] ; r8
	mov [regs + 8*8], rax  ;r8

	mov rax, [rsi + 8*8] ; r9
	mov [regs + 8*9], rax  ;r9

	mov rax, [rsi + 8*9] ; r10
	mov [regs + 8*10], rax ; r10

	mov rax, [rsi + 8*10] ; r11
	mov [regs + 8*11], rax ; r11

	mov rax, [rsi + 8*11] ; r12
	mov [regs + 8*12], rax ; r12

	mov rax, [rsi + 8*12] ; r13
	mov [regs + 8*13], rax ; r13

	mov rax, [rsi + 8*13] ; r14
	mov [regs + 8*14], rax ; r14

	mov rax, [rsi + 8*14] ; r15
	mov [regs + 8*15], rax; r15

	mov rax, [rsi + 8*17] ; rflags
	mov [regs + 8*16], rax ;rflags

	mov rax, [rsi + 8*15] ; rip
	mov [regs + 8*17], rax 	  ; rip

	mov rax, [rsi + 8*16] ; cs
	mov [regs + 8*18], rax 	  ; cs

	pop rsi
	ret

outb:
    push rbp              
    mov rbp, rsp           
    mov dx, di             
    mov al, sil            
    out dx, al
    mov rsp, rbp           
    pop rbp                
    ret                    

inb:
	push rbp
	mov rbp, rsp
    mov dx, di             
    in  al, dx             
    movzx eax, al
	mov rsp, rbp
	pop rbp        
    ret                    

callTimerTick:
	int 20h
	ret

section .bss
regs resq 19 ; 19 x 8 bytes 