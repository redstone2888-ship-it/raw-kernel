/*
 * panic.c - kernel error output
 * 
 * (c) 2026, Redstone2888
 * Read LICENSE.txt for details
*/

#include <vga.h>
#include <unistd.h>
#include <asm.h>
#include <syscall.h>

void panic(const char *err) {
    cli();
    print_text("RAW-KERNEL\n");
    print_text("---------- KERNEL PANIC! ----------\n");
    print_text("Error issue: ");
    print_text(err);
    print_text("\nTo restart, hold PC's power button\n");
    print_text("-----------------------------------\n");

    while (1) {
        hlt();
    }
}