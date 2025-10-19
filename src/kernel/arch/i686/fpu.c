#include "fpu.h"
#include <stdint.h>

void i686_FPU_Initialize() {
    __asm__ __volatile__("clts");
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2);    // Clear EM (Emulation) to enable FPU instructions
    cr0 |= (1 << 1);     // Set MP (Monitor Coprocessor)
    cr0 |= (1 << 5);     // Set NE (Numeric Error) to use #MF exceptions instead of IRQ13
    __asm__ __volatile__("mov %0, %%cr0" : : "r"(cr0));

    uint32_t cr4;
    __asm__ __volatile__("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 9);
    cr4 |= (1 << 10);
    __asm__ __volatile__("mov %0, %%cr4" : : "r"(cr4));

    __asm__ __volatile__("fninit");
}
