#include <task.h>

#include <bwio.h>

#define SL 10
#define FP 11
#define IP 12
#define LR 13

#define USER_MODE_FLAG 0x10

void task_create(Task *t, void (*code)()) {
   t->tid = 1; 
   t->sp = (int *) (t->stack + STACK_SIZE);
   t->sp -= 14; // Make room for 14 registers

   asm(
        "mrs %[status_register], cpsr"
        : [status_register] "=r" (t->spsr)
        :
   );
   t->spsr = ((t->spsr >> 5) << 5) | USER_MODE_FLAG;

   int i;
   for (i = 0; i < 10; ++i) {
       t->sp[i] = 0;
   }

   t->sp[SL] = (unsigned int) (t->stack + STACK_SIZE);
   t->sp[IP] = 0;
   t->sp[FP] = t->sp[SL];
   t->sp[LR] = (unsigned int) code; // TODO change this to an exit function

   t->pc = code;
}

int *task_get_sp(Task *t) {
	return t->sp;
}

unsigned int task_get_spsr(Task *t) {
	return t->spsr;
}

void * task_get_pc(Task *t) {
	return t->pc;
}

void task_print(Task *t) {
    bwprintf(COM2, "TD address:%x Task id:%d, sp: %x, spsr: %x\n\r",
            t, t->tid, t->sp, t->spsr);
}
