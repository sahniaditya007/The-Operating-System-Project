# ===============================================
# Build Configuration for Stage 2 Bootloader
# ===============================================

# -----------------------------------------------
# Default build directories and tools
# -----------------------------------------------
BUILD_DIR ?= build/                       # Default directory for build output files

ASM ?= nasm                              # Assembler used for .asm files
ASMFLAGS ?= -f obj                       # Output format: 16-bit .obj (for Watcom linker)

CC16 ?= /usr/bin/watcom/binl/wcc          # Watcom 16-bit C compiler path
CFLAGS16 ?= -4 -d3 -s -wx -ms -zl -zq     # Watcom compiler flags:
#  -4    : Generate 16-bit 286 code
#  -d3   : Debug info level 3
#  -s    : Disable stack overflow checks
#  -wx   : Treat warnings as errors
#  -ms   : Small memory model (code+data < 64KB)
#  -zl   : Remove default library search
#  -zq   : Quiet mode (minimal console output)
#  (commented option -oneatxzh can control optimization/linkage behavior)

LD16 ?= /usr/bin/watcom/binl/wlink        # Watcom 16-bit linker path


# -----------------------------------------------
# Source and object file definitions
# -----------------------------------------------
SOURCES_C = $(wildcard *.c)               # All C source files in current directory
SOURCES_ASM = $(wildcard *.asm)           # All assembly source files in current directory

# Generate object file paths for both C and ASM
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/stage2/c/%.obj, $(SOURCES_C))
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/stage2/asm/%.obj, $(SOURCES_ASM))


# -----------------------------------------------
# Phony targets (not actual files)
# -----------------------------------------------
.PHONY: all stage2 clean always


# ===============================================
# Default Target
# ===============================================
# Running 'make' will build the Stage 2 binary.
all: stage2


# ===============================================
# Stage 2 Build Target
# ===============================================
# Links all C and ASM object files into stage2.bin
stage2: $(BUILD_DIR)/stage2.bin

# Link all compiled object files using Watcom linker
$(BUILD_DIR)/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	$(LD16) NAME $(BUILD_DIR)/stage2.bin FILE { $(OBJECTS_ASM) $(OBJECTS_C) } \
	OPTION MAP=$(BUILD_DIR)/stage2.map @linker.lnk
	# NAME: output binary name
	# FILE: object files to include
	# OPTION MAP: generate a .map file for debugging
	# @linker.lnk: extra linker options file


# ===============================================
# Compile C Sources
# ===============================================
# Rule to compile each .c file into an .obj file
$(BUILD_DIR)/stage2/c/%.obj: %.c always
	$(CC16) $(CFLAGS16) -fo=$@ $<
	# -fo=$@ : output file name
	# $<     : input source file


# ===============================================
# Assemble ASM Sources
# ===============================================
# Rule to assemble each .asm file into an .obj file
$(BUILD_DIR)/stage2/asm/%.obj: %.asm always
	$(ASM) $(ASMFLAGS) -o $@ $<
	# -o $@ : output file name
	# $<    : input source file


# ===============================================
# Always
# ===============================================
# Ensures build directories exist before compiling
always:
	mkdir -p $(BUILD_DIR)/stage2/c
	mkdir -p $(BUILD_DIR)/stage2/asm


# ===============================================
# Clean
# ===============================================
# Removes all generated build files for Stage 2
clean:
	rm -f $(BUILD_DIR)/stage2.bin
