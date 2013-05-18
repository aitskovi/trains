#include <bwio.h>
#include <switch.h>
#include <syscall.h>
#include <task.h>
#include <request.h>

#define MAX_TASKS 4

static Task tasks[MAX_TASKS];
static Task *active;

void interrupt() {
    bwprintf(COM2, "Hello Interrupt!");
}

/**
 * Simple One Function User Task.
 */
void hello() {
    bwprintf(COM2, "Hello: Initializing\n\r");
    bwprintf(COM2, "Hello: Pre-Syscall");
    syscall(1);
    bwprintf(COM2, "Hello: Post-Syscall");
}

void initialize_kernel() {
	bwsetfifo(COM2, OFF);

    void (**syscall_handler)() = 0x28;
    *syscall_handler = &kernel_enter;

    bwprintf(COM2, "Creating Task!\n\r");
    bwprintf(COM2, "Hello is %x\n\r", hello);
    task_create(&tasks[0], hello);
    bwprintf(COM2, "Task Created!\n\r");
    task_print(&tasks[0]);
}

void handle (Request *req) {
	bwprintf(COM2, "Back in da Kernel!\n\r");
}

Task *schedule () {
	return &tasks[0];
}

int main() {
    initialize_kernel();

    bwprintf(COM2, "Kernel Initialized\n\r");

    unsigned int i; Request req;
    for( i = 0; i < 4; i++ ) {
    	active = schedule();
    	kernel_exit( active, &req );
    	handle( &req );
    }

    return 0;
}
