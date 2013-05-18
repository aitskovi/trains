#include <switch.h>
#include <bwio.h>
#include <request.h>
#include <task.h>

unsigned int kernel_exit(Task *active) {
	bwprintf( COM2, "In kernel_exit\n\r" );
	bwprintf( COM2, "kernel_exit activating\n\r" );
	kernel_enter( );
	bwprintf( COM2, "exiting kernel_exit\n\r" );
}

void kernel_enter() {

}

/*
void kernel_exit(Task *task) {
    // Push the registers onto the kernel stack.
    asm(
        "stmdb sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}" "\n\t" // Backup the registers.
        "msr cpsr_c, #31" "\n\t" // System Mode.
        "mov sp, %[user_stack_pointer]" "\n\t" // Install the correct stack pointer.
        "ldmia sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}" "\n\t" // Load all our data.
        "mov r3, lr" "\n\t"
        "msr cpsr_c, #19" "\n\t" // Supervisor Mode
        "msr spsr_c, #16" "\n\t" // Make SPSR User Mode.
        "movs pc, r3" "\n\t" // Le Go!
        :
        : [user_stack_pointer] "r" (task->sp),
          [user_status_register] "r" (task->spsr)
       );
}

// Backup Kernel Registers.
// Move into System Mode.
// Unload all the information.
//

void kernel_enter(Task *t) {
    asm(
        "ldmia sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}" "\n\t" // Unbackup.
       );
}
*/

