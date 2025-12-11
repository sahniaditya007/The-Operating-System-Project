# Interview Quick Reference - Operating System Project

## Elevator Pitches

### 15-Second Version
*"I built a 32-bit operating system from scratch in C and Assembly. It boots from disk, has a GUI with mouse and keyboard support, and includes multiple applications."*

### 30-Second Version  
*"I built a complete operating system for the x86 architecture. It has a custom bootloader, FAT filesystem, device drivers for keyboard and mouse, and a GUI with seven applications - all written from scratch without using any existing OS or libraries."*

### 60-Second Version (for longer introductions)
*"I built a 32-bit operating system from scratch for the x86 architecture. It includes a custom bootloader, FAT filesystem support, hardware drivers for keyboard and mouse, and a graphical user interface with multiple applications. Everything is written in C and Assembly without using any existing OS or standard libraries. The project demonstrates deep understanding of computer architecture, low-level programming, and operating system concepts."*

---

## Core Technologies
- **C** and **x86 Assembly (NASM)**
- **i686 (32-bit x86) architecture**
- **SCons build system**
- **Custom cross-compiler toolchain** (i686-elf-gcc)
- **QEMU/Bochs** for testing

---

## Main Components (5 Key Areas)

### 1. Bootloader (Two-Stage)
- **Stage 1**: 512-byte MBR bootloader in Assembly
- **Stage 2**: Extended bootloader in C that loads the kernel
- Implements FAT filesystem reader and ELF loader

### 2. Hardware Abstraction Layer (HAL)
- **GDT**: Memory segmentation for protected mode
- **IDT**: Maps interrupts to handler functions
- **ISR**: CPU exception handlers
- **IRQ**: Hardware interrupt handlers

### 3. Device Drivers
- **PS/2 Keyboard**: Scancode to ASCII conversion
- **PS/2 Mouse**: Tracks position and button states
- **VGA Graphics**: Direct framebuffer manipulation (320x200)

### 4. Memory Management
- BIOS interrupt (INT 15h E820h) for memory detection
- Passes memory map to kernel via boot parameters

### 5. GUI System
- Event-driven architecture
- 7 applications: Desktop, Clock, Calendar, Notepad, Snake, Settings, Monitor
- Windows 1.0 inspired UI with 3D borders

---

## Technical Deep-Dive Topics

### Be Ready to Explain:

#### 1. Boot Process Flow
```
BIOS → Stage 1 (MBR) → Stage 2 (Bootloader) → Kernel → GUI
```

#### 2. How VGA Graphics Work
```c
// Direct hardware access at 0xA0000
uint8_t* vga = (uint8_t*)0xA0000;
vga[y * 320 + x] = color;  // Write pixel
```

#### 3. Interrupt Handling
```c
// Register handler
i686_IRQ_RegisterHandler(0, timer_handler);

// Hardware fires interrupt → CPU looks up IDT → Handler called
```

#### 4. Why Cross-Compiler?
- Normal compilers target existing OS (Linux/Windows)
- Need compiler that targets "bare metal" (no OS)
- i686-elf-gcc produces code for standalone x86

---

## Common Interview Questions & Answers

### Q: "Why two-stage bootloader?"
**A**: "BIOS only loads 512 bytes (MBR). That's too small for filesystem code and kernel loader. Stage 1 fits in 512 bytes and loads Stage 2, which has no size limit."

### Q: "How do you debug without printf/gdb?"
**A**: "I use QEMU with GDB remote debugging, serial port logging for kernel messages, and analyze CPU registers when exceptions occur. Also added debug output to VGA screen."

### Q: "What's the hardest part?"
**A**: "Understanding the boot process and mode transitions. CPU starts in 16-bit real mode, but modern code needs 32-bit protected mode. Getting this transition right while setting up GDT and IDT correctly took a lot of debugging."

### Q: "How does keyboard input work?"
**A**: "PS/2 keyboard sends scancodes on port 0x60 when keys are pressed. My driver reads these scancodes, converts them to ASCII characters, and stores them in a queue for applications to consume."

### Q: "What's the memory layout?"
**A**: 
```
0x00000000 - 0x000003FF: Real Mode IVT (Interrupt Vector Table)
0x00000400 - 0x000004FF: BIOS Data Area
0x00000500 - 0x00007BFF: Free memory (Stage 2 loads here)
0x00007C00 - 0x00007DFF: Stage 1 Bootloader (512 bytes)
0x00100000+            : Kernel loaded here (1MB+)
0x000A0000 - 0x000BFFFF: VGA framebuffer
```

### Q: "Can you explain how FAT filesystem works?"
**A**: "FAT uses a table (File Allocation Table) that maps clusters. Each file has a starting cluster number. The FAT tells you which cluster comes next, forming a linked list. You follow this chain to read the entire file."

### Q: "What would you add next?"
**A**: "Multitasking with process scheduling, memory paging for virtual memory, and file write support. Also higher resolution graphics using VESA modes."

---

## Code Examples to Memorize

### Example 1: Kernel Entry Point
```c
void start(BootParams* bootParams) {
    // Initialize FPU
    i686_FPU_Initialize();
    
    // Initialize HAL (GDT, IDT, ISR, IRQ)
    HAL_Initialize();
    
    // Install drivers
    mouse_install();
    keyboard_install();
    
    // Enable interrupts
    i686_EnableInterrupts();
    
    // Set graphics mode
    vga_set_mode(0x13);
    
    // GUI loop
    gui_init();
    while(1) { gui_update(); }
}
```

### Example 2: Drawing a Pixel
```c
void vga_put_pixel(int x, int y, uint8_t color) {
    uint8_t* vga = (uint8_t*)0xA0000;
    if (x >= 0 && x < 320 && y >= 0 && y < 200) {
        vga[y * 320 + x] = color;
    }
}
```

### Example 3: Interrupt Handler
```c
void timer_handler(Registers* regs) {
    // Called automatically by hardware timer
    tick_count++;
}

// Registration
i686_IRQ_RegisterHandler(0, timer_handler);
```

---

## Key Statistics
- **~3,000+ lines** of C and Assembly code
- **86 source files** total
- **Boot time**: < 1 second (QEMU)
- **Resolution**: 320x200 pixels, 256 colors
- **7 GUI applications** implemented
- **3 device drivers**: Keyboard, Mouse, VGA

---

## Architecture Diagram (ASCII)

```
┌─────────────────────────────────────────────┐
│           USER APPLICATIONS                 │
│  Desktop | Clock | Calendar | Notepad ...   │
├─────────────────────────────────────────────┤
│           GUI FRAMEWORK                     │
│  Event Loop | Window Manager | Drawing      │
├─────────────────────────────────────────────┤
│           DEVICE DRIVERS                    │
│  Keyboard | Mouse | VGA Graphics            │
├─────────────────────────────────────────────┤
│  HARDWARE ABSTRACTION LAYER (HAL)           │
│  GDT | IDT | ISR | IRQ                      │
├─────────────────────────────────────────────┤
│           KERNEL CORE                       │
│  Memory Manager | Interrupt Handler         │
├─────────────────────────────────────────────┤
│           BOOTLOADER                        │
│  Stage 1 (MBR) → Stage 2 (ELF Loader)       │
├─────────────────────────────────────────────┤
│           HARDWARE                          │
│  CPU | Memory | Disk | VGA | PS/2           │
└─────────────────────────────────────────────┘
```

---

## Skills Demonstrated

### Technical Skills:
- ✅ Low-level C programming
- ✅ x86 Assembly language
- ✅ Computer architecture knowledge
- ✅ Operating system concepts
- ✅ Hardware interfacing
- ✅ Interrupt-driven programming
- ✅ Memory management
- ✅ Filesystem implementation
- ✅ Graphics programming
- ✅ Driver development

### Soft Skills:
- ✅ Self-directed learning
- ✅ Problem-solving
- ✅ Debugging complex systems
- ✅ Project planning
- ✅ Documentation writing
- ✅ Build system configuration

---

## Red Flags to Avoid

❌ **DON'T SAY**: "I followed a tutorial step-by-step"
✅ **DO SAY**: "I learned from OSDev wiki and Intel manuals, then implemented my own design"

❌ **DON'T SAY**: "It's just a simple OS"
✅ **DO SAY**: "It demonstrates core OS concepts used in production systems like Linux"

❌ **DON'T SAY**: "I don't know how X works"
✅ **DO SAY**: "I implemented X using [technical approach], let me explain..."

---

## Before the Interview

### Review These Files:
1. `src/kernel/main.c` - Kernel entry point
2. `src/bootloader/stage2/main.c` - Bootloader logic
3. `src/kernel/hal/hal.c` - HAL initialization
4. `src/kernel/gui/gui.c` - GUI system
5. `src/kernel/arch/i686/vga_gfx.c` - Graphics driver

### Test Your Knowledge:
- Can you explain the boot process in 2 minutes?
- Can you draw the memory layout from memory?
- Can you explain how interrupts work?
- Can you describe how a pixel gets drawn?
- Can you explain the two-stage bootloader?

---

## Practice Answers (60 seconds each)

### "Tell me about this project"
*"I built a 32-bit operating system from scratch for the x86 architecture. It starts with a two-stage bootloader written in Assembly and C that loads the kernel from a FAT filesystem. The kernel initializes the hardware abstraction layer, sets up interrupt handling, and loads device drivers for keyboard, mouse, and VGA graphics. On top of that, I built a GUI system with event-driven architecture and multiple applications. The entire project is about 3,000 lines of C and Assembly, and boots in under a second in QEMU."*

### "What did you learn?"
*"This project taught me how computers really work at the hardware level. I learned about memory segmentation, interrupt handling, direct hardware I/O, and how operating systems abstract hardware for applications. The debugging skills I gained were invaluable - when something crashes, you have to examine CPU registers and memory dumps to figure out what went wrong. I also learned to read technical specifications like Intel manuals and implement complex protocols like the PS/2 keyboard interface."*

### "Why build an OS?"
*"I wanted to deeply understand how software interacts with hardware. When you use a system call in Linux or Windows, you're using abstractions built by the OS. I wanted to understand what's underneath those abstractions. Building an OS forces you to implement everything from scratch - memory management, device drivers, filesystems - which gives you a complete understanding of the full stack from hardware to application."*

---

## Resources URLs (Memorize These)

- **OSDev Wiki**: https://wiki.osdev.org/
- **Intel Manuals**: https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html
- **Project Repository**: [Your GitHub URL]

---

## Final Tips

1. **Be enthusiastic** - Show passion for low-level programming
2. **Use diagrams** - Draw the boot process or memory layout
3. **Be specific** - Reference actual code and line numbers
4. **Admit unknowns** - "I haven't implemented X yet, but here's how I would..."
5. **Connect to real world** - Compare to Linux, Windows, GRUB, etc.
6. **Show growth mindset** - Discuss what you'd improve next

---

*Quick review: Read this document before the interview, practice the "60-second answers", and review the key code files listed above. You've got this!*
