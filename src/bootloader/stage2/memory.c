#include "memory.h"


void* segoffset_to_linear(void* addr)
{
    uint32_t offset = (uint32_t)(addr) & 0xFFFF;
    uint32_t segment = (uint32_t)(addr) >> 16;
    return (void*)(segment * 16 + offset);
}