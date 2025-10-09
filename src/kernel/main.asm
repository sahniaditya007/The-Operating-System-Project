; ==============================================
; Simple 16-bit "Hello World" Kernel
; ==============================================

org 0x0              ; Origin at address 0x0000 — where the code will be loaded
bits 16              ; Assemble for 16-bit real mode

%define ENDL 0x0D, 0x0A   ; Define newline characters (Carriage Return + Line Feed)

; ==============================================
; Entry Point
; ==============================================
start:
    ; Load the address of the message string into SI register
    mov si, msg_hello

    ; Call the 'puts' subroutine to print the string
    call puts

.halt:
    cli               ; Clear interrupts (disable them)
    hlt               ; Halt the CPU — system stops here

; ==============================================
; Subroutine: puts
; ----------------------------------------------
; Prints a null-terminated string using BIOS interrupt 0x10.
; Parameters:
;   ds:si → Pointer to the string in memory
; Destroys:
;   al, ah (used for character output)
; Preserves:
;   si, ax, bx (restored before return)
; ==============================================
puts:
    ; Save registers that will be modified
    push si
    push ax
    push bx

.loop:
    lodsb             ; Load byte at [si] into AL and increment SI
    or al, al         ; Check if AL == 0 (end of string)
    jz .done          ; If zero, jump to end of function

    mov ah, 0x0E      ; BIOS teletype output function (INT 10h, AH=0Eh)
    mov bh, 0         ; Page number (0)
    int 0x10          ; Call BIOS to print the character in AL

    jmp .loop         ; Repeat for next character

.done:
    ; Restore saved registers
    pop bx
    pop ax
    pop si

    ret               ; Return to caller

; ==============================================
; Data Section
; ==============================================
; Null-terminated string with CR+LF at the end
msg_hello: db 'Hello world from KERNEL!', ENDL, 0
