#include <vga.h>
#include <unistd.h>
#include <asm.h>
#include <syscall.h>

void kernel_panic(const char *code, const char *err)
{
    cli();

    clear_screen();
    println("\n :(\n");
    println(" Your PC ran into a problem and needs to restart.\n To restart / shutdown, hold your PC's power button\n");
    print_text(" Error code: ");
    println(code);
    print_text(" Error description: ");
    println(err);

    print_text("\n\n Restarting in ");
    for (int i = 5; i > 0; i--) {
        printf_v("%v...", i);
        sleep(1);
    }

    kraw_syscall(KRAW_CMD_RESTART);
}