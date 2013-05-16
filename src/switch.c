#include <switch.h>
#include <bwio.h>

#include <task.h>

void kernel_exit(Task *task) {
    // Push the registers onto the kernel stack.
    asm(
        "stmdb sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}" "\t\n" // Backup the registers.
        //"msr cpsr_c, #31" "\t\n" // Move into System Mode.
        "msr cpsr_c, #16" "\t\n" // Move into User Mode.
        "msr cpsr_c, #31" "\t\n"
        "mov sp, %[user_stack_pointer]" "\t\n" // Install the correct stack pointer.
        "ldmia sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}" "\t\n" // Load all our data.
        //"msr cpsr_c, #19" "\t\n" // Move into Supervisor Mode
        :
        : [user_stack_pointer] "r" (task->sp),
          [user_status_register] "r" (task->spsr)
       );
}

void kernel_enter() {
    asm(
        "ldmia sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}" "\t\n" // Unbackup.
       );

    //unsigned int number;
    //asm("mov %[syscall_number], r7" : [syscall_number] "=r" (number) :);
    //bwprintf(COM2, "Recieved System Call: %d\n\r", number);
}
