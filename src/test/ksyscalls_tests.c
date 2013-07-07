#include <ksyscalls.h>

#include <string.h>

#include <memory.h>
#include <messaging.h>
#include <scheduling.h>
#include <task.h>
#include <verify.h>

static Task *t0;
static Task *t1;
static Task *t2;

/**
 * Kernel state reset before syscall tests.
 */
void reset() {
    initialize_memory();
    initialize_tasks();
    initialize_scheduling();
    initialize_messaging();

    t0 = task_create(0, 0, MEDIUM);
    t1 = task_create(0, 0, MEDIUM);
    t2 = task_create(0, 1, MEDIUM);
}

int kmessaging_test() {
    reset();

    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    ksend(t0, t1->tid, msg, msglen, reply, replylen);

    vassert(t0->state == SEND_BLOCKED);

    int src;
    char rcvd[7];
    int rcvdlen = 7;
    krecieve(t1, &src, rcvd, rcvdlen);

    vassert(t0->state == RPLY_BLOCKED);
    vassert(t1->state == READY);
    vassert(src == 0);
    vassert(task_get_return_value(t1) == 7);
    vassert(strcmp(rcvd, msg) == 0);

    char *rply = "Hey!";
    int rplylen = 5;
    kreply(t1, t0->tid, rply, rplylen);

    vassert(t1->state == READY);
    vassert(t0->state == READY);
    vassert(task_get_return_value(t1) == 0);
    vassert(task_get_return_value(t0) == 5);
    vassert(strcmp(reply, rply) == 0);

    return 0;
}

int krecieve_blocking_test() {
    reset();

    int src;
    char rcvd[7];
    int rcvdlen = 7;
    krecieve(t1, &src, rcvd, rcvdlen);

    vassert(t1->state == RECV_BLOCKED);

    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    ksend(t0, t1->tid, msg, msglen, reply, replylen);

    vassert(t0->state == RPLY_BLOCKED);
    vassert(t1->state == READY);
    vassert(task_get_return_value(t1) == 7);
    vassert(src == 0);
    vassert(strcmp(rcvd, msg) == 0);

    return 0;
}

int kreply_non_blocked_test() {
    reset();

    char *rply = "Hey!";
    int rplylen = 5;
    kreply(t1, t0->tid, rply, rplylen);

    // Check that reply fails!
    vassert(t1->state == READY);
    vassert(task_get_return_value(t1) == -3);

    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    ksend(t0, t1->tid, msg, msglen, reply, replylen);

    vassert(t0->state == SEND_BLOCKED);

    // Check that it fails again!
    kreply(t1, t0->tid, rply, rplylen);
    vassert(t1->state == READY);
    vassert(task_get_return_value(t1) == -3);

    int src;
    char rcvd[7];
    int rcvdlen = 7;
    krecieve(t1, &src, rcvd, rcvdlen);

    vassert(t0->state == RPLY_BLOCKED);
    vassert(t1->state == READY);
    vassert(src == 0);
    vassert(task_get_return_value(t1) == 7);
    vassert(strcmp(rcvd, msg) == 0);

    return 0;
}

int ksend_transaction_failed() {
    reset();

    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    ksend(t0, t1->tid, msg, msglen, reply, replylen);

    vassert(t0->state == SEND_BLOCKED);

    kexit(t1);
    vassert(t0->state == READY);
    vassert(task_get_return_value(t0) == -3);
    vassert(t1->state == ZOMBIE);

    return 0;
}

int kmytid_test() {
    reset();

    kmytid(t1);

    vassert(t1->state == READY);
    vassert(task_get_return_value(t1) == 1);

    return 0;
}

int kmy_parent_tid_test() {
    reset();

    kmy_parent_tid(t1);
    kmy_parent_tid(t2);

    vassert(t1->state == READY);
    vassert(task_get_return_value(t1) == 0);
    vassert(t2->state == READY);
    vassert(task_get_return_value(t2) == 1);

    return 0;
}

int kcreate_test() {
    reset();

    kcreate(t2, MEDIUM, 0, 0);
    vassert(t2->state == READY);
    vassert(task_get_return_value(t2) > t2->tid);

    Task *child = task_get(task_get_return_value(t2));
    vassert(child != 0);
    vassert(child->state == READY);

    return 0;
}

int kwait_tid_test() {
    reset();

    kexit(t0);
    vassert(t0->state == ZOMBIE);

    // Shouldn't block when calling wait on ZOMBIE task
    kwait_tid(t1, t0->tid);
    vassert(t1->state == READY);
    vassert(task_get_return_value(t1) == 0);
    vassert(t0->state == ZOMBIE);

    // Should block on non zombie.
    kwait_tid(t1, t2->tid);
    vassert(t2->state != ZOMBIE);
    vassert(t1->state == WAIT_BLOCKED);

    // Should unblock when non zombie exits.
    kexit(t2);
    vassert(t2->state == ZOMBIE);
    vassert(t1->state == READY);

    return 0;
}

struct vsuite* syscalls_suite() {
    struct vsuite *suite = vsuite_create("Syscalls Tests", reset);
    vsuite_add_test(suite, kmessaging_test);
    vsuite_add_test(suite, krecieve_blocking_test);
    vsuite_add_test(suite, kreply_non_blocked_test);
    vsuite_add_test(suite, ksend_transaction_failed);
    vsuite_add_test(suite, kmytid_test);
    vsuite_add_test(suite, kmy_parent_tid_test);
    vsuite_add_test(suite, kcreate_test);
    vsuite_add_test(suite, kwait_tid_test);
    return suite;
}
