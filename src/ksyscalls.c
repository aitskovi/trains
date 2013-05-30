#include <ksyscalls.h>

#include <task.h>
#include <scheduling.h>
#include <messaging.h>

int ksend(Task *active, int tid, char *msg, int msglen, char *reply, int replylen) {
    // TODO Make sure you're not sending message to yourself.
    int invalid = task_is_invalid(tid);
    if (invalid) {
        task_set_return_value(active, invalid);
        make_ready(active);
        return 0;
    }

    int result = msg_send(active->tid, tid, msg, msglen, reply, replylen);
    if (result != 0) return -1;

    Task *task = task_get(tid);
    if (task->state == RECV_BLOCKED) {
        // Unblock the recieve blocked task waiting for a message.
        int recv_result = msg_recieve(tid, 0, 0, 0);
        // TODO Assert that recieve result is not error.
        task_set_return_value(task, recv_result);
        make_ready(task);
        active->state = RPLY_BLOCKED;
    } else {
        active->state = SEND_BLOCKED;
    }

    return 0;
}

int krecieve(Task *active, int *tid, char *msg, int msglen) {
    int result = msg_recieve(active->tid, tid, msg, msglen);
    if (result  == -1) {
        // We had nothing to recieve. We're recv_blocked.
        active->state = RECV_BLOCKED;
    } else {
        // Grab the task, move it to next state.
        Task *task = task_get(*tid);
        task->state = RPLY_BLOCKED;

        // Set our return value, activate!.
        task_set_return_value(active, result);
        make_ready(active);
    }

    return 0;
}

int kreply(Task *active, int tid, char *reply, int replylen) {
    // Check for tid validity.
    int invalid = task_is_invalid(tid);
    if (invalid) {
        task_set_return_value(active, invalid);
        make_ready(active);
        return 0;
    }

    int result = msg_reply(tid, reply, replylen);
    if (result < 0) {
        task_set_return_value(active, -3);
        make_ready(active);
        return 0;
    }

    // Unblock the task replied to.
    Task *task = task_get(tid);
    task_set_return_value(task, result);
    make_ready(task);

    // Make the current task get back on the queue too.
    task_set_return_value(active, 0);
    make_ready(active);

    return 0;
}

int kexit(Task *active) {
    active->state = ZOMBIE;

    // Cleanup any leftover sends.
    int next;
    while ((next = msg_pop(active->tid)) != -1) {
        Task *task = task_get(next);
        task_set_return_value(task, -3);
        make_ready(task);
    }

    return 0;
}
