/**
 * @file hal.c
 * @brief This file contains the implementation of the Hardware Abstraction Layer (HAL).
 */

#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/vga_text.h>

/**
 * @brief Initializes the Hardware Abstraction Layer (HAL).
 * 
 * This function initializes the GDT, IDT, ISRs, and IRQs.
 */
void HAL_Initialize()
{
    // Initialize the Global Descriptor Table (GDT).
    i686_GDT_Initialize();

    // Initialize the Interrupt Descriptor Table (IDT).
    i686_IDT_Initialize();

    // Initialize the Interrupt Service Routines (ISRs).
    i686_ISR_Initialize();

    // Initialize the Interrupt ReQuest handlers (IRQs).
    i686_IRQ_Initialize();
}