#ifndef _TASK_H_
#define _TASK_H_

#include <scheduling.h>

#define STACK_SIZE 1024 * 50 // 50kb stack

#define MAX_TASKS 10

typedef int tid_t;

typedef struct Task {
    /**
     * The id of the task.
     */
    tid_t tid;

    /**
     * The current state of the task.
     */
    // enum TASK_STATE state;

    /**
     * The stack pointer for the task.
     */
    int *sp;

    /**
     * The saved program status register.
     * Do we really need to save this?
     */
    unsigned int spsr;

    /**
     * The stored pc for the task.
     */
    void *pc;

    /**
     * The return value to a syscall
     */
    int return_value;

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
     * The stack for this task.
     */
    unsigned char *stack;
} Task;

void task_print(Task *t);
Task * task_create(void (*code)(), tid_t parent_tid, enum task_priority priority);
int *task_get_sp(Task *t);
unsigned int task_get_spsr(Task *t);
void *task_get_pc(Task *t);
void task_save_pc(Task *t, void *pc);
void task_save_sp(Task *t, int *sp);
void task_save_spsr(Task *t, unsigned int spsr);
void task_set_return_value (Task *t, int value);
int task_get_return_value(Task *t);

Task *task_get(tid_t tid);
int task_is_invalid(tid_t tid);

void initialize_tasks();

#endif
