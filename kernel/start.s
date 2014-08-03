BITS 32
ALIGN 4


mboot:
	; Multiboot headers:
	;   Page aligned loading, please
	MULTIBOOT_PAGE_ALIGN	equ 1<<0
	;   We require memory information
	MULTIBOOT_MEMORY_INFO	equ 1<<1
	;   We would really, really like graphics...
	MULTIBOOT_USE_GFX		equ 1<<2
	;   We are multiboot compatible!
	MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
	;   Load up those flags.
	MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_USE_GFX
	;   Checksum the result
	MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
	; Load the headers into the binary image.
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM
	dd 0x00000000 ; header_addr
	dd 0x00000000 ; load_addr
	dd 0x00000000 ; load_end_addr
	dd 0x00000000 ; bss_end_addr
	dd 0x00000000 ; entry_addr
	; Graphics requests
	dd 0x00000000 ; 0 = linear graphics
	dd 0
	dd 0
	dd 32         ; Set me to 32 or else.



; Some external references.
extern code, bss, end

; Main entrypoint
global start
start:
	; Set up stack pointer.
	mov esp, 0x7FFFF
	push esp
	; Push the incoming mulitboot headers
	push eax ; Header magic
	push ebx ; Header pointer
	; Disable interrupts
	cli
	; Call the C entry
	extern	start_kernel
	call	start_kernel
	jmp		$
extern gdtpointer
extern idtpointer

extern do_irq


;exports
global idt_flush
global gdt_flush
global tss_flush
global kputhex
global kputint

	
gdt_flush:
	lgdt [gdtpointer] 
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	jmp 0x08:flush
flush:
	ret ; returns back to C-code

idt_flush:
	lidt [idtpointer]
	ret
	
tss_flush:
	mov ax, 0x2b
	ltr ax
	ret




global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31
global _isr127
;  0: Divide By Zero Exception
isr0:
    cli
    push byte 0
    push byte 0
    jmp isr_common_stub

;  1: Debug Exception
isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_common_stub

;  2: Non Maskable Interrupt Exception
isr2:
    cli
    push byte 0
    push byte 2
    jmp isr_common_stub

;  3: Int 3 Exception
isr3:
    cli
    push byte 0
    push byte 3
    jmp isr_common_stub

;  4: INTO Exception
isr4:
    cli
    push byte 0
    push byte 4
    jmp isr_common_stub

;  5: Out of Bounds Exception
isr5:
    cli
    push byte 0
    push byte 5
    jmp isr_common_stub

;  6: Invalid Opcode Exception
isr6:
    cli
    push byte 0
    push byte 6
    jmp isr_common_stub

;  7: Coprocessor Not Available Exception
isr7:
    cli
    push byte 0
    push byte 7
    jmp isr_common_stub

;  8: Double Fault Exception (With Error Code!)
isr8:
    cli
    push byte 8
    jmp isr_common_stub

;  9: Coprocessor Segment Overrun Exception
isr9:
    cli
    push byte 0
    push byte 9
    jmp isr_common_stub

; 10: Bad TSS Exception (With Error Code!)
isr10:
    cli
    push byte 10
    jmp isr_common_stub

; 11: Segment Not Present Exception (With Error Code!)
isr11:
   cli
    push byte 11
    jmp isr_common_stub

; 12: Stack Fault Exception (With Error Code!)
isr12:
    cli
    push byte 12
    jmp isr_common_stub

; 13: General Protection Fault Exception (With Error Code!)
isr13:
    cli
    push byte 13
    jmp isr_common_stub

; 14: Page Fault Exception (With Error Code!)
isr14:
   cli
    push byte 14
    jmp isr_common_stub

; 15: Reserved Exception
isr15:
    cli
    push byte 0
    push byte 15
    jmp isr_common_stub

; 16: Floating Point Exception
isr16:
    cli
    push byte 0
    push byte 16
    jmp isr_common_stub

; 17: Alignment Check Exception
isr17:
    cli
    push byte 0
    push byte 17
    jmp isr_common_stub

; 18: Machine Check Exception
isr18:
    cli
    push byte 0
    push byte 18
    jmp isr_common_stub

; 19: Reserved
isr19:
    cli
    push byte 0
    push byte 19
    jmp isr_common_stub

; 20: Reserved
isr20:
    cli
    push byte 0
    push byte 20
    jmp isr_common_stub

; 21: Reserved
isr21:
    cli
    push byte 0
    push byte 21
    jmp isr_common_stub

; 22: Reserved
isr22:
    cli
    push byte 0
    push byte 22
    jmp isr_common_stub

; 23: Reserved
isr23:
    cli
    push byte 0
    push byte 23
    jmp isr_common_stub

; 24: Reserved
isr24:
   cli
    push byte 0
    push byte 24
    jmp isr_common_stub

; 25: Reserved
isr25:
    cli
    push byte 0
    push byte 25
    jmp isr_common_stub

; 26: Reserved
isr26:
    cli
    push byte 0
    push byte 26
    jmp isr_common_stub

; 27: Reserved
isr27:
    cli
    push byte 0
    push byte 27
    jmp isr_common_stub

; 28: Reserved
isr28:
    cli
    push byte 0
    push byte 28
    jmp isr_common_stub

; 29: Reserved
isr29:
    cli
    push byte 0
    push byte 29
    jmp isr_common_stub

; 30: Reserved
isr30:
    cli
    push byte 0
    push byte 30
    jmp isr_common_stub

; 31: Reserved
isr31:
    cli
    push byte 0
    push byte 31
    jmp isr_common_stub

_isr127:
    cli
    push byte 0
    push byte 32
    jmp isr_syscall
extern isr_handler
isr_common_stub:
push eax
    push ecx
    push edx
    push ebx
    
    push ebp
    push esi
    push edi

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

push esp
call isr_handler
mov esp, eax

pop gs
    pop fs
    pop es
    pop ds

    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    add esp, 8
sti
    iret
; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
extern syscall_handler

isr_syscall:
push eax
    push ecx
    push edx
    push ebx
    
    push ebp
    push esi
    push edi

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

push esp
call syscall_handler
mov esp, eax

pop gs
    pop fs
    pop es
    pop ds

    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    add esp, 8
sti
    iret
    



    
   ;here is the irq adresses so we can access it from C code 
global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15


; 33: IRQ1
irq0:
    cli
    push byte 0
    push byte 32
    jmp irq_common_stub
; 33: IRQ1
irq1:
    cli
    push byte 0
    push byte 33
    jmp irq_common_stub

; 34: IRQ2
irq2:
    cli
    push byte 0
    push byte 34
    jmp irq_common_stub

; 35: IRQ3
irq3:
    cli
    push byte 0
    push byte 35
    jmp irq_common_stub

; 36: IRQ4
irq4:
    cli
    push byte 0
    push byte 36
    jmp irq_common_stub

; 37: IRQ5
irq5:
    cli
    push byte 0
    push byte 37
    jmp irq_common_stub

; 38: IRQ6
irq6:
    cli
    push byte 0
    push byte 38
    jmp irq_common_stub

; 39: IRQ7
irq7:
    cli
    push byte 0
    push byte 39
    jmp irq_common_stub

; 40: IRQ8
irq8:
    cli
    push byte 0
    push byte 40
    jmp irq_common_stub

; 41: IRQ9
irq9:
    cli
    push byte 0
    push byte 41
    jmp irq_common_stub

; 42: IRQ10
irq10:
    cli
    push byte 0
    push byte 42
    jmp irq_common_stub

; 43: IRQ11
irq11:
    cli
    push byte 0
    push byte 43
    jmp irq_common_stub

; 44: IRQ12
irq12:
    cli
    push byte 0
    push byte 44
    jmp irq_common_stub

; 45: IRQ13
irq13:
    cli
    push byte 0
    push byte 45
    jmp irq_common_stub

; 46: IRQ14
irq14:
    cli
    push byte 0
    push byte 46
    jmp irq_common_stub

; 47: IRQ15
irq15:
    cli
    push byte 0
    push byte 47
    jmp irq_common_stub

extern irq_handler
extern do_IRQ
extern do_IRQ_
;here we save the processor state, calls the fault handler, then restore the stack frame 
irq_common_stub:
cli
push eax
    push ecx
    push edx
    push ebx
    
    push ebp
    push esi
    push edi

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

push esp
call do_IRQ
mov esp, eax

pop gs
    pop fs
    pop es
    pop ds

    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    add esp, 8
sti
    iret


global read_eip
read_eip: ; Clever girl
	pop eax
	jmp eax

global copy_page_physical
copy_page_physical:
    push ebx
    pushf
    cli
    mov ebx, [esp+12]
    mov ecx, [esp+16]
    mov edx, cr0
    and edx, 0x7FFFFFFF
    mov cr0, edx
    mov edx, 0x400
.page_loop:
    mov eax, [ebx]
    mov [ecx], eax
    add ebx, 4
    add ecx, 4
    dec edx
    jnz .page_loop
    mov edx, cr0
    or  edx, 0x80000000
    mov cr0, edx
    popf
    pop ebx
    ret

; Return to Userspace (from thread creation)
global return_to_userspace
return_to_userspace:
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret


section .bss

hexkod: resb 16
section .text

kputhex:
pusha
lea esi, [hexkod + 14] 
.loop:
xor edx, edx
mov ebx, 16
div ebx	
mov byte [esi], dl	
cmp dl, 0xa
jge .bokstaver

add byte [esi], 0x30	
jmp .end	
.bokstaver:
mov byte [esi], dl
add byte [esi], 'A' - 10
.end:
dec esi
cmp eax, 0	
jne .loop


    dec esi	
mov byte [esi], '0'
mov byte [esi +1], 'x'
push esi

pop esi
popa
ret

section .bss
numstr: resb 16
section .text
kputint:


lea esi, [numstr + 14] 
.loop:
xor edx, edx 
mov ebx, 0xa 
div ebx


mov byte [esi], dl 
add byte [esi], 0x30

dec esi 
cmp eax, 0 
jne .loop 
inc esi 
push esi

pop esi
ret
; BSS Section
SECTION .bss
	resb 8192 ; 8KB of memory reserved
