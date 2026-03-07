/*
 * kernel.c — Main kernel entry
 *
 * (c) 2026, Redstone2888
 * Read LICENSE.txt for details
 *
 * Handles:
 * - Entry point called by GRUB
 * - FAT12 initialization
 * - RAMFS / scheduler setup
 * - Panic checks for boot/loading (you can delete this)
 */

#include <stdint.h>
#include <stddef.h>
#include <vga.h>
#include <panic.h>
#include <asm.h>
#include <keyboard.h>
#include <string.h>
#include <shell.h>
#include <ramfs.h>
#include <shed.h>
#include <programs.h>
#include <FAT12.h>

// Multiboot module structure
typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    char* cmdline;
} multiboot_module_t;

// Multiboot info structure (from GRUB)
typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
} multiboot_info_t;

// Utility: print FAT12 file name in "NAME.EXT" format
void print_fat_name(char* name, char* ext) {
    println("Filename:");
    for (int i = 0; i < 8 && name[i] != ' '; i++) putchar(name[i]);
    putchar('.');
    for (int i = 0; i < 3 && ext[i] != ' '; i++) putchar(ext[i]);
    print_text("\n\n");
}

// Main kernel entry point (called by GRUB)
void kernel_main(unsigned long magic, unsigned long addr) {
    multiboot_info_t* mb_info = (multiboot_info_t*)addr;

    clear_screen(); // start with a clean screen

    // Check if booted by GRUB
    if (!(magic == 0x2BADB002)) {
        panic("LOADED_NOT_BY_GRUB");
    }

    // Ensure at least one module is loaded (e.g., disk image)
    if (mb_info->mods_count == 0) {
        panic("NO_MODULES_FOUND");
    }

    // Get pointer to the first multiboot module (disk image)
    multiboot_module_t* modules = (multiboot_module_t*)(mb_info->mods_addr);
    disk_image = (uint8_t*)modules[0].mod_start;

    // Initialize filesystem
    read_fat();
    read_root_dir();

    // Initialize scheduler and start kernel tasks
    shed_init();
}