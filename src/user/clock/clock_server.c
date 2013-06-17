/*
 * clock_server.c
 *
 *  Created on: Jun 8, 2013
 *      Author: aianus
 */

#include <nameserver.h>
#include <clock_server.h>
#include <clock_notifier.h>
#include <syscall.h>
#include <heap_priority_queue.h>
#include <dassert.h>
#include <log.h>

static tid_t server_tid = -1;

int Time() {
    if (server_tid < 0) {
        return -1;
    }
    ClockMessage msg, reply;
    msg.type = TIME_REQUEST;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == TIME_RESPONSE, "Invalid response from clock server");
    return reply.time;
}

int DelayUntil(int time) {
    if (server_tid < 0) {
        return -1;
    }
    ClockMessage msg, reply;
    msg.type = DELAY_UNTIL_REQUEST;
    msg.time = time;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == DELAY_RESPONSE, "Invalid response from clock server");
    return 0;
}

int Delay(int ticks) {
    if (server_tid < 0) {
        return -1;
    } else if (ticks <= 0) {
        return 0;
    }
    ClockMessage msg, reply;
    msg.type = DELAY_REQUEST;
    msg.delay = ticks;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == DELAY_RESPONSE, "Invalid response from clock server");
    return 0;
}

void clock_server() {
    ClockMessage msg, reply;
    tid_t tid;
    time_t time = 0;

    PriorityQueueElement element;
    PriorityQueueElement buffer[MAX_BLOCKED_TASKS + 1];
    HeapPriorityQueue blocked_tasks = priority_queue_create(buffer, MAX_BLOCKED_TASKS + 1);

    dlog("Clock Server: Initialized\n");
    server_tid = MyTid();
    RegisterAs("ClockServer");
    dlog("Clock Server: Registered\n");

    Create(REALTIME, clock_notifier);
    dlog("Clock Server: Notifier Created\n");

    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch(msg.type) {
        case TICK_REQUEST:
            time++;
            dlog("Ticking %u!\n", time);
            reply.type = TICK_RESPONSE;
            Reply(tid, (char *) &reply, sizeof(reply));
            // Unblock all waiting tasks whose time has arrived
            while (priority_queue_size(&blocked_tasks)) {
                PriorityQueueElement head = priority_queue_peek(&blocked_tasks);
                if (head.priority <= time) {
                    priority_queue_extract(&blocked_tasks);
                    reply.type = DELAY_RESPONSE;
                    Reply((tid_t) head.data, (char *) &reply, sizeof(reply));
                } else {
                    break;
                }
            }
            break;
        case TIME_REQUEST:
            reply.type = TIME_RESPONSE;
            reply.time = time;
            Reply(tid, (char *) &reply, sizeof(reply));
            break;
        case DELAY_UNTIL_REQUEST:
            if (msg.time < time) {
                reply.type = DELAY_RESPONSE;
                Reply(tid, (char *) &reply, sizeof(reply));
            } else {
                element.data = (void *) tid;
                element.priority = msg.time;
                priority_queue_insert(&blocked_tasks, element);
            }
            break;
        case DELAY_REQUEST:
            element.data = (void *) tid;
            element.priority = time + msg.delay;
            priority_queue_insert(&blocked_tasks, element);
            dlog("Received delay request from task %u at time %u", tid, time);
            break;
        default:
            log("Clock server received an invalid message\n");
            break;
        }
    }
}
