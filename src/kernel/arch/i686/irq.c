/**
 * @file irq.c
 * @brief This file contains the implementation of the Interrupt ReQuest (IRQ) handlers.
 */

#include "irq.h"
#include "pic.h"
#include "i8259.h"
#include "io.h"
#include <stddef.h>
#include <util/arrays.h>
#include "stdio.h"

#define PIC_REMAP_OFFSET        0x20

// An array of IRQ handlers.
IRQHandler g_IRQHandlers[16];
// The PIC driver.
static const PICDriver* g_Driver = NULL;

/**
 * @brief The main IRQ handler.
 * 
 * @param regs The registers at the time of the interrupt.
 */
void i686_IRQ_Handler(Registers* regs)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    
    // If there is a registered handler for this IRQ, call it.
    if (g_IRQHandlers[irq] != NULL)
    {
        // handle IRQ
        g_IRQHandlers[irq](regs);
    }
    // If there is no registered handler, print a message.
    else
    {
        printf("Unhandled IRQ %d...\n", irq);
    }

    // Send the end of interrupt signal to the PIC.
    g_Driver->SendEndOfInterrupt(irq);
}

/**
 * @brief Initializes the IRQs.
 */
void i686_IRQ_Initialize()
{
    // An array of PIC drivers.
    const PICDriver* drivers[] = {
        i8259_GetDriver(),
    };

    // Probe for a PIC driver.
    for (int i = 0; i < SIZE(drivers); i++) {
        if (drivers[i]->Probe()) {
            g_Driver = drivers[i];
        }
    }

    // If no PIC driver was found, print a warning and return.
    if (g_Driver == NULL) {
        printf("Warning: No PIC found!\n");
        return;
    }

    // Initialize the PIC driver.
    printf("Found %s PIC.\n", g_Driver->Name);
    g_Driver->Initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    // Register the IRQ handlers.
    for (int i = 0; i < 16; i++)
        i686_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, i686_IRQ_Handler);

    // Enable interrupts.
    i686_EnableInterrupts();

    // Unmask the timer and keyboard interrupts (commented out).
    // g_Driver->Unmask(0);
    // g_Driver->Unmask(1);
}

/**
 * @brief Registers an IRQ handler.
 * 
 * @param irq The IRQ number.
 * @param handler The handler to register.
 */
void i686_IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}

/**
 * @brief Gets the PIC driver.
 * 
 * @return The PIC driver.
 */
const PICDriver* i686_IRQ_GetDriver()
{
    return g_Driver;
}
