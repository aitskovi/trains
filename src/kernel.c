#include <bwio.h>
#include <switch.h>
#include <syscall.h>
#include <task.h>
#include <request.h>
#include <scheduling.h>
#include <user.h>

static Task *active;

/**
 * Simple One Function User Task.
 */
void hello() {
    bwprintf(COM2, "Hello: Initializing\n");
    while (1) {
        bwprintf(COM2, "Hello: Pre-Syscall\n");
        int return_value = MyTid();
        bwprintf(COM2, "Hello: Post-Syscall returned value was %u\n", return_value);
        int parent_tid = MyParentTid();
        bwprintf(COM2, "My Parent Tid is: %d\n", parent_tid);
        Pass();
        bwprintf(COM2, "Passing\n");
        bwprintf(COM2, "Exiting!\n");
        Exit();
    }
}

void initialize_kernel() {
    bwsetfifo(COM2, OFF);

    void (**syscall_handler)() = (void (**)())0x28;
    *syscall_handler = &kernel_enter;

    initialize_scheduling();
    initialize_tasks();
}

/**
 * Handles a request for a task.
 *
 * Returns an integer defining whether a task should be rescheduled.
 */
int handle(Task *task, Request *req) {
    switch (req->request) {
    case MY_TID:
        //bwprintf(COM2, "Got MyTidRequest");
        task_set_return_value(task, task->tid);
        break;
    case CREATE:
        //bwprintf(COM2, "Got Create System Call with priority: %d, code: %x\n\r",
                //req->args[0], req->args[1]);
        if ((int) req->args[0] < 0 || (int) req->args[0] > NUM_PRIORITIES) {
            task_set_return_value(task, -1);
            return 0;
        }
        Task *child = task_create(req->args[1], task->tid, (enum task_priority)req->args[0]);
        if (!child) {
            task_set_return_value(task, -2);
        } else {
            make_ready(child);
            task_set_return_value(task, child->tid);
        }
        break;
    case MY_PARENT_TID:
        //bwprintf(COM2, "Got ParentTid System Call\n");
        task_set_return_value(task, task->parent_tid);
        break;
    case PASS:
        //bwprintf(COM2, "Got Pass System Call\n");
        break;
    case EXIT:
        //bwprintf(COM2, "Got Exit System Call\n");
        return -1;
    default:
        bwprintf(COM2, "Undefined request number %u\n", req->request);
        break;
    }

    return 0;
}

int main() {
    initialize_kernel();

    //bwprintf(COM2, "Kernel Initialized\n");

    //bwprintf(COM2, "Creating Task!\n");
    //bwprintf(COM2, "Hello is %x\n", hello);
    active = task_create(first, 0, MEDIUM);
    //bwprintf(COM2, "Task Created!\n");
    task_print(active);

    make_ready(active);

    Request *req;
    while((active = schedule())) {
        req = kernel_exit(active);
        int should_reschedule = handle(active, req);
        if (should_reschedule == 0) make_ready(active);
    }

    return 0;
}
