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
#include <FAT12.h>
#include <programs.h>
#include <FAT12.h>

typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    char* cmdline;
} multiboot_module_t;

typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
} multiboot_info_t;

void print_fat_name(char* name, char* ext) {
    println("Filename:");
    for (int i = 0; i < 8 && name[i] != ' '; i++) putchar(name[i]);
    putchar('.');
    for (int i = 0; i < 3 && ext[i] != ' '; i++) putchar(ext[i]);
    print_text("\n\n");
}

void kernel_main(unsigned long magic, unsigned long addr) {
    multiboot_info_t* mb_info = (multiboot_info_t*)addr;

    clear_screen();

    if (!(magic == 0x2BADB002)) {
        panic("LOADED_NOT_BY_GRUB");
    }

    if (mb_info->mods_count == 0) {
        panic("NO_MODULES_FOUND");
    }

    multiboot_module_t* modules = (multiboot_module_t*)(mb_info->mods_addr);
    disk_image = (uint8_t*)modules[0].mod_start;

    read_fat();
    read_root_dir();

    print_fat_name("KERNEL", "BIN");

    print_text("Kernel started with no problems\nNo discrete shell found, launching built-in shell\n");

    shed_init();
}