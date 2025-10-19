/**
 * @file main.c
 * @brief This is the main entry point for the kernel.
 */

#include <stdint.h>

#include <hal/hal.h>

#include <arch/i686/irq.h>

#include <debug.h>

#include <boot/bootparams.h>

#include <arch/i686/vga_gfx.h>

#include <util/font.h>

#include <arch/i686/mouse.h>

#include <arch/i686/fpu.h>



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

    // printf(".");

}



/**

 * @brief The main entry point of the kernel.

 * 

 * @param bootParams A pointer to the boot parameters passed from the bootloader.

 */

void start(BootParams* bootParams)

{

    // Call global constructors

    _init();



    // Initialize the Hardware Abstraction Layer (HAL).

    HAL_Initialize();

    // Initialize Floating Point Unit to prevent x87 exceptions.
    i686_FPU_Initialize();

    mouse_install();



    vga_set_mode(0x13);

    vga_put_pixel(10, 10, 15);

    vga_put_pixel(11, 10, 7);

    vga_put_pixel(12, 10, 8);



    font_draw_char(20, 20, 'A', 15);



    vga_draw_rect(30, 30, 50, 40, 15);

    vga_fill_rect(90, 30, 50, 40, 7);



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















    // Register a timer handler (commented out).

    //i686_IRQ_RegisterHandler(0, timer);



    // A test function to trigger a crash (commented out).

    //crash_me();



    // Halt the CPU.

    while(1) {

        vga_fill_rect(0, 0, 320, 200, 0);

        draw_cursor(mouse_x, mouse_y, 15);

    }



    }



    