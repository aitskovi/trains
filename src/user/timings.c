/*
 * timings.c
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#include <bwio.h>
#include <syscall.h>
#include <fine_timer.h>
#include <log.h>

#define MESSAGE_LENGTH 64
#define LOOPS 10

static FineTimer timer;

void consumer() {
    int src, length, result;
    char msg[MESSAGE_LENGTH];
    char reply[MESSAGE_LENGTH];
    unsigned int i;
    for (i = 0; i < LOOPS; ++i) {
        length = Receive(&src, msg, MESSAGE_LENGTH);
        result = Reply(src, reply, MESSAGE_LENGTH);
    }
    Exit();
}

void producer() {
    char msg[MESSAGE_LENGTH];
    char reply[MESSAGE_LENGTH];

    tid_t consumer_tid = Create(HIGH, consumer);

    unsigned int i;
    for (i = 0; i < LOOPS; ++i) {
        fine_timer_reset(&timer);
        Send(consumer_tid, msg, MESSAGE_LENGTH, reply, MESSAGE_LENGTH);
        log("SEND/RECEIVE/REPLY took %uus\n", fine_time_to_usec(fine_timer_elapsed(&timer)));
    }
    Exit();
}

void first() {
    FineTimer timer;

    unsigned int i;
    for (i = 0; i < 4; ++i) {
        fine_timer_reset(&timer);
        Pass();
        unsigned int elapsed = fine_timer_elapsed(&timer);
        log("Round trip took %u usec\n", fine_time_to_usec(elapsed));
    }

    Create(MEDIUM, producer);

    Exit();
}
