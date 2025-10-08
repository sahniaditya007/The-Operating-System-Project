; =============================================================================
; A simple "Hello, World!" bootloader for 16-bit real mode.
; =============================================================================

org 0x7C00                  ; The BIOS loads bootloaders at memory address 0x7C00.
bits 16                     ; We are in 16-bit real mode.

%define ENDL 0x0D, 0x0A     ; Define a newline macro (Carriage Return + Line Feed).

#FAT12 header
jmp short start             ; Jump over the header to executable code.
nop                         ; Padding (2-byte jump instruction alignment).

; --- BIOS Parameter Block (BPB) and FAT12 header ---
bdb_oem:        db 'MSWIN4.1'      ; OEM identifier (8 bytes, standard label for FAT disks)
bdb_bytes_per_sector:       dw 512  ; Standard sector size for floppy disks (512 bytes)
bdb_sector_per_cluster:     db 1    ; One sector per cluster (common for floppies)
bdb_reserved_sectors:       dw 1    ; The boot sector itself
bdb_fat_count:              db 2    ; Two copies of the File Allocation Table (FAT)
bdb_dir_entries_count:      dw 0E0h ; 224 root directory entries
bdb_total_sectors:          dw 2880 ; 2880 * 512 = 1.44 MB total disk size
bdb_media_descriptor_type:  db 0F0h ; 0xF0 indicates a 3.5" floppy disk
bdb_sectors_per_fat:        dw 9    ; Each FAT table occupies 9 sectors
bdb_sectors_per_track:      dw 18   ; 18 sectors per track (standard floppy geometry)
bdb_heads:                  dw 2    ; 2 heads (double-sided)
bdb_hidden_sectors:         dw 0    ; No hidden sectors on a floppy
bdb_large_sector_count:     dd 0    ; Not used for floppies (used in larger drives)

#extended boot record
edr_drive_number:           db 0    ; 0x00 for floppy drive, 0x80 for HDD
                            db 0    ; Reserved byte (unused)
edr_signature:              db 29h  ; Extended boot signature for FAT12
ebr_volume_id:              db 12h, 34h, 56h, 78h ; Volume serial number (unique identifier)
ebr_volume_label:           db 'NANOBYTE OS'      ; Volume label (11 chars, padded with spaces)
ebr_system_id:              db 'FAT12'            ; Filesystem type identifier (8 chars)

; =============================================================================
; CODE SECTION
; =============================================================================

start:
    jmp main    ; Jump to the main execution point. Keeps header data intact.

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
                   ; This ensures the stack starts right after the bootloader.

    ; Read something from floppy disk.
    ; BIOS should set DL to drive number when bootloader is loaded.
    mov [edr_drive_number], dl   ; Save current drive number for later BIOS calls.

    mov ax, 1       ; LBA = 1 (second sector of disk, after bootloader)
    mov cl, 1       ; Read 1 sector only
    mov bx, 0x7E00  ; Data should be loaded after bootloader (safe memory area)
    call disk_read  ; Call disk reading routine to fetch data

    ; --- Print Welcome Message ---
    mov si, msg_hello ; Point SI to our hello world message.
    call puts         ; Call the puts function to print it.

    ; --- Halt Execution ---
    cli               ; Disable interrupts to prevent accidental interrupts
    hlt               ; Halt the CPU to save power and stop execution.

; =============================================================================
; ERROR HANDLERS
; =============================================================================

floopy_error:
    mov si, msg_read_failed  ; Load address of read failure message.
    call puts                ; Display the message on screen.
    jmp wait_key_and_reboot  ; Wait for a keypress, then reboot.

wait_key_and_reboot:
    mov ah, 0
    int 16h             ; Wait for keypress (BIOS keyboard interrupt).
    jmp 0FFFFh:0        ; Jump to BIOS reset vector at FFFF:0000.
    hlt                 ; Safety halt if BIOS reboot fails.

.halt:
    cli                 ; Disable interrupts, CPU cannot escape halt state.
    hlt

; =============================================================================
; DISK ROUTINES
; =============================================================================

; Converts an LBA address to a CHS address
; Parameters:
;   ax: LBA address
; Returns:
;   cx [bits 0-5]: sector number
;   cx [bits 6-15]: cylinder
;   dh: head
;
lba_to_chs:

    push ax                      ; Preserve LBA value
    push dx

    xor dx, dx                   ; Clear DX before division
    div word [bdb_sectors_per_track] ; Divide LBA by sectors/track
                                    ; AX = LBA / sectors_per_track (temp quotient)
                                    ; DX = LBA % sectors_per_track (remainder = sector)
    inc dx                       ; Sector numbers start at 1, not 0.
    mov cx, dx                   ; Store sector number in CX.

    xor dx, dx
    div word [bdb_heads]         ; Divide quotient by heads.
                                 ; AX = cylinder number
                                 ; DX = head number
    mov dh, dl                   ; DH = head value.
    mov ch, al                   ; CH = cylinder low 8 bits.
    shl ah, 6                    ; Shift upper 2 bits of cylinder into position.
    or cl, ah                    ; Combine them with CL (sector + upper cylinder bits).

    pop ax
    mov dl, al                   ; Restore drive number in DL (if BIOS overwrote)
    pop ax
    ret 

; -----------------------------------------------------------------------------
; Reads sectors from a disk using BIOS interrupt 13h.
; Parameters:
;   ax: LBA address
;   cl: number of sectors to read (up to 128)
;   dl: drive number
;   es:bx: memory address where to store read data
; -----------------------------------------------------------------------------
disk_read:

    push ax             ; Save registers we will modify
    push bx
    push cx
    push dx
    push di 

    push cx                 ; Temporarily save CL (sector count)
    call lba_to_chs         ; Convert LBA to CHS before calling BIOS
    pop ax                  ; AL = number of sectors to read

    mov ah, 02h             ; BIOS function: Read sectors (INT 13h, AH=02h)
    mov di, 3               ; Retry count (3 times before giving up)

.retry:
    pusha                   ; Save all general registers (BIOS may change them)
    stc                     ; Set Carry Flag (some BIOSes fail to clear it on error)
    int 13h                 ; BIOS Disk Service: read sectors into ES:BX
    jnc .done               ; Jump if no error (Carry cleared = success)

    ; Read failed
    popa
    call disk_reset         ; Attempt to reset disk and try again

    dec di                  ; Decrement retry counter
    test di, di             ; Check if retries remain
    jnz .retry              ; If yes, retry the operation

.fail:
    ; All attempts exhausted, show error message
    jmp floopy_error

.done:
    popa                    ; Restore register state

    ; Restore registers in reverse order
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; -----------------------------------------------------------------------------
; Reset disk controller using BIOS interrupt 13h function 00h.
; -----------------------------------------------------------------------------
disk_reset:
    pusha
    mov ah, 0               ; AH=0 resets the disk system
    stc                     ; Set Carry Flag (for consistency)
    int 13h                 ; BIOS call: reset disk
    jc floopy_error         ; If Carry still set, reset failed
    popa
    ret

; =============================================================================
; DATA SECTION
; =============================================================================
msg_hello: db 'Hello, world!', ENDL, 0          ; Null-terminated print string.
msg_read_failed: db 'Read from disk failed!', ENDL, 0 ; Error message.

; =============================================================================
; BOOTLOADER SIGNATURE
; The BIOS checks for the word 0xAA55 at the end of the first sector to
; recognize a valid bootable disk. This must appear at offset 510.
; =============================================================================
times 510-($-$$) db 0         ; Pad remaining bytes of the 512-byte sector with zeros.
dw 0xAA55                     ; Boot signature (must be last two bytes).
