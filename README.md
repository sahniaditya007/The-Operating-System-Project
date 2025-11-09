# The Operating System Project

A 32-bit operating system for the i686 architecture, built from scratch. This project includes a bootloader, a kernel with a simple GUI, and various drivers.

## Screenshots

Here are some screenshots of the OS in action:

![Screenshot 1](output%20images/img1.png)
![Screenshot 2](output%20images/img2.png)
![Screenshot 3](output%20images/img3.png)
![Screenshot 4](output%20images/img4.png)

## Features

*   **i686 Architecture:** Designed for the 32-bit Intel architecture.
*   **Custom Bootloader:** A two-stage bootloader to initialize the system.
*   **FAT Filesystem:** Support for reading from FAT12/FAT16/FAT32 filesystems.
*   **Graphical User Interface:** A basic GUI with windowing support.
*   **PS/2 Mouse and Keyboard:** Drivers for mouse and keyboard input.
*   **Memory Management:** Includes memory detection and management features.
*   **Build System:** Uses SCons for a flexible and powerful build process.

## Building and Running

### Dependencies

First, install the necessary dependencies for your distribution:

**Ubuntu/Debian:**
```sh
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget \
                   nasm mtools python3 python3-pip python3-parted scons dosfstools libguestfs-tools qemu-system-x86
```

**Fedora:**
```sh
sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo wget \
                   nasm mtools python3 python3-pip python3-pyparted python3-scons dosfstools guestfs-tools qemu-system-x86
```

**Arch Linux:**
```sh
paru -S gcc make bison flex libgmp-static libmpc mpfr texinfo nasm mtools qemu-system-x86 python3 scons
```
*(An AUR helper like `paru` or `yay` is required)*

After installing the packages, install the Python dependencies:
```sh
python3 -m pip install -r requirements.txt
```

### Toolchain Setup

Next, you need to set up the cross-compiler toolchain. The build script will download and build `binutils` and `gcc` for the i686-elf target.

Run the following command to set up the toolchain. You can customize the installation directory in `build_scripts/config.py`.
```sh
scons toolchain
```

### Building the OS

Once the toolchain is set up, you can build the operating system:
```sh
scons
```
This will create a disk image in the `build/i686_debug` directory.

### Running the OS

You can run the OS in QEMU using the following command:
```sh
scons run
```

To run with debugging support (GDB):
```sh
scons debug
```

You can also run the OS in Bochs:
```sh
scons bochs
```

## Project Structure

*   `src/bootloader`: The stage 1 and stage 2 bootloader.
*   `src/kernel`: The main kernel code, including drivers and the GUI.
*   `src/libs`: Libraries used by the kernel.
*   `image`: Scripts for creating the disk image.
*   `scripts`: Helper scripts for running and debugging the OS.
*   `tools`: Host-side tools for working with the filesystem.
*   `SConstruct`: The main SCons build script.

## License

This project is licensed under the MIT License.