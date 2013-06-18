#include <bwio.h>
#include <switch.h>
#include <syscall.h>
#include <task.h>
#include <request.h>
#include <scheduling.h>
#include <fine_timer.h>
#include <messaging.h>
#include <ksyscalls.h>
#include <nameserver.h>
#include <memory.h>
#include <ts7200.h>
#include <log.h>
#include <waiting.h>
#include <interrupt.h>

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
    initialize_fine_timer();
    initialize_scheduling();
    initialize_tasks();
    initialize_messaging();
    initialize_events();
    initialize_waiting();
}

/**
 * Handles a request for a task.
 *
 */
int handle(Task *task, Request *req) {
    // Handle Interrupts.
    if (req == 0) {
        int data;
        int event = process_interrupt(&data); 
        kevent(event, data);
        make_ready(task);
        return 0;
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
    case WAIT_TID:
        kwait_tid(task, (int)req->args[0]);
        break;
    case SHUTDOWN:
        return -1;
    default:
        bwprintf(COM2, "Undefined request number %u\n", req->request);
        break;
    }

    return 0;
}

void dump_timing_info() {
    unsigned int num_tasks, i, total_cpu_time;
    Task *tasks = get_tasks(&num_tasks);
    total_cpu_time = 0;
    for (i = 0; i < num_tasks; ++i) {
        total_cpu_time += tasks[i].cpu_time;
    }
    log("\nTask runtime info:\n");
    for (i = 0; i < num_tasks; ++i) {
        unsigned int microseconds;
        microseconds = fine_time_to_usec(tasks[i].cpu_time);
        float percent = 100.0f * tasks[i].cpu_time / total_cpu_time;
        log("Task %u ran for %uus or %u percent of the time\n", tasks[i].tid, microseconds, (int) percent);
    }
}

int main() {
    initialize_kernel();

    active = task_create(first, 0, REALTIME);
    make_ready(active);

    Request *req;
    FineTimer timer;
    while((active = schedule())) {
        fine_timer_reset(&timer);
        req = kernel_exit(active);
        active->cpu_time += fine_timer_elapsed(&timer);
        int shutdown = handle(active, req);
        if (shutdown) break;
    }

    // Print out runtime stats
    dump_timing_info();

    log ("Kernel exiting\n");
    disable_interrupts();
    return 0;
}
