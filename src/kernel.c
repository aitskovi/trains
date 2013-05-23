#include <bwio.h>
#include <switch.h>
#include <syscall.h>
#include <task.h>
#include <request.h>
#include <scheduling.h>

static Task *active;

void interrupt() {
    bwprintf(COM2, "Hello Interrupt!");
}

/**
 * Simple One Function User Task.
 */
void hello() {
    bwprintf(COM2, "Hello: Initializing\n");
    while (1) {
        bwprintf(COM2, "Hello: Pre-Syscall\n");
        int return_value = MyTid(666);
        bwprintf(COM2, "Hello: Post-Syscall returned value was %u\n", return_value);
    }
}

void initialize_kernel() {
    bwsetfifo(COM2, OFF);

    void (**syscall_handler)() = 0x28;
    *syscall_handler = &kernel_enter;

    initialize_scheduling();
    initialize_tasks();
}

void handle(Task *task, Request *req) {
    switch (req->request) {
    case 1:
        bwprintf(COM2, "Got MyTidRequest with argument %d", req->args[0]);
        task_set_return_value(task, task->tid + req->args[0]);
        break;
    default:
        bwprintf(COM2, "Undefined request number %u\n", req->request);
        break;
    }
}

int main() {
    initialize_kernel();

    bwprintf(COM2, "Kernel Initialized\n");

    bwprintf(COM2, "Creating Task!\n");
    bwprintf(COM2, "Hello is %x\n", hello);
    active = task_create(hello, 0, MEDIUM);
    bwprintf(COM2, "Task Created!\n");
    task_print(active);

    make_ready(active);

    unsigned int i, req;
    for (i = 0; i < 4; i++) {
        active = schedule();
        req = kernel_exit(active);
        handle(active, req);
        make_ready(active);
    }

    return 0;
}
