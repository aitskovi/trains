#include <bwio.h>
#include <switch.h>
#include <syscall.h>
#include <task.h>
#include <request.h>
#include <scheduling.h>
#include <user.h>
#include <time.h>
#include <messaging.h>
#include <ksyscalls.h>
#include <nameserver.h>

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

    initialize_time();
    initialize_scheduling();
    initialize_tasks();
    initialize_messaging();
}

/**
 * Handles a request for a task.
 *
 */
void handle(Task *task, Request *req) {
    switch (req->request) {
    case MY_TID:
        kmytid(task);
        break;
    case CREATE:
        if ((int) req->args[0] < 0 || (int) req->args[0] > NUM_PRIORITIES) {
            task_set_return_value(task, -1);
            make_ready(task);
            return;
        }
        Task *child = task_create(req->args[1], task->tid, (enum task_priority)req->args[0]);
        if (!child) {
            task_set_return_value(task, -2);
        } else {
            make_ready(child);
            task_set_return_value(task, child->tid);
        }
        make_ready(task);
        break;
    case MY_PARENT_TID:
        kmy_parent_tid(task);
        break;
    case PASS:
        make_ready(task);
        break;
    case EXIT:
        kexit(task);
        break;
    case SEND:
        ksend(task, (int)req->args[0], (char *)req->args[1], (int)req->args[2], (char *)req->args[3], (int)req->args[4]);
        break;
    case RECEIVE:
        krecieve(task, (int *)req->args[0], (char *)req->args[1], (int)req->args[2]);
        break;
    case REPLY:
        kreply(task, (int)req->args[0], (char *)req->args[1], (int)req->args[2]);
        break;
    default:
        bwprintf(COM2, "Undefined request number %u\n", req->request);
        break;
    }
}

int main() {
    initialize_kernel();

    // This has to be done after kernel initialization.
    initialize_nameserver();

    //bwprintf(COM2, "Kernel Initialized\n");
    //bwprintf(COM2, "Creating Task!\n");
    //bwprintf(COM2, "Hello is %x\n", hello);
    active = task_create(registration, 0, MEDIUM);
    //bwprintf(COM2, "Task Created!\n");
    //task_print(active);

    make_ready(active);

    Request *req;
    while((active = schedule())) {
        req = kernel_exit(active);
        handle(active, req);
    }

    return 0;
}
