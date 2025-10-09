#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Define a simple boolean type for compatibility
typedef uint8_t bool;
#define true 1
#define false 0

// ============================
// FAT12 Boot Sector Structure
// ============================
// This structure represents the layout of the FAT12 boot sector.
// It contains filesystem metadata that describes how the disk is organized.
typedef struct 
{
    uint8_t BootJumpInstruction[3];   // Jump instruction to boot code
    uint8_t OemIdentifier[8];         // OEM identifier string
    uint16_t BytesPerSector;          // Number of bytes per sector
    uint8_t SectorsPerCluster;        // Number of sectors per cluster
    uint16_t ReservedSectors;         // Reserved sectors before FAT
    uint8_t FatCount;                 // Number of FAT copies
    uint16_t DirEntryCount;           // Number of root directory entries
    uint16_t TotalSectors;            // Total number of sectors in the volume
    uint8_t MediaDescriptorType;      // Media descriptor (e.g., 0xF0 for floppy)
    uint16_t SectorsPerFat;           // Number of sectors per FAT table
    uint16_t SectorsPerTrack;         // Sectors per track (for BIOS)
    uint16_t Heads;                   // Number of heads (for BIOS)
    uint32_t HiddenSectors;           // Hidden sectors before the partition
    uint32_t LargeSectorCount;        // Large total sector count (if > 65535)

    // Extended boot record fields
    uint8_t DriveNumber;              // Drive number (BIOS)
    uint8_t _Reserved;                // Reserved byte
    uint8_t Signature;                // Boot signature (usually 0x29)
    uint32_t VolumeId;                // Volume serial number
    uint8_t VolumeLabel[11];          // Volume label (padded with spaces)
    uint8_t SystemId[8];              // Filesystem type string (e.g., "FAT12")

} __attribute__((packed)) BootSector;


// ============================
// FAT12 Directory Entry Structure
// ============================
// Each entry represents a file or directory in the FAT root directory.
typedef struct 
{
    uint8_t Name[11];             // File name in 8.3 format (no dot)
    uint8_t Attributes;           // File attributes (e.g., read-only, hidden)
    uint8_t _Reserved;            // Reserved
    uint8_t CreatedTimeTenths;    // Tenths of seconds file was created
    uint16_t CreatedTime;         // Creation time
    uint16_t CreatedDate;         // Creation date
    uint16_t AccessedDate;        // Last accessed date
    uint16_t FirstClusterHigh;    // High 16 bits of first cluster (FAT32 only)
    uint16_t ModifiedTime;        // Last modified time
    uint16_t ModifiedDate;        // Last modified date
    uint16_t FirstClusterLow;     // Low 16 bits of first cluster
    uint32_t Size;                // File size in bytes
} __attribute__((packed)) DirectoryEntry;


// ============================
// Global Variables
// ============================
BootSector g_BootSector;           // Global boot sector structure
uint8_t* g_Fat = NULL;             // FAT table data
DirectoryEntry* g_RootDirectory = NULL;  // Root directory entries
uint32_t g_RootDirectoryEnd;       // LBA of end of root directory


// ============================
// Function: readBootSector
// ============================
// Reads the boot sector (first sector of the disk) into memory.
bool readBootSector(FILE* disk)
{
    return fread(&g_BootSector, sizeof(g_BootSector), 1, disk) > 0;
}


// ============================
// Function: readSectors
// ============================
// Reads one or more sectors from disk at the given LBA (Logical Block Address).
bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut)
{
    bool ok = true;
    ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(bufferOut, g_BootSector.BytesPerSector, count, disk) == count);
    return ok;
}


// ============================
// Function: readFat
// ============================
// Reads the FAT table from the disk into memory.
bool readFat(FILE* disk)
{
    g_Fat = (uint8_t*) malloc(g_BootSector.SectorsPerFat * g_BootSector.BytesPerSector);
    return readSectors(disk, g_BootSector.ReservedSectors, g_BootSector.SectorsPerFat, g_Fat);
}


// ============================
// Function: readRootDirectory
// ============================
// Reads the root directory entries into memory.
bool readRootDirectory(FILE* disk)
{
    // Calculate the starting LBA of the root directory
    uint32_t lba = g_BootSector.ReservedSectors + g_BootSector.SectorsPerFat * g_BootSector.FatCount;

    // Calculate total size of root directory
    uint32_t size = sizeof(DirectoryEntry) * g_BootSector.DirEntryCount;
    uint32_t sectors = (size / g_BootSector.BytesPerSector);
    if (size % g_BootSector.BytesPerSector > 0)
        sectors++;

    // Store the end LBA for later use (start of data area)
    g_RootDirectoryEnd = lba + sectors;

    // Allocate memory and read directory
    g_RootDirectory = (DirectoryEntry*) malloc(sectors * g_BootSector.BytesPerSector);
    return readSectors(disk, lba, sectors, g_RootDirectory);
}


// ============================
// Function: findFile
// ============================
// Searches the root directory for a file with a matching 8.3 name.
DirectoryEntry* findFile(const char* name)
{
    for (uint32_t i = 0; i < g_BootSector.DirEntryCount; i++)
    {
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0)
            return &g_RootDirectory[i];
    }

    return NULL; // File not found
}


// ============================
// Function: readFile
// ============================
// Reads a file’s data from the disk following its cluster chain.
bool readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* outputBuffer)
{
    bool ok = true;
    uint16_t currentCluster = fileEntry->FirstClusterLow;

    // Loop through FAT chain until end-of-file marker
    do {
        // Convert cluster number to LBA
        uint32_t lba = g_RootDirectoryEnd + (currentCluster - 2) * g_BootSector.SectorsPerCluster;

        // Read all sectors for this cluster
        ok = ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
        outputBuffer += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

        // Read the next cluster number from FAT (12-bit entries)
        uint32_t fatIndex = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0)
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
        else
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;

    } while (ok && currentCluster < 0x0FF8); // 0x0FF8–0x0FFF are end markers

    return ok;
}


// ============================
// Main Program
// ============================
// Usage: program <disk_image> <file_name>
int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    // Open the disk image file
    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Cannot open disk image %s!\n", argv[1]);
        return -1;
    }

    // Step 1: Read boot sector
    if (!readBootSector(disk)) {
        fprintf(stderr, "Could not read boot sector!\n");
        return -2;
    }

    // Step 2: Read FAT table
    if (!readFat(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(g_Fat);
        return -3;
    }

    // Step 3: Read root directory
    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read root directory!\n");
        free(g_Fat);
        free(g_RootDirectory);
        return -4;
    }

    // Step 4: Find file in root directory
    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Could not find file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        return -5;
    }

    // Step 5: Read file contents
    uint8_t* buffer = (uint8_t*) malloc(fileEntry->Size + g_BootSector.BytesPerSector);
    if (!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Could not read file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        free(buffer);
        return -6;
    }

    // Step 6: Print file contents (printable ASCII or hex)
    for (size_t i = 0; i < fileEntry->Size; i++)
    {
        if (isprint(buffer[i])) fputc(buffer[i], stdout);
        else printf("<%02x>", buffer[i]);
    }
    printf("\n");

    // Cleanup
    free(buffer);
    free(g_Fat);
    free(g_RootDirectory);
    return 0;
}
