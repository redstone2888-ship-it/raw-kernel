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

        // List files in root directory (FAT12)
        else if (!strcmp(argv[0], "ls")) {
            int found = 0;
            for (int i = 0; i < 224; i++) {
                dir_entry_t* e = &root_dir[i];
                // skip empty, deleted, and volume label entries
                if ((unsigned char)e->name[0] == 0x00) break;
                if ((unsigned char)e->name[0] == 0xE5) continue;
                if (e->attr & 0x08) continue; // volume label
                if (e->attr & 0x10) continue; // directory (we're flat FAT12)

                // print name
                for (int j = 0; j < 8 && e->name[j] != ' '; j++)
                    putchar(e->name[j]);
                putchar('.');
                for (int j = 0; j < 3 && e->ext[j] != ' '; j++)
                    putchar(e->ext[j]);

                // print size, right-aligned in a column
                print_text("  ");
                char szbuf[12];
                itoa(e->size, szbuf);
                print_text(szbuf);
                print_text(" bytes\n");
                found++;
            }
            if (!found) print_text("(empty)\n");
        }

        // Print file contents (text files only)
        else if (!strcmp(argv[0], "cat")) {
            if (argc < 2) {
                print_text("Usage: cat NAME.EXT\n");
            } else {
                // split "NAME.EXT" into name[8] and ext[3], space-padded
                char name[9] = "        ";
                char ext[4]  = "   ";
                char* dot = 0;
                char* arg = argv[1];
                for (int i = 0; arg[i]; i++) {
                    if (arg[i] == '.') { dot = arg + i; break; }
                }
                int ni = 0;
                char* p = arg;
                while (*p && *p != '.' && ni < 8) name[ni++] = *p++;
                name[8] = 0;
                if (dot) {
                    dot++;
                    for (int i = 0; i < 3 && dot[i]; i++) ext[i] = dot[i];
                }
                ext[3] = 0;

                to_upper(name); to_upper(ext);
                dir_entry_t* entry = find_file(name, ext);
                if (!entry) {
                    print_text("cat: file not found\n");
                } else {
                    read_file(entry, memory_buffer, MAX_FILE_SIZE);
                    // print until size or null
                    for (uint32_t i = 0; i < entry->size && i < MAX_FILE_SIZE; i++) {
                        char c = (char)memory_buffer[i];
                        if (c == 0) break;
                        putchar(c);
                    }
                    putchar('\n');
                }
            }
        }

        // Write text to a file: write NAME.EXT some text here
        else if (!strcmp(argv[0], "write")) {
            if (argc < 3) {
                print_text("Usage: write NAME.EXT text...\n");
            } else {
                char name[9] = "        ";
                char ext[4]  = "   ";
                char* arg = argv[1];
                char* dot = 0;
                for (int i = 0; arg[i]; i++)
                    if (arg[i] == '.') { dot = arg + i; break; }
                int ni = 0;
                char* p = arg;
                while (*p && *p != '.' && ni < 8) name[ni++] = *p++;
                name[8] = 0;
                if (dot) {
                    dot++;
                    for (int i = 0; i < 3 && dot[i]; i++) ext[i] = dot[i];
                }
                ext[3] = 0;

                uint32_t pos = 0;
                for (int i = 2; i < argc && pos < MAX_FILE_SIZE - 2; i++) {
                    char* word = argv[i];
                    while (*word && pos < MAX_FILE_SIZE - 2)
                        memory_buffer[pos++] = *word++;
                    if (i + 1 < argc && pos < MAX_FILE_SIZE - 2)
                        memory_buffer[pos++] = ' ';
                }
                memory_buffer[pos++] = '\n';

                to_upper(name); to_upper(ext);
                dir_entry_t* entry = find_file(name, ext);
                if (!entry) entry = create_file(name, ext);
                if (!entry) {
                    print_text("write: root directory full\n");
                } else {
                    int r = write_file(entry, memory_buffer, pos);
                    if (r == 0)
                        print_text("OK\n");
                    else
                        print_text("write: disk full\n");
                }
            }
        }

        // Delete a file: rm NAME.EXT
        else if (!strcmp(argv[0], "rm")) {
            if (argc < 2) {
                print_text("Usage: rm NAME.EXT\n");
            } else {
                char name[9] = "        ";
                char ext[4]  = "   ";
                char* arg = argv[1];
                char* dot = 0;
                for (int i = 0; arg[i]; i++)
                    if (arg[i] == '.') { dot = arg + i; break; }
                int ni = 0;
                char* p = arg;
                while (*p && *p != '.' && ni < 8) name[ni++] = *p++;
                name[8] = 0;
                if (dot) {
                    dot++;
                    for (int i = 0; i < 3 && dot[i]; i++) ext[i] = dot[i];
                }
                ext[3] = 0;

                to_upper(name); to_upper(ext);
                dir_entry_t* entry = find_file(name, ext);
                if (!entry) {
                    print_text("rm: file not found\n");
                } else {
                    fat_free_chain(entry->first_cluster_lo);
                    entry->name[0] = (char)0xE5;
                    write_fat();
                    write_root_dir();
                    print_text("OK\n");
                }
            }
        }

                // Change text color: color FG BG (0-15 each)
        else if (!strcmp(argv[0], "color")) {
            if (argc < 3) {
                print_text("Usage: color FG BG  (0-15 each)\n");
                print_text("  0=black 1=blue 2=green 3=cyan\n");
                print_text("  4=red 5=magenta 6=brown 7=lgray\n");
                print_text("  8=dgray 9=lblue 10=lgreen 11=lcyan\n");
                print_text("  12=lred 13=lmag 14=yellow 15=white\n");
            } else {
                int fg = atoi(argv[1]);
                int bg = atoi(argv[2]);
                if (fg < 0 || fg > 15 || bg < 0 || bg > 15) {
                    print_text("Error: values must be 0-15\n");
                } else {
                    color = (unsigned char)((bg << 4) | fg);
                    print_text("Color changed.\n");
                }
            }
        }

        // Help
        else if (!strcmp(argv[0], "help")) {
            print_text("Commands:\n");
            print_text("  ls            - list files on disk\n");
            print_text("  cat NAME.EXT  - print file contents\n");
            print_text("  write NAME.EXT text - write text to file\n");
            print_text("  rm NAME.EXT   - delete a file\n");
            print_text("  color FG BG   - change text color (0-15)\n");
            print_text("  ps            - list tasks\n");
            print_text("  kill PID      - kill a task\n");
            print_text("  clear         - clear screen\n");
            print_text("  reboot        - reboot system\n");
            print_text("  shutdown      - power off\n");
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
            println("Unknown command: %s", argv[0]);
        }
    }
}