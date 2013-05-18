#ifndef _TASK_H_
#define _TASK_H_

/*
enum TASK_STATE {
    TASK_ACTIVE,
    TASK_READY,
};
*/

#define STACK_SIZE 1048576 // 1mb stack

typedef struct Task {
    /**
     * The id of the task.
     */
    unsigned int tid;

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
     * The stack for this task.
     */
    unsigned char stack[STACK_SIZE];
} Task;

void task_print(Task *t);
void task_create(Task *t, void (*code)());
int *task_get_sp(Task *t);
unsigned int task_get_spsr(Task *t);
void *task_get_pc(Task *t);
void task_save_pc(Task *t, void *pc);
void task_save_sp(Task *t, int *sp);
void task_save_spsr(Task *t, unsigned int spsr);

#endif
