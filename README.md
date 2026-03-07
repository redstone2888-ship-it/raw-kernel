# Raw-Kernel

Experimental **operating system kernel** written in C/ASM for learning purposes.

# Overview

Raw-Kernel is a minimal kernel, designed to understand how low-level OS components work: booting, memory management, interrupts, and basic drivers.  
This project is purely educational — it’s not meant for production use.

# Features

- Boot via GRUB
- Basic task scheduling
- Interrupt handling
- VGA text output
- FAT12 reading support

# Build Instructions

1. Clone the repository:

```bash
git clone https://github.com/redstone2888-ship-it/raw-kernel.git
cd raw-kernel
```

2. Build the kernel:
```bash
make
```

3. Run:
**Important:** Do not run kernel directly with `-kernel`, it must loaded via GRUB.  
```bash
make run
```

# **NEW!** Building a disk.img

**WARNING!** The kernel requires `disk.img`; without it, it will return the `NO_MODULES_FOUND` error. However, you can comment out lines 48-50 to ignore this error.  

You can create one like this:
```bash
# Create a empty image:
dd if=/dev/zero of=disk.img bs=512 count=2880

# Format it as FAT12
mkfs.fat -F 12 disk.img

# Optional: mount it and add files
mkdir /tmp/build
sudo mount -o loop disk.img /tmp/build
cp your_files /tmp/build
sudo umount /tmp/build
```

# Repository Structure

`boot/` - To boot correctly via GRUB.  
`drivers/` - Drivers for screen and keyboard.  
`fs/` - File system drivers.  
`include/` - Include lib files. **Tip:** use `#include <{FILE_IN_INCLUDE_FOLDER}>`.  
`kernel/` - Main kernel files.  
`lib/` - Other lib files.  
`disk.img` - Optional, you can build it yourself.  
`linker.ld` - **Do not edit**, this is for proper `.iso` build.  
`Makefile` - For building.  

# Contributing

**Want to improve `raw-kernel`?**
1. Fork the repository  
2. Create a branch `feature/your-feature`  
3. Commit and push your changes  
4. Open a pull request describing your changes  

# Issues / Questions

Use the **Issues** tab on **GitHub** to report bugs or suggest improvements.  

# License

This project is licensed under the MIT License. See the `LICENSE.txt` file for details.  