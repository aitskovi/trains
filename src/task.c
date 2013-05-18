#include <task.h>

#include <bwio.h>

#define SL 10
#define FP 11
#define IP 12
#define LR 13

void task_create(Task *t, void (*code)()) {
   t->tid = 1; 
   t->sp = (int *) (t->stack + STACK_SIZE);
   t->sp -= 14; // Make room for 14 registers

   asm(
        "mrs %[status_register], spsr"
        : [status_register] "=r" (t->spsr) 
        :
   );

   int i;
   for (i = 0; i < 10; ++i) {
       t->sp[i] = 0;
   }

   t->sp[SL] = (unsigned int) (t->stack + STACK_SIZE);
   t->sp[IP] = 0;
   t->sp[FP] = t->sp[SL];
   t->sp[LR] = (unsigned int) code;
}

void task_print(Task *t) {
    bwprintf(COM2, "Task id:%d, sp: %x, spsr: %x\n\r",
            t->tid, t->sp, t->spsr);
}
