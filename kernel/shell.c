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

// ---------- GUI задача ----------
void test_ui_task() {
    uint8_t bg = make_color(COLOR_WHITE, COLOR_BLUE);

    clear_screen();
    draw_clear(make_color(COLOR_WHITE, COLOR_BLACK));
    draw_rect(0, 0, 80, 25, bg);
    draw_border(0, 0, 80, 25, make_color(COLOR_YELLOW, COLOR_BLUE));
    draw_text_center_at("RAWOS", COLOR_WHITE, COLOR_BLUE, VGA_HEIGHT / 2);
    sleep(3);
    draw_text_center_at("ITS REALLY GOOD", COLOR_WHITE, COLOR_BLUE, VGA_HEIGHT / 2 + 1);
    sleep(2);

    clear_screen();

    bg = make_color(COLOR_WHITE, COLOR_CYAN);
    draw_rect(0, 0, 80, 25, bg);
    draw_border(0, 0, 80, 25, make_color(COLOR_WHITE, COLOR_CYAN));
    draw_text_center("THE BEST OS IN WORLD", COLOR_WHITE, COLOR_CYAN);
    sleep(2);

    char str[] = "maybe...";
    for (int i = 0; str[i]; i++) {
        draw_char(VGA_WIDTH / 2 - 4 + i, VGA_HEIGHT / 2 + 1, str[i], COLOR_WHITE, COLOR_CYAN);
        usleep(500000);
    }

    int i = 0;
    while(i < 300) {
        draw_clear(make_color(COLOR_WHITE, COLOR_RED));
        usleep(10000);
        draw_clear(make_color(COLOR_WHITE, COLOR_BLUE));
        usleep(10000);
        i++;
    }

    sleep(2);
    kernel_panic("0xTROLL", "HEHE_TROLLED");
    while (1) hlt();
}

// ---------- Shell ----------
void shell() {
    char line[128];

    while (1) {
        print_text("> ");
        readline(line, sizeof(line));

        char* argv[8];
        int argc = split(line, argv);
        if (argc == 0) continue;

        // ----- Основные команды -----
        if (!strcmp(argv[0], "clear")) {
            clear_screen();
        }
        else if (!strcmp(argv[0], "ls")) {
            for (int i = 0; i < 224; i++) {
                if (root_dir[i].name[0] != 0) { // непустая запись
                    println("%c%c%c%c%c%c%c%c.%c%c%c",
                        root_dir[i].name[0], root_dir[i].name[1],
                        root_dir[i].name[2], root_dir[i].name[3],
                        root_dir[i].name[4], root_dir[i].name[5],
                        root_dir[i].name[6], root_dir[i].name[7],
                        root_dir[i].ext[0], root_dir[i].ext[1],
                        root_dir[i].ext[2]);
                }
            }
        }
        else if (!strcmp(argv[0], "cat") && argc > 1) {
            dir_entry_t* entry = find_file(argv[1]);
            if (entry) {
                read_file(entry, memory_buffer);
                println("%s", memory_buffer);
            } else {
                println("File not found");
            }
        }

        // ----- GUI -----
        else if (!strcmp(argv[0], "gui")) {
            shed_add(gui, "gui");
            shed_tick();
        }

        // ----- FAT12 запуск бинарника -----
        else if (!strcmp(argv[0], "run") && argc > 1) {
            dir_entry_t* entry = find_file(argv[1]);
            if (entry) {
                read_file(entry, memory_buffer);
                shed_add_binary(argv[1], memory_buffer, entry->size);
            } else {
                println("File not found");
            }
        }

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

        // ----- Список процессов -----
        else if (!strcmp(argv[0], "ps")) {
            println("PID  NAME  STATE");
            for (int i = 0; i < MAX_TASKS; i++) {
                if (tasks[i].state != TASK_DEAD) {
                    println(" %d   %s   %s", tasks[i].pid, tasks[i].name, state_to_string(tasks[i].state));
                } else {
                    continue;
                }
            }
        }

        // ----- Завершение ядра -----
        else if (!strcmp(argv[0], "reboot")) {
            kraw_syscall(KRAW_CMD_RESTART);
        }
        else if (!strcmp(argv[0], "shutdown")) {
            kraw_syscall(KRAW_CMD_POWER_OFF);
        }

        else {
            println("Unknown command");
        }
    }
}