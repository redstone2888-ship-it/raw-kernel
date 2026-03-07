/*
 * shed.c — Simple task scheduler for
 *
 * (c) 2026, Redstone2888
 * Read LICENSE.txt for details
 *
 * Handles:
 * - Task management
 * - Task switching
 * - Adding/killing tasks
 */

#include <shed.h>
#include <asm.h>
#include <shell.h>
#include <FAT12.h>

task_t tasks[MAX_TASKS];
int current_task = 0;

// Initialize scheduler and start the shell
void shed_init() {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].pid = i;
        tasks[i].state = TASK_DEAD;
        tasks[i].entry = 0;
        tasks[i].stack = 0;
        tasks[i].program = 0;
        tasks[i].size = 0;
    }

    while (1) {
        if (tasks[0].state == TASK_DEAD) {
            shed_add(shell, "SHELL");
            shed_tick();
        }
    }
}

// Add a function as a new task
int shed_add(void (*fn)(), const char* name) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_DEAD) {
            tasks[i].entry = fn;
            tasks[i].state = TASK_READY;
            tasks[i].program = 0;
            tasks[i].size = 0;
            if (name) {
                for (int j = 0; j < 16 && name[j]; j++)
                    tasks[i].name[j] = name[j];
            }
            return tasks[i].pid;
        }
    }
    return -1;
}

// Kill a task by PID
void shed_kill(int pid) {
    if (pid < 0 || pid >= MAX_TASKS) return;
    tasks[pid].state = TASK_DEAD;
}

// Switch to the next READY task
void shed_yield() {
    int start = current_task;
    do {
        current_task = (current_task + 1) % MAX_TASKS;
        if (tasks[current_task].state == TASK_READY) {
            tasks[current_task].state = TASK_RUNNING;
            tasks[start].state = TASK_READY;

            if (tasks[current_task].entry)
                tasks[current_task].entry();
            else if (tasks[current_task].program)
                run_program(tasks[current_task].program, tasks[current_task].size);

            return;
        }
    } while (current_task != start);
    hlt(); // halt CPU if no ready tasks
}

// Called by shed_init or task loops to switch tasks
void shed_tick() {
    shed_yield();
}

// Convert task state enum to string
const char* state_to_string(task_state_t state) {
    switch(state) {
        case TASK_DEAD: return "DEAD";
        case TASK_READY: return "READY";
        case TASK_RUNNING: return "RUNNING";
        case TASK_SLEEP: return "SLEEP";
        default: return "???";
    }
}