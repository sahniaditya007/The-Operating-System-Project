#pragma once

#include <stdint.h>
#include "gdt.h"
#include "idt.h"

void __attribute__((cdecl)) i686_GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);
void __attribute__((cdecl)) i686_IDT_Load(IDTDescriptor* idtDescriptor);
