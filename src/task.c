#include <task.h>

#include <bwio.h>

#define SL 10
#define IP 11
#define FP 12
#define LR 13
#define PC 14

void task_create(Task *t, void (*code)()) {
   t->tid = 1; 
   t->pc = code;
   t->sp = t->stack + STACK_SIZE - 15;
   asm(
        "mrs %[status_register], spsr"
        : [status_register] "=r" (t->spsr) 
        :
   );

   int i;
   for (i = 0; i < 10; ++i) {
       t->sp[i] = 0;
   }

   t->sp[SL] = t->stack + STACK_SIZE;
   t->sp[IP] = 0;
   t->sp[FP] = t->sp[SL];
   t->sp[LR] = 0;
   t->sp[PC] = t->pc;
}

void task_print(Task *t) {
    bwprintf(COM2, "Task id:%d, sp: %x, spsr: %x, pc: %x\n\r",
            t->tid, t->sp, t->spsr, t->pc);
}
