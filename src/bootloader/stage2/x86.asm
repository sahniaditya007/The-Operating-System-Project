; ===============================================
; x86_Video_WriteCharTeletype
; -----------------------------------------------
; Description:
;   Prints a single character on the screen using
;   BIOS interrupt 0x10, function 0x0E (teletype mode).
;
; Calling convention:
;   _cdecl (C standard calling convention)
;
; Parameters (stack layout, small memory model):
;   [BP + 0] → previous BP (saved frame pointer)
;   [BP + 2] → return address
;   [BP + 4] → character (char c)
;   [BP + 6] → page number (byte page)
;
; Notes:
;   - Uses INT 10h AH=0Eh to print a character.
;   - Advances cursor automatically.
;   - This is used by putc() and puts() in C.
; ===============================================

bits 16                         ; Assemble for 16-bit real mode

section _TEXT class=CODE         ; Define code section

global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:

    ; -------------------------------------------
    ; Prologue: set up call frame
    ; -------------------------------------------
    push bp                     ; Save previous base pointer
    mov bp, sp                  ; Establish new stack frame

    ; -------------------------------------------
    ; Save registers we’ll modify
    ; -------------------------------------------
    push bx                     ; BIOS call modifies BX, so preserve it

    ; -------------------------------------------
    ; Retrieve parameters from stack
    ; -------------------------------------------
    ; Stack layout:
    ;   [BP + 4] = character (char c)
    ;   [BP + 6] = page number
    ;
    ; AH = 0x0E (BIOS teletype output function)
    ; AL = character to print
    ; BH = display page number
    ; BIOS interrupt: INT 10h
    mov ah, 0Eh                 ; Select teletype output function
    mov al, [bp + 4]            ; Load character into AL
    mov bh, [bp + 6]            ; Load page number into BH

    int 10h                     ; Call BIOS interrupt for video output

    ; -------------------------------------------
    ; Epilogue: restore registers and stack
    ; -------------------------------------------
    pop bx                      ; Restore BX register

    mov sp, bp                  ; Reset stack pointer
    pop bp                      ; Restore previous base pointer
    ret                         ; Return to caller
