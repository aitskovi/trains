#include <bwio.h>
#include <switch.h>
#include <syscall.h>
#include <task.h>

void interrupt() {
    bwprintf(COM2, "Hello Interrupt!");
}

/**
 * Simple One Function User Task.
 */
void hello() {
    bwprintf(COM2, "Hello: Initializing\n\r");
    //for(;;) {
        //bwprintf(COM2, "Hello: Pre-Syscall");
        //syscall(1);
        //bwprintf(COM2, "Hello: Post-Syscall");
    //}
}

int main() {
    bwsetfifo(COM2, OFF);
    bwprintf(COM2, "Hello World!\n\r");

    void (**syscall_handler)() = 0x28;
    *syscall_handler = software_interrupt;

    //syscall(1);

    Task t;
    bwprintf(COM2, "Creating Task!\n\r");
    task_create(&t, hello);
    bwprintf(COM2, "Task Created!\n\r");
    bwprintf(COM2, "Hello is %x\n\r", hello);
    task_print(&t);
    kernel_exit(&t);

    return 0;
}
