# ===============================
# Compiler and Assembler Settings
# ===============================
# Define the tools used to build the project.
ASM = nasm                               # Assembler for assembly (.asm) files
CC = gcc                                 # C compiler for building tools
CC16 = /usr/bin/watcom/binl/wcc          # 16-bit C compiler (Watcom)
LD16 = /usr/bin/watcom/binl/wlink        # 16-bit linker (Watcom)

# ===============================
# Directory Paths
# ===============================
SRC_DIR = src                            # Source code directory
TOOLS_DIR = tools                        # Tools source directory
BUILD_DIR = build                        # Directory for build outputs

# ===============================
# Phony Targets
# ===============================
# Phony targets don’t correspond to actual files.
# They ensure that these rules always run when invoked.
.PHONY: all floppy_image kernel bootloader clean always tools_fat

# ===============================
# Default Build Target
# ===============================
# Running 'make' without arguments will build:
# - the floppy disk image
# - the FAT tool
all: floppy_image tools_fat


# ===============================
# FLOPPY IMAGE CREATION
# ===============================
# Target: floppy_image
# Creates a bootable floppy disk image with bootloader and kernel.
floppy_image: $(BUILD_DIR)/main_floppy.img

# Create the floppy disk image
$(BUILD_DIR)/main_floppy.img: bootloader kernel
	# 1. Create a blank 1.44MB floppy disk image (2880 sectors × 512 bytes)
	dd if=/dev/zero of=$(BUILD_DIR)/main_floppy.img bs=512 count=2880

	# 2. Format it as FAT12 with the label "NBOS"
	mkfs.fat -F 12 -n "NBOS" $(BUILD_DIR)/main_floppy.img

	# 3. Write stage1 bootloader (first sector, no truncation)
	dd if=$(BUILD_DIR)/stage1.bin of=$(BUILD_DIR)/main_floppy.img conv=notrunc

	# 4. Copy additional files into the image:
	#    - stage2 bootloader
	#    - kernel binary
	#    - test.txt file for testing
	mcopy -i $(BUILD_DIR)/main_floppy.img $(BUILD_DIR)/stage2.bin "::stage2.bin"
	mcopy -i $(BUILD_DIR)/main_floppy.img $(BUILD_DIR)/kernel.bin "::kernel.bin"
	mcopy -i $(BUILD_DIR)/main_floppy.img test.txt "::test.txt"


# ===============================
# BOOTLOADER BUILD
# ===============================
# Target: bootloader
# Builds both stage1 and stage2 parts of the bootloader.
bootloader: stage1 stage2

# --- Stage 1 ---
stage1: $(BUILD_DIR)/stage1.bin

# Build stage1 by invoking its own Makefile
$(BUILD_DIR)/stage1.bin: always
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

# --- Stage 2 ---
stage2: $(BUILD_DIR)/stage2.bin

# Build stage2 by invoking its own Makefile
$(BUILD_DIR)/stage2.bin: always
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))


# ===============================
# KERNEL BUILD
# ===============================
# Target: kernel
# Builds the operating system kernel binary.
kernel: $(BUILD_DIR)/kernel.bin

# Build kernel by calling its own Makefile
$(BUILD_DIR)/kernel.bin: always
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR))


# ===============================
# TOOLS BUILD (e.g., FAT parser)
# ===============================
# Target: tools_fat
# Builds the FAT file system tool used for testing or analysis.
tools_fat: $(BUILD_DIR)/tools/fat

$(BUILD_DIR)/tools/fat: always $(TOOLS_DIR)/fat/fat.c
	# Ensure tools directory exists
	mkdir -p $(BUILD_DIR)/tools
	# Compile the FAT tool with debugging info (-g)
	$(CC) -g -o $(BUILD_DIR)/tools/fat $(TOOLS_DIR)/fat/fat.c


# ===============================
# ALWAYS
# ===============================
# Ensures the build directory exists before building anything.
always:
	mkdir -p $(BUILD_DIR)


# ===============================
# CLEAN
# ===============================
# Cleans up all generated files and directories.
clean:
	# Run clean in all submodules (bootloader stages and kernel)
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean

	# Remove all built files
	rm -rf $(BUILD_DIR)/*
