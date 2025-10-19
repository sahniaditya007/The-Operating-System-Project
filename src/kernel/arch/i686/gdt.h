#pragma once

#include <stdint.h>

/**
 * @brief A GDT entry.
 */
typedef struct
{
    uint16_t LimitLow;                  // limit (bits 0-15)
    uint16_t BaseLow;                   // base (bits 0-15)
    uint8_t BaseMiddle;                 // base (bits 16-23)
    uint8_t Access;                     // access
    uint8_t FlagsLimitHi;               // limit (bits 16-19) | flags
    uint8_t BaseHigh;                   // base (bits 24-31)
} __attribute__((packed)) GDTEntry;

/**
 * @brief A GDT descriptor.
 */
typedef struct
{
    uint16_t Limit;                     // sizeof(gdt) - 1
    GDTEntry* Ptr;                      // address of GDT
} __attribute__((packed)) GDTDescriptor;

#define i686_GDT_CODE_SEGMENT 0x08
#define i686_GDT_DATA_SEGMENT 0x10

void i686_GDT_Initialize();