; =============================================================================
; A simple "Hello, World!" bootloader for 16-bit real mode.
; =============================================================================

org 0x7C00  ; The BIOS loads bootloaders at memory address 0x7C00.
bits 16     ; We are in 16-bit real mode.

%define ENDL 0x0D, 0x0A ; Define a newline macro (unused in this version).

start:
    jmp main    ; Jump to the main execution point.

; --- puts function ---
; Prints a null-terminated string to the screen using BIOS interrupt 0x10.
; Input: DS:SI should point to the beginning of the string.
; Output: None.
; Clobbers: AX.
puts:
    ; Save the registers we are about to modify so we can restore them later.
    push si
    push ax
    push bx ; It's good practice to save BX as int 0x10/ah=0x0e uses it.

.loop:
    lodsb       ; Load byte from [DS:SI] into AL, and increment SI.
    or al, al   ; Check if the byte in AL is zero (the null terminator).
    jz .done    ; If it is zero, we are done, so jump to the .done label.

    mov ah, 0x0E ; BIOS teletype output function.
    mov bh, 0x00 ; Page number.
    mov bl, 0x07 ; Text attribute (light grey on black).
    int 0x10     ; Call the BIOS video interrupt to print the character in AL.

    jmp .loop    ; Go back to the start of the loop to print the next character.

.done:
    ; Restore the registers we saved at the beginning in reverse order.
    pop bx
    pop ax
    pop si      ; Corrected from 'push si' to 'pop si'.
    ret         ; Return from the function.


main:
    ; --- Setup Segments and Stack ---
    ; We need to initialize our segment registers to a known state.
    mov ax, 0      ; Can't write 0 directly to a segment register.
    mov ds, ax     ; Set Data Segment (DS) to 0.
    mov es, ax     ; Set Extra Segment (ES) to 0.

    ; Set up the stack to grow downwards from our code's starting address.
    mov ss, ax     ; Set Stack Segment (SS) to 0.
    mov sp, 0x7C00 ; Set Stack Pointer (SP) to 0x7C00.

    ; --- Print Welcome Message ---
    mov si, msg_hello ; Point SI to our hello world message.
    call puts         ; Call the puts function to print it.

    ; --- Halt Execution ---
    hlt               ; Halt the CPU to save power.

.hang:
    jmp .hang         ; Infinite loop as a fallback if hlt fails.


; --- Data Section ---
msg_hello: db 'Hello, world!', ENDL, 0 ; The null-terminated string to print.


; --- Bootloader Signature ---
; The BIOS checks for these last two bytes (0xAA55) to identify a valid
; bootable device. We pad the file with zeros to ensure the signature
; is at the correct position (bytes 511 and 512).
times 510-($-$$) db 0
dw 0xAA55
