/**
 * @file isr.c
 * @brief This file contains the implementation of the Interrupt Service Routines (ISRs).
 */

#include "isr.h"
#include "idt.h"
#include "gdt.h"
#include "io.h"
#include <stdio.h>
#include <stddef.h>

// An array of ISR handlers.
ISRHandler g_ISRHandlers[256];

// An array of exception messages.
static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

// External function to initialize the ISR gates.
void i686_ISR_InitializeGates();

/**
 * @brief Initializes the ISRs.
 */
void i686_ISR_Initialize()
{
    // Initialize the ISR gates.
    i686_ISR_InitializeGates();

    // Enable all IDT gates.
    for (int i = 0; i < 256; i++)
        i686_IDT_EnableGate(i);

    // Disable the syscall gate.
    i686_IDT_DisableGate(0x80);
}

/**
 * @brief The main ISR handler.
 * 
 * @param regs The registers at the time of the interrupt.
 */
void __attribute__((cdecl)) i686_ISR_Handler(Registers* regs)
{
    // If there is a registered handler for this interrupt, call it.
    if (g_ISRHandlers[regs->interrupt] != NULL)
        g_ISRHandlers[regs->interrupt](regs);

    // If the interrupt is a hardware interrupt, print a message.
    else if (regs->interrupt >= 32)
        printf("Unhandled interrupt %d!\n", regs->interrupt);

    // If the interrupt is an exception, print a message and panic.
    else 
    {
        printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        
        printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        printf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);

        printf("KERNEL PANIC!\n");
        i686_Panic();
    }
}

/**
 * @brief Registers an ISR handler.
 * 
 * @param interrupt The interrupt number.
 * @param handler The handler to register.
 */
void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler)
{
    g_ISRHandlers[interrupt] = handler;
    i686_IDT_EnableGate(interrupt);
}