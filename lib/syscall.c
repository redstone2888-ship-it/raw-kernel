#include <asm.h>
#include <vga.h>
#include <syscall.h>
#include <panic.h>

void kraw_syscall(int cmd) {
    switch (cmd) {
        case KRAW_CMD_RESTART:
            outb(0x64, 0xFE);
            while(1) hlt();
            break;
        case KRAW_CMD_POWER_OFF:
            outb(0x604, 0x2000);
            panic("SHUTDOWN_NOT_SUCCEFUL");
            break;
        default:
            println("syscall: unknown syscall");
            break;
    }
}