/**
 * @file idt.c
 * @brief This file contains the implementation of the Interrupt Descriptor Table (IDT).
 */

#include "idt.h"

void __attribute__((cdecl)) i686_IDT_Load(IDTDescriptor* idtDescriptor);
#include "gdt_idt_asm.h"
#include <stdint.h>
#include <util/binary.h>




// The Interrupt Descriptor Table.
IDTEntry g_IDT[256];

// The IDT descriptor.
IDTDescriptor g_IDTDescriptor = { sizeof(g_IDT) - 1, g_IDT };

// External function to load the IDT.


/**
 * @brief Sets an IDT gate.
 * 
 * @param interrupt The interrupt number.
 * @param base The base address of the interrupt handler.
 * @param segmentDescriptor The segment descriptor.
 * @param flags The flags.
 */
void i686_IDT_SetGate(int interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags)
{
    g_IDT[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    g_IDT[interrupt].SegmentSelector = segmentDescriptor;
    g_IDT[interrupt].Reserved = 0;
    g_IDT[interrupt].Flags = flags;
    g_IDT[interrupt].BaseHigh = ((uint32_t)base >> 16) & 0xFFFF;
}

/**
 * @brief Enables an IDT gate.
 * 
 * @param interrupt The interrupt number.
 */
void i686_IDT_EnableGate(int interrupt)
{
    FLAG_SET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

/**
 * @brief Disables an IDT gate.
 * 
 * @param interrupt The interrupt number.
 */
void i686_IDT_DisableGate(int interrupt)
{
    FLAG_UNSET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

/**
 * @brief Initializes the IDT.
 */
void i686_IDT_Initialize()
{
    // Load the IDT.
    i686_IDT_Load(&g_IDTDescriptor);
}