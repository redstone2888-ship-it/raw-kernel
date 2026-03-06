#include <shed.h>
#include <asm.h>
#include <shell.h>
#include <FAT12.h>  // для memory_buffer

task_t tasks[MAX_TASKS];
int current_task = 0;

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

// добавить функцию как задачу
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

// добавить бинарник из FAT12
int shed_add_binary(const char* name, uint8_t* buf, size_t size) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_DEAD) {
            tasks[i].entry = 0;
            tasks[i].state = TASK_READY;
            tasks[i].program = buf;
            tasks[i].size = size;
            if (name) {
                for (int j = 0; j < 16 && name[j]; j++)
                    tasks[i].name[j] = name[j];
            }
            // TODO: выделить отдельный стек для бинарника, если нужно
            return tasks[i].pid;
        }
    }
    return -1;
}

void shed_kill(int pid) {
    if (pid < 0 || pid >= MAX_TASKS) return;
    tasks[pid].state = TASK_DEAD;
}

// переключение задач
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
    hlt();
}

void shed_tick() {
    shed_yield();
}

void run_program(uint8_t* buf, size_t size) {
    if (!buf || size == 0)
        return;

    // кастинг бинарника в функцию и вызов
    void (*prog)() = (void(*)())buf;

    prog(); // вызываем "бинарник" как функцию
}

const char* state_to_string(task_state_t state) {
    switch(state) {
        case TASK_DEAD: return "DEAD";
        case TASK_READY: return "READY";
        case TASK_RUNNING: return "RUNNING";
        case TASK_SLEEP: return "SLEEP";
        default: return "???";
    }
}