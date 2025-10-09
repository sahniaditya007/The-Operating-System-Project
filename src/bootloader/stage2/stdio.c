#include "stdio.h"   // Provides declarations for standard I/O functions
#include "x86.h"     // Provides low-level x86 hardware interaction functions (like video output)


// ===============================================
// Function: putc
// -----------------------------------------------
// Writes a single character to the screen using
// BIOS teletype mode (int 0x10, AH=0x0E).
// Parameters:
//   c - the character to print
// ===============================================
void putc(char c)
{
    // Call low-level x86 function to print a character
    // 0 = page number (video page 0)
    x86_Video_WriteCharTeletype(c, 0);
}


// ===============================================
// Function: puts
// -----------------------------------------------
// Prints a null-terminated string to the screen
// by repeatedly calling putc() for each character.
// Parameters:
//   str - pointer to a C string (null-terminated)
// ===============================================
void puts(const char* str)
{
    // Loop through each character until null terminator '\0'
    while (*str)
    {
        putc(*str);   // Print one character
        str++;        // Move to the next character
    }
}
