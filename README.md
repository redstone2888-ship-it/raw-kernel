# Raw-Kernel

Experimental **operating system kernel** written in C/ASM for learning purposes.

# Overview

Raw-Kernel is a minimal kernel, designed to understand how low-level OS components work: booting, memory management, interrupts, and basic drivers.  
This project is purely educational — it's not meant for production use.

# Features

- Boot via GRUB
- Basic task scheduling
- Interrupt handling
- VGA text output
- FAT12 read/write support (via ATA PIO driver)
- Built-in shell with `ls`, `cat`, `write`, `rm`, `color`, `ps`, `kill`

# Build Instructions

1. Clone the repository:

```bash
git clone https://github.com/redstone2888-ship-it/raw-kernel.git
cd raw-kernel
```

2. Create a blank FAT12 disk image (only needed once):
```bash
make disk.img
```

3. Build the kernel:
```bash
make
```

4. Run in QEMU:
```bash
make run
```

> **Note:** Do not run the kernel directly with `-kernel` — it must be loaded via GRUB.

# Disk Image

`disk.img` is a 1.44MB FAT12 floppy image connected to QEMU as a second IDE drive. The kernel reads and writes it directly via ATA PIO — changes made inside the shell persist on disk.

## Managing disk.img from your host (macOS / Linux)

Use the included `rawdisk.py` utility — no dependencies, pure Python:

```bash
# List files
python3 rawdisk.py disk.img ls

# Show free space
python3 rawdisk.py disk.img info

# Inject a file from your host into the image
python3 rawdisk.py disk.img put notes.txt

# Extract a file from the image to your host
python3 rawdisk.py disk.img get NOTES.TXT

# Print file contents
python3 rawdisk.py disk.img cat NOTES.TXT

# Delete a file
python3 rawdisk.py disk.img rm NOTES.TXT
```

This means you can write files inside the kernel, then pull them out on your Mac — or inject files from your Mac and read them inside the kernel.

# Shell Commands

| Command | Description |
|---|---|
| `ls` | List files on disk |
| `cat NAME.EXT` | Print file contents |
| `write NAME.EXT text` | Write text to a file |
| `rm NAME.EXT` | Delete a file |
| `color FG BG` | Change text color (0–15) |
| `ps` | List running tasks |
| `kill PID` | Kill a task |
| `clear` | Clear screen |
| `reboot` | Reboot system |
| `shutdown` | Power off |
| `help` | Show all commands |

# Repository Structure

`boot/` - GRUB bootloader entry point.  
`drivers/` - VGA, keyboard, and ATA PIO drivers.  
`fs/` - FAT12 filesystem driver.  
`include/` - Header files. **Tip:** use `#include <filename>`.  
`kernel/` - Main kernel, shell, scheduler, panic handler.  
`lib/` - String utils, syscalls.  
`rawdisk.py` - Host-side disk utility for managing `disk.img`.  
`linker.ld` - **Do not edit** — required for proper `.iso` build.  
`Makefile` - Build system.  

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
