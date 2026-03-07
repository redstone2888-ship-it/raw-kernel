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

extern task_t tasks[MAX_TASKS];
extern int current_task;

void shed_init();
int shed_add(void (*fn)(), const char* name);
void shed_kill(int pid);
void shed_yield();
void shed_tick();

const char* state_to_string(task_state_t state);

#endif