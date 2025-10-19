/**
 * @file main.c
 * @brief This is the main entry point for the stage 2 bootloader.
 */

#include <stdint.h>
#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "string/string.h"
#include "mbr.h"
#include "stdlib.h"
#include "string.h"
#include "elf.h"
#include "memdetect.h"
#include <boot/bootparams.h>

// The buffer where the kernel is loaded.
uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL;
// The memory address where the kernel is located.
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

// The boot parameters that are passed to the kernel.
BootParams g_BootParams;

// A function pointer to the kernel's entry point.
typedef void (*KernelStart)(BootParams* bootParams);

/**
 * @brief The main logic of the bootloader.
 * 
 * This function initializes the disk and FAT file system, loads the kernel from the disk,
 * and then jumps to the kernel's entry point.
 * 
 * @param bootDrive The boot drive number.
 * @param partition A pointer to the partition information.
 * @return 0 on success, -1 on failure.
 */
int run_bootloader(uint16_t bootDrive, void* partition)
{
    // Initialize the disk.
    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive))
    {
        printf("Disk init error\r\n");
        return -1;
    }

    // Detect the partition.
    Partition part;
    MBR_DetectPartition(&part, &disk, partition);

    // Initialize the FAT file system.
    if (!FAT_Initialize(&part))
    {
        printf("FAT init error\r\n");
        return -1;
    }

    // Prepare the boot parameters.
    g_BootParams.BootDevice = bootDrive;
    Memory_Detect(&g_BootParams.Memory);

    // Load the kernel.
    KernelStart kernelEntry;
    if (!ELF_Read(&part, "/boot/kernel.elf", (void**)&kernelEntry))
    {
        printf("ELF read failed, booting halted!");
        return -1;
    }

    // Execute the kernel.
    kernelEntry(&g_BootParams);

    return 0;
}

/**
 * @brief The entry point of the stage 2 bootloader.
 * 
 * @param bootDrive The boot drive number.
 * @param partition A pointer to the partition information.
 */
void __attribute__((cdecl)) start(uint16_t bootDrive, void* partition)
{
    // Clear the screen.
    clrscr();

    // Run the bootloader.
    run_bootloader(bootDrive, partition);

    // Halt the CPU.
    for (;;);
}