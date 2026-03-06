// hey grub, its me, your kernel, please load me
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,             /* Magic number */
    0x00000003,             /* Flags */
    -(0x1BADB002 + 0x00000003) /* Checksum */
};

// на будущее, может что то будет
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


void kernel_main(void) {
    clear_screen(); // очищаю экран от мусора
    shed_init(); // инициализирую процессы
    shed_add(shell, "SHELL"); // создаю процесс консоли

    // ВСЁ
}
