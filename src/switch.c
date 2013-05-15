#include <switch.h>
#include <bwio.h>

#include <task.h>

static void *ksp = 0;

void kernel_exit(Task *task) {
    // Push the registers onto the kernel stack.
    asm(
        "stmfd sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r14, r15}" "\t\n"
        "str sp, %[kernel_stack_pointer]" "\t\n"
        "mov sp, %[user_stack_pointer]" "\t\n"
        "msr spsr, %[user_status_register]" "\t\n"
        "ldmfd sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r14, r15}" "\t\n"
        : [kernel_stack_pointer] "=m" (ksp)
        : [user_stack_pointer] "r" (task->sp),
          [user_status_register] "r" (task->spsr)
       );
    // SP points to kernel stack atm.
    // Change mode to user mode.
    // Insert sp and spsr into registers.
    // Resume task state (unpop registers)
    // Resume the pc of active task.
}
