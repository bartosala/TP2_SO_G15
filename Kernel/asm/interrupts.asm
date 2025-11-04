GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler
GLOBAL _irq80Handler

GLOBAL _exception0Handler
GLOBAL _exception6Handler

EXTERN exceptionDispatcher
EXTERN syscallDispatcher
EXTERN schedule
EXTERN timer_handler
EXTERN bufferWrite

SECTION .text


%macro pushState 0
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rbp
	push rdi
	push rsi
	push rdx
	push rcx
	push rbx
	push rax

%endmacro

%macro popState 0
	pop rax
	pop rbx
	pop rcx
	pop rdx 
	pop rsi 
	pop rdi 
	pop rbp 
	pop r8 
	pop r9 
	pop r10 
	pop r11 
	pop r12 
	pop r13 
	pop r14 
	pop r15
%endmacro

%macro exceptionHandler 1
	pushState
	
	mov rdi, %1 			; pasaje de parametro de la excepcion
	call exceptionDispatcher

	popState
	iretq
%endmacro

_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick)
_irq00Handler: ; basado en "interesting_handler de la práctica"
	pushState
	call timer_handler
	
	mov rdi, rsp
	call schedule ; Llama al scheduler
	mov rsp, rax
	mov al, 20h
	out 20h, al ; EOI
	
	popState
	iretq

;Keyboard
_irq01Handler:
	pushState

	call bufferWrite

	mov al, 20h
	out 20h, al ; EOI

	popState
	iretq


_irq80Handler:
	push rbp ; registros a preservar
    mov rbp, rsp
    push rbx

    ; Pasaje de parametros x86_64
    mov rdi, rax  ; Número de syscall
    mov rsi, rbx  ; Primer parámetro
	mov rax, rdx ;
    mov rdx, rcx  ; Segundo parámetro
    mov rcx, rax  ; Tercer parámetro

    ; Llamar a la función syscallDispatcher
    call syscallDispatcher
	
	pop rbx
	mov rsp, rbp
	pop rbp
    iretq

;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

;Invalid Opcode Exception
_exception6Handler:
	exceptionHandler 6

haltcpu:
	cli
	hlt
	ret

SECTION .bss
	aux resq 1