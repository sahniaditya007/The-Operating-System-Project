[bits 32]

; GDT load function
global i686_GDT_Load
i686_GDT_Load:
    ; Function signature: void i686_GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment)
    ; Arguments: 
    ;   [esp + 4] = descriptor
    ;   [esp + 8] = codeSegment  
    ;   [esp + 12] = dataSegment
    
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; Load GDT descriptor address
    lgdt [eax]              ; Load GDT
    
    mov eax, [ebp + 16]     ; Load data segment
    mov ds, ax              ; Set data segments
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to reload code segment
    mov eax, [ebp + 12]     ; Load code segment
    push eax                ; Push code segment
    push .reload_cs         ; Push return address
    retf                    ; Far return (reloads CS)
    
.reload_cs:
    pop ebp
    ret

; IDT load function  
global i686_IDT_Load
i686_IDT_Load:
    ; Function signature: void i686_IDT_Load(IDTDescriptor* idtDescriptor)
    ; Arguments:
    ;   [esp + 4] = idtDescriptor
    
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]      ; Load IDT descriptor address
    lidt [eax]              ; Load IDT
    
    pop ebp
    ret

; I/O and interrupt functions
global i686_outb
i686_outb:
    ; Function signature: void i686_outb(uint16_t port, uint8_t value)
    ; Arguments:
    ;   [esp + 4] = port
    ;   [esp + 8] = value
    
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]       ; Load port
    mov al, [ebp + 12]      ; Load value
    out dx, al              ; Output byte
    
    pop ebp
    ret

global i686_inb
i686_inb:
    ; Function signature: uint8_t i686_inb(uint16_t port)
    ; Arguments:
    ;   [esp + 4] = port
    ; Returns: value in AL
    
    push ebp
    mov ebp, esp
    
    mov dx, [ebp + 8]       ; Load port
    in al, dx               ; Input byte
    
    pop ebp
    ret

global i686_EnableInterrupts
i686_EnableInterrupts:
    ; Function signature: uint8_t i686_EnableInterrupts()
    ; Returns: previous interrupt flag state
    
    pushf                   ; Push flags
    pop eax                 ; Get flags in EAX
    and eax, 0x200          ; Check interrupt flag (bit 9)
    shr eax, 9              ; Shift to bit 0
    sti                     ; Enable interrupts
    ret

global i686_DisableInterrupts  
i686_DisableInterrupts:
    ; Function signature: uint8_t i686_DisableInterrupts()
    ; Returns: previous interrupt flag state
    
    pushf                   ; Push flags
    pop eax                 ; Get flags in EAX
    and eax, 0x200          ; Check interrupt flag (bit 9)
    shr eax, 9              ; Shift to bit 0  
    cli                     ; Disable interrupts
    ret

global i686_Panic
i686_Panic:
    ; Function signature: void i686_Panic()
    ; Infinite loop with interrupts disabled
    
    cli                     ; Disable interrupts
.hang:
    hlt                     ; Halt processor
    jmp .hang               ; Loop forever