#include <task.h>

#include <bwio.h>

#define SL 6
#define FP 7
#define IP 8
#define LR 9

#define USER_MODE_SPSR 0x10

static struct Task tasks[MAX_TASKS];
static unsigned int next_tid = 0;

Task * task_create(void (*code)(), unsigned int parent_tid, enum task_priority priority) {
    // Don't go off the end of the task array.
    if (next_tid == MAX_TASKS) return 0;

    Task *t = &tasks[next_tid++];

    t->next = 0;
    t->tid = next_tid - 1;
    t->sp = (int *) (t->stack + STACK_SIZE);
    t->sp -= 10; // Make room for 14 registers
    t->spsr = USER_MODE_SPSR;

    int i;
    for (i = 0; i < 6; ++i) {
        t->sp[i] = 0;
    }

    t->sp[SL] = (unsigned int) (t->stack + STACK_SIZE);
    t->sp[IP] = 0;
    t->sp[FP] = t->sp[SL];
    t->sp[LR] = (unsigned int) code; // TODO change this to an exit function

    t->pc = code;

    t->parent_tid = parent_tid;
    t->priority = priority;
    t->state = UNKNOWN;
    return t;
}

void task_set_return_value(Task *t, int value) {
    t->return_value = value;
}

int *task_get_sp(Task *t) {
    return t->sp;
}

int task_get_return_value(Task *t) {
    return t->return_value;
}

unsigned int task_get_spsr(Task *t) {
    return t->spsr;
}

void * task_get_pc(Task *t) {
    return t->pc;
}

void task_save_pc(Task *t, void *pc) {
    t->pc = pc;
    //bwprintf(COM2, "Saved pc: %x\n", t->pc);
}

void task_save_sp(Task *t, int *sp) {
    t->sp = sp;
    //bwprintf(COM2, "Saved sp: %x\n", t->sp);
}

void task_save_spsr(Task *t, unsigned int spsr) {
    t->spsr = spsr;
    //bwprintf(COM2, "Saved spsr: %x\n", t->spsr);
}

Task *task_get(int tid) {
    if (tid < next_tid && tid >= 0) return &tasks[tid];
    else return 0;
}

int task_is_invalid(int tid) {
    Task *task = task_get(tid);
    if (task == 0) return -1;
    else if (task->state == ZOMBIE) return -2;
    else return 0;
}

void task_print(Task *t) {
    bwprintf(COM2, "TD address:%x Task id:%d, sp: %x, spsr: %x\n\r", t, t->tid,
            t->sp, t->spsr);
}

void initialize_tasks() {
    next_tid = 0;
}
