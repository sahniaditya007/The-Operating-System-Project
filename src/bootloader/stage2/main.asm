; ===============================================
; Stage 2 Bootloader Entry Point (16-bit mode)
; ===============================================
bits 16                         ; Assemble for 16-bit real mode instructions

section _ENTRY class=CODE        ; Define the code section (entry point segment)

extern _cstart_                  ; C function defined elsewhere (in C source)
global entry                     ; Export the 'entry' label as the program entry point


; ===============================================
; Entry Point
; -----------------------------------------------
; This is the first code executed when Stage 2
; gains control from the bootloader.
; It sets up the CPU environment and then calls
; the C runtime entry function (_cstart_).
; ===============================================
entry:
    cli                          ; Disable interrupts to avoid unwanted behavior

    ; -------------------------------------------
    ; Stack setup
    ; -------------------------------------------
    mov ax, ds                   ; Copy current data segment selector into AX
    mov ss, ax                   ; Set SS = DS (stack segment same as data)
    mov sp, 0                    ; Initialize stack pointer to offset 0
    mov bp, sp                   ; Set base pointer to same location (optional)
    sti                          ; Re-enable interrupts (safe now that stack exists)

    ; -------------------------------------------
    ; Pass boot drive number to C function
    ; -------------------------------------------
    ; When Stage 1 jumps here, DL holds the boot drive number (e.g., 0x00 = floppy, 0x80 = HDD)
    ; The function _cstart_ expects a 16-bit argument for the boot drive in DX,
    ; so we zero out DH and push DX to pass it properly.
    xor dh, dh                   ; Zero upper 8 bits of DX
    push dx                      ; Push boot drive number onto the stack
    call _cstart_                ; Call C entry point function

    ; -------------------------------------------
    ; Halt the CPU after returning from C code
    ; -------------------------------------------
    cli                          ; Disable interrupts again
    hlt                          ; Halt the CPU (system stops here)
