#include <ksyscalls.h>

#include <string.h>
#include <assert.h>

#include <memory.h>
#include <messaging.h>
#include <scheduling.h>
#include <task.h>

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

void kmessaging_test() {
    reset();

    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    ksend(t0, t1->tid, msg, msglen, reply, replylen);

    assert(t0->state == SEND_BLOCKED);

    int src;
    char rcvd[7];
    int rcvdlen = 7;
    krecieve(t1, &src, rcvd, rcvdlen);

    assert(t0->state == RPLY_BLOCKED);
    assert(t1->state == READY);
    assert(src == 0);
    assert(task_get_return_value(t1) == 7);
    assert(strcmp(rcvd, msg) == 0);

    char *rply = "Hey!";
    int rplylen = 5;
    kreply(t1, t0->tid, rply, rplylen);

    assert(t1->state == READY);
    assert(t0->state == READY);
    assert(task_get_return_value(t1) == 0);
    assert(task_get_return_value(t0) == 5);
    assert(strcmp(reply, rply) == 0);
}

void krecieve_blocking_test() {
    reset();

    int src;
    char rcvd[7];
    int rcvdlen = 7;
    krecieve(t1, &src, rcvd, rcvdlen);

    assert(t1->state == RECV_BLOCKED);

    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    ksend(t0, t1->tid, msg, msglen, reply, replylen);

    assert(t0->state == RPLY_BLOCKED);
    assert(t1->state == READY);
    assert(task_get_return_value(t1) == 7);
    assert(src == 0);
    assert(strcmp(rcvd, msg) == 0);
}

void kreply_non_blocked_test() {
    reset();

    char *rply = "Hey!";
    int rplylen = 5;
    kreply(t1, t0->tid, rply, rplylen);

    // Check that reply fails!
    assert(t1->state == READY);
    assert(task_get_return_value(t1) == -3);

    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    ksend(t0, t1->tid, msg, msglen, reply, replylen);

    assert(t0->state == SEND_BLOCKED);

    // Check that it fails again!
    kreply(t1, t0->tid, rply, rplylen);
    assert(t1->state == READY);
    assert(task_get_return_value(t1) == -3);

    int src;
    char rcvd[7];
    int rcvdlen = 7;
    krecieve(t1, &src, rcvd, rcvdlen);

    assert(t0->state == RPLY_BLOCKED);
    assert(t1->state == READY);
    assert(src == 0);
    assert(task_get_return_value(t1) == 7);
    assert(strcmp(rcvd, msg) == 0);
}

void ksend_transaction_failed() {
    reset();

    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    ksend(t0, t1->tid, msg, msglen, reply, replylen);

    assert(t0->state == SEND_BLOCKED);

    kexit(t1);
    assert(t0->state == READY);
    assert(task_get_return_value(t0) == -3);
    assert(t1->state == ZOMBIE);
}

void kmytid_test() {
    reset();

    kmytid(t1);

    assert(t1->state == READY);
    assert(task_get_return_value(t1) == 1);
}

void kmy_parent_tid_test() {
    reset();

    kmy_parent_tid(t1);
    kmy_parent_tid(t2);

    assert(t1->state == READY);
    assert(task_get_return_value(t1) == 0);
    assert(t2->state == READY);
    assert(task_get_return_value(t2) == 1);
}

void kcreate_test() {
    reset();

    kcreate(t2, MEDIUM, 0);
    assert(t2->state == READY);
    assert(task_get_return_value(t2) > t2->tid);

    Task *child = task_get(task_get_return_value(t2));
    assert(child != 0);
    assert(child->state == READY);
}

int main() {
    kmessaging_test();
    krecieve_blocking_test();
    kreply_non_blocked_test();
    ksend_transaction_failed();
    kmytid_test();
    kmy_parent_tid_test();
    kcreate_test();
    return 0;
}
