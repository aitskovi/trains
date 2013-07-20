#include <ksyscalls.h>

#include <dassert.h>
#include <task.h>
#include <scheduling.h>
#include <messaging.h>
#include <waiting.h>

int ksend(Task *active, int tid, char *msg, int msglen, char *reply, int replylen) {
    int logging = active->tid == 36 || active->tid == 34;
    // TODO Make sure you're not sending message to yourself.
    int invalid = task_is_invalid(tid);
    if (invalid) {
        task_set_return_value(active, invalid);
        make_ready(active);
        return 0;
    }

    int result = msg_send(active->tid, tid, msg, msglen, reply, replylen);
    if (result != 0) {
        if (logging) log("%d Message Failed!", active->tid);
        task_set_return_value(active, result);
        make_ready(active);
        return -1;
    }

    Task *task = task_get(tid);
    if (task->state == RECV_BLOCKED) {
        if (logging) log("%d Got Rply Blocked", active->tid);
        // Unblock the recieve blocked task waiting for a message.
        int recv_result = msg_recieve(tid, 0, 0, 0);
        // TODO Assert that recieve result is not error.
        task_set_return_value(task, recv_result);
        make_ready(task);
        active->state = RPLY_BLOCKED;
    } else {
        if (logging) log("%d, Got Send Blocked", active->tid);
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
        int logging = *tid == 36 || *tid == 34;
        if (logging) log("%d got unblocked by %d", *tid, active->tid);
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

    int logging = tid == 36 || tid == 34;
    if (logging) log("Replying to %d from %d", tid, active->tid);

    int result = msg_reply(tid, reply, replylen);
    if (result < 0) {
        task_set_return_value(active, -3);
        make_ready(active);
        return 0;
    }

    // Unblock the task replied to.
    Task *task = task_get(tid);
    ckassert(task->state == RPLY_BLOCKED, "Task is not reply blocked");
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

    // Unblock any tasks that are waiting on this task to exit.
    Task *waiter;
    while ((waiter = waiting_pop(active))) {
        task_set_return_value(waiter, 0);
        make_ready(waiter);
    }

    return 0;
}

int kmytid(Task *task) {
    task_set_return_value(task, task->tid);
    make_ready(task);

    return 0;
}

int kmy_parent_tid(Task *task) {
    task_set_return_value(task, task->parent_tid);
    make_ready(task);

    return 0;
}

int kcreate(struct Task *task, int priority, void(*code)(int), int arg) {
    if (priority < 0 || priority > NUM_PRIORITIES) {
        task_set_return_value(task, -1);
        make_ready(task);
        return 0;
    }

    Task *child = task_create(code, task->tid, (enum task_priority)priority);
    if (!child) {
        task_set_return_value(task, -2);
    } else {
        task_set_return_value(child, arg);
        make_ready(child);
        task_set_return_value(task, child->tid);
    }
    make_ready(task);

    return 0;
}

int kwait_tid(Task *active, int tid) {
    Task *task = task_get(tid);
    if (!task) {
        task_set_return_value(active, -1);
        make_ready(active);
        return 0;
    }

    int error = waiting_add(task, active);
    if (!error) {
        // Block until task completes.
        active->state = WAIT_BLOCKED;
    } else {
        // Task has already completed, just reschedule right away.
        task_set_return_value(active, 0);
        make_ready(active);
    }

    return 0;
}
