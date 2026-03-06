#ifndef SHED_H
#define SHED_H

#include <stdint.h>
#include <stddef.h>

#define MAX_TASKS 16

typedef enum {
    TASK_DEAD = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_SLEEP
} task_state_t;

typedef struct task {
    int pid;
    task_state_t state;
    void* stack;
    char name[16];

    void (*entry)();
    uint8_t* program;
    size_t size;
} task_t;

// глобальные задачи
extern task_t tasks[MAX_TASKS];
extern int current_task;

// инициализация
void shed_init();

// добавить функцию
int shed_add(void (*fn)(), const char* name);

// добавить бинарник
int shed_add_binary(const char* name, uint8_t* buf, size_t size);

// убить задачу
void shed_kill(int pid);

// переключение
void shed_yield();
void shed_tick();

// функция для запуска бинарника
void run_program(uint8_t* buf, size_t size);

const char* state_to_string(task_state_t state);

#endif