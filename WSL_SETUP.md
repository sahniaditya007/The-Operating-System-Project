# WSL Setup Instructions

This guide will help you set up and run the nanobyte_os project in WSL (Windows Subsystem for Linux).

## Prerequisites

1. Install WSL2 with Ubuntu:
```bash
wsl --install -d Ubuntu
```

2. Install required dependencies in WSL:
```bash
sudo apt update
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo wget \
                   nasm mtools python3 python3-pip python3-parted scons dosfstools libguestfs-tools qemu-system-x86
```

3. Install Python requirements:
```bash
python3 -m pip install -r requirements.txt
```

## Building the Project

1. Create the toolchain directory:
```bash
mkdir -p cross-toolchain
```

2. Build the toolchain:
```bash
scons toolchain
```

3. Build the OS:
```bash
scons
```

4. Run the OS in QEMU:
```bash
scons run
```

## Notes for WSL

- The project has been fixed to work properly in WSL environment
- Shell script syntax errors have been corrected
- Python package dependencies have been cleaned up
- Toolchain paths are now properly configured for WSL

## Troubleshooting

If you encounter issues:
1. Make sure all dependencies are installed
2. Ensure the cross-toolchain directory exists and has proper permissions
3. Check that QEMU is properly installed and accessible
4. For GUI applications like QEMU, you may need to install an X server or use WSLg