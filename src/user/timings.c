/*
 * timings.c
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#include <bwio.h>
#include <syscall.h>
#include <time.h>

#define MESSAGE_LENGTH 64
#define LOOPS 10

static Timer timer;

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
        timer_reset(&timer);
        int result = Send(consumer_tid, msg, MESSAGE_LENGTH, reply, MESSAGE_LENGTH);
        bwprintf(COM2, "SEND/RECEIVE/REPLY took %uus\n", timer_elapsed(&timer).useconds);
    }
    Exit();
}

void first() {
    Timer timer;

    unsigned int i;
    for (i = 0; i < 4; ++i) {
        timer_reset(&timer);
        Pass();
        Time elapsed = timer_elapsed(&timer);
        bwprintf(COM2, "Round trip took %u usec\n", elapsed.useconds);
    }

    Create(MEDIUM, producer);

    Exit();
}
