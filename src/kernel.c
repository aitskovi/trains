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

static Task *active;

void first();

void interrupt() {
    log("Interrupted!\n");
}

void initialize_irq() {
    //int *status = VIC_IRQ_STATUS_OFFSET + VIC_FIQ_STATUS_OFFSET;
    int *enabled = (int *)(VIC1_BASE + VIC_INT_ENABLE_OFFSET);
    log("Interrupts are %x\n", *enabled);
    *enabled = 0x7f7ffff; // Enable ALL Interrupts
    log("Interrupts are %x\n", *enabled);

    // Enable interrupts in the cpu!
    asm("msr cpsr_c, #147");

    // Try to generate an intterupt.
    int *irq_status = (int *)(VIC1_BASE + VIC_IRQ_STATUS_OFFSET);
    log("Interrupt Status: %x\n", *irq_status);
    log("Generating Interrupt\n");
    int *soft_int = (int *)(VIC1_BASE + VIC_SOFT_INT_OFFSET);
    soft_int = 0xf000;
    log("Generated Interrupt!\n");
    irq_status = (int *)(VIC1_BASE + VIC_IRQ_STATUS_OFFSET);
    log("Interrupt Status: %x\n", *irq_status);
}

void initialize_kernel() {
    bwsetfifo(COM2, OFF);

    void (**syscall_handler)() = (void (**)())0x28;
    *syscall_handler = &kernel_enter;

    void (**irq_handler)() = (void (**)())0x38;
    *irq_handler = &interrupt;

    initialize_memory();
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
    default:
        bwprintf(COM2, "Undefined request number %u\n", req->request);
        break;
    }
}

int main() {
    initialize_kernel();

    // This has to be done after kernel initialization.
    initialize_nameserver();
    initialize_irq();

    active = task_create(first, 0, HIGH);
    make_ready(active);

    Request *req;
    while((active = schedule())) {
        req = kernel_exit(active);
        handle(active, req);
    }

    return 0;
}
