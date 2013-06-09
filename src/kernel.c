#include <bwio.h>
#include <switch.h>
#include <syscall.h>
#include <task.h>
#include <request.h>
#include <scheduling.h>
#include <time.h>
#include <messaging.h>
#include <ksyscalls.h>
#include <nameserver.h>
#include <memory.h>
#include <ts7200.h>
#include <log.h>

static Task *active;

void first();

void initialize_cache() {
    asm("mov r1, #0");
    asm("mcr p15, 0, r1, c7, c5, 0");
    asm("mrc p15, 0, r1, c1, c0, 0");
    asm("orr r1, r1, #4096");
    asm("orr r1, r1, #4");
    asm("mcr p15, 0, r1, c1, c0, 0");
}

void initialize_kernel() {
    bwsetfifo(COM2, OFF);

    void (**syscall_handler)() = (void (**)())0x28;
    *syscall_handler = &kernel_enter;

    void (**irq_handler)() = (void (**)())0x38;
    *irq_handler = &irq_enter;

    initialize_cache();
    initialize_memory();
    initialize_time();
    initialize_scheduling();
    initialize_tasks();
    initialize_messaging();
    initialize_events();
}

/**
 * Handles a request for a task.
 *
 */
void handle(Task *task, Request *req) {
    // Handle Interrupts.
    if (req == 0) {
        int data;
        int event = process_interrupt(&data); 
        kevent(event, data);
        make_ready(task);
        return;
    }


    // Handle System Calls.
    switch (req->request) {
    case MY_TID:
        kmytid(task);
        break;
    case CREATE:
        kcreate(task, (int)req->args[0] /* priority */, req->args[1] /* code */);
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
    case AWAIT_EVENT:
        kawait(task, (int)req->args[0]);
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

    active = task_create(first, 0, REALTIME);
    make_ready(active);

    Request *req;
    while((active = schedule())) {
        req = kernel_exit(active);
        handle(active, req);
    }

    return 0;
}
