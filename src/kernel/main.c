/**
 * @file main.c
 * @brief This is the main entry point for the kernel.
 */

#include <stdint.h>

#include <hal/hal.h>

#include <arch/i686/irq.h>
#include <arch/i686/io.h>

#include <debug.h>

#include <boot/bootparams.h>

#include <arch/i686/vga_gfx.h>

#include <util/font.h>

#include <arch/i686/mouse.h>

#include <arch/i686/fpu.h>

#include <arch/i686/keyboard.h>

#include <gui/gui.h>



// External function to call global constructors.

extern void _init();



// A test function to trigger a crash.

void crash_me();



/**

 * @brief A simple timer handler that prints a dot to the screen.

 * 

 * @param regs The registers at the time of the interrupt.

 */

void timer(Registers* regs)

{

    (void)regs;

    // Timer increments are handled in GUI loop

    // This just ensures timer interrupts are working

}



/**

 * @brief The main entry point of the kernel.

 * 

 * @param bootParams A pointer to the boot parameters passed from the bootloader.

 */

BootParams* g_BootParams = NULL;

void start(BootParams* bootParams)

{

    // Store boot params globally
    g_BootParams = bootParams;

    // Call global constructors

    _init();



    // Initialize Floating Point Unit to prevent x87 exceptions.
    i686_FPU_Initialize();

    // Initialize the Hardware Abstraction Layer (HAL).
    HAL_Initialize();

    // Initialize mouse and keyboard
    mouse_install();
    keyboard_install();

    // CRITICAL: Ensure interrupts are enabled after device installation
    i686_EnableInterrupts();

    vga_set_mode(0x13);



    // Log boot information.

    log_debug("Main", "Boot device: %x", bootParams->BootDevice);

    log_debug("Main", "Memory region count: %d", bootParams->Memory.RegionCount);

    for (int i = 0; i < bootParams->Memory.RegionCount; i++) 

    {

        log_debug("Main", "MEM: start=0x%llx length=0x%llx type=%x", 

            bootParams->Memory.Regions[i].Begin,

            bootParams->Memory.Regions[i].Length,

            bootParams->Memory.Regions[i].Type);

    }















    // Register a timer handler
    i686_IRQ_RegisterHandler(0, timer);
    
    // Unmask timer interrupt
    const PICDriver* pic = i686_IRQ_GetDriver();
    if (pic) {
        pic->Unmask(0);  // Timer IRQ
    }

    // Initialize GUI system
    gui_init();
    
    // CRITICAL: Ensure interrupts are still enabled before main loop
    i686_EnableInterrupts();
    
    log_debug("Main", "GUI initialized, entering main loop");

    // Main GUI loop
    while(1) {
        // Enable interrupts - they should already be enabled, but ensure it
        __asm__ __volatile__("sti");
        
        gui_update();
        
        // Small delay to prevent CPU hogging
        // Interrupts can still fire during this delay
        for (volatile int i = 0; i < 10000; i++);
    }



    }



    