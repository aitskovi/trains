#include <task.h>

#include <bwio.h>
#include <dassert.h>
#include <memory.h>

enum trap_frame {
    RVALUE = 0,
    SPSR,
    PC,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    SL,
    FP,
    LR,
    TRAP_FRAME_SIZE
};

#define USER_MODE_SPSR 0x10

static struct Task tasks[MAX_TASKS];
static unsigned int next_tid = 0;

Task * task_create(void (*code)(), tid_t parent_tid, enum task_priority priority) {
    // Don't go off the end of the task array.
    if (next_tid == MAX_TASKS) return 0;

    Task *t = &tasks[next_tid++];

    // Set-up our bookkeeping info. 
    t->tid = next_tid - 1;
    t->parent_tid = parent_tid;
    t->priority = priority;
    t->state = UNKNOWN;
    t->next = 0;

    // Allocate a stack for us from our heap.
    t->stack = kmalloc(STACK_SIZE);
    dassert(t->stack != 0, "Task Stack Allocation Failed");

    t->sp = (int *) (t->stack + STACK_SIZE);
    t->sp -= TRAP_FRAME_SIZE;   // Make room for the trap frame.

    // Generate the trap frame.
    t->sp[RVALUE] = 0;
    t->sp[SPSR] = USER_MODE_SPSR;
    t->sp[PC] = code;
    t->sp[R4] = 0;
    t->sp[R5] = 0;
    t->sp[R6] = 0;
    t->sp[R7] = 0;
    t->sp[R8] = 0;
    t->sp[R9] = 0;
    t->sp[SL] = (unsigned int) (t->stack + STACK_SIZE);
    t->sp[FP] = 0;
    t->sp[LR] = code;

    return t;
}

void task_set_return_value(Task *t, int value) {
    t->sp[RVALUE] = value;
}

int *task_get_sp(Task *t) {
    return t->sp;
}

void task_save_sp(Task *t, int *sp) {
    t->sp = sp;
    //bwprintf(COM2, "Saved sp: %x\n", t->sp);
}

Task *task_get(tid_t tid) {
    if (tid < next_tid && tid >= 0) return &tasks[tid];
    else return 0;
}

int task_is_invalid(tid_t tid) {
    Task *task = task_get(tid);
    if (task == 0) return -1;
    else if (task->state == ZOMBIE) return -2;
    else return 0;
}

void task_print(Task *t) {
    bwprintf(COM2, "TD address:%x Task id:%d, sp: %x", t, t->tid,
            t->sp);
}

void initialize_tasks() {
    next_tid = 0;
}
