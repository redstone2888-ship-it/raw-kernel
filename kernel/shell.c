/*
 * shell.c — the built-in shell
 *
 * (c) 2026, Redstone2888
 * Read LICENSE.txt for details
 */

#include <vga.h>
#include <panic.h>
#include <keyboard.h>
#include <string.h>
#include <syscall.h>
#include <draw.h>
#include <asm.h>
#include <unistd.h>
#include <FAT12.h>
#include <shed.h>
#include <programs.h>

void shell() {
    char line[128];

    while (1) {
        print_text("> ");
        readline(line, sizeof(line));

        char* argv[8];
        int argc = split(line, argv);
        if (argc == 0) continue;

        // Clear screen command
        if (!strcmp(argv[0], "clear")) {
            clear_screen();
        }

        // Kill a task by PID
        else if (!strcmp(argv[0], "kill")) {
            if (argc < 2) {
                print_text("Usage: kill [PID]\n");
            } else {
                int pid = atoi(argv[1]);
                if (pid < 0 || pid >= MAX_TASKS) {
                    print_text("Error: invalid PID\n");
                } else if (tasks[pid].state == TASK_DEAD) {
                    print_text("Error: task already dead\n");
                } else {
                    shed_kill(pid);
                }
            }
        }

        // List all tasks
        else if (!strcmp(argv[0], "ps")) {
            println("PID  NAME  STATE");
            for (int i = 0; i < MAX_TASKS; i++) {
                if (tasks[i].state != TASK_DEAD) {
                    println(" %d   %s   %s", tasks[i].pid, tasks[i].name, state_to_string(tasks[i].state));
                }
            }
        }

        // Reboot the system
        else if (!strcmp(argv[0], "reboot")) {
            kraw_syscall(KRAW_CMD_RESTART);
        }

        // Shutdown the system
        else if (!strcmp(argv[0], "shutdown")) {
            kraw_syscall(KRAW_CMD_POWER_OFF);
        }

        else {
            println("Unknown command");
        }
    }
}