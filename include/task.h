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
     * The stack for this task.
     */
    unsigned char stack[STACK_SIZE];
} Task;

void task_print(Task *t);
void task_create(Task *t, void (*code)());

#endif
