#ifndef _TASK_H_
#define _TASK_H_

#include <scheduling.h>

#define STACK_SIZE 1024 * 50 // 50kb stack

#define MAX_TASKS 20

typedef int tid_t;

typedef struct Task {
    /**
     * The id of the task.
     */
    tid_t tid;

    /**
     * The stack pointer for the task.
     */
    int *sp;

    /**
     * The parent tid of the task
     */
    unsigned int parent_tid;

    /**
     * The priority of the task
     */
    enum task_priority priority;

    /**
     * The state of the task.
     */
    enum task_state state;

    /**
     * Used for task queueing.
     */
    struct Task *next;

    /**
     * Used to store how long a task has been active
     */
    unsigned int cpu_time;

    /**
     * The stack for this task.
     */
    unsigned char *stack;
} Task;

Task * task_create(void (*code)(), tid_t parent_tid, enum task_priority priority);

void task_print(Task *t);

void *task_get_pc(Task *t);

int *task_get_sp(Task *t);
void task_save_sp(Task *t, int *sp);

int task_get_return_value(Task *t);
void task_set_return_value(Task *t, int value);

Task *task_get(tid_t tid);

int task_is_invalid(tid_t tid);

void initialize_tasks();

Task * get_tasks(unsigned int *size);

#endif
