GLOBAL cpuVendor
GLOBAL kb_getKey
GLOBAL outb
GLOBAL inb
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