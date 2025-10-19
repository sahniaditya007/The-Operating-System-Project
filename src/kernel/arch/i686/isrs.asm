
;
; isrs.asm
;
; Defines the macros for generating ISRs and includes the generated ISR file.
;

extern i686_ISR_Handler

%macro ISR_NOERRORCODE 1
[bits 32]
global i686_ISR%1
i686_ISR%1:
    push dword 0    ; push a dummy error code
    push dword %1   ; push the interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRORCODE 1
[bits 32]
global i686_ISR%1
i686_ISR%1:
    push dword %1   ; push the interrupt number
    jmp isr_common_stub
%endmacro

[bits 32]
isr_common_stub:
    pusha           ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds      ; Lower 16-bits of eax = ds.
    push eax        ; save the data segment descriptor

    mov ax, 0x10    ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call i686_ISR_Handler
    add esp, 4

    pop eax         ; reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa            ; Pops edi,esi,ebp,esp,ebx,edx,ecx,eax
    add esp, 8      ; Cleans up the pushed error code and interrupt number
    iret            ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

%include "isrs_gen.asm"
