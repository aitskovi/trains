/*
 * timings.c
 *
 *  Created on: 2013-05-29
 *      Author: alex
 */

#include <bwio.h>
#include <syscall.h>
#include <time.h>

void producer() {
    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    bwprintf(COM2, "Producer: Sending %s\n", msg);
    int result = Send(2, msg, msglen, reply, replylen);
    bwprintf(COM2, "Producer: Send Result: %d\n", result);
    bwprintf(COM2, "Producer: Reply: %s\n", reply);
    Exit();
}

void consumer() {
    int src;
    char msg[7];
    int msglen = 7;
    bwprintf(COM2, "Consumer: Receiving\n");
    int length = Receive(&src, msg, msglen);
    bwprintf(COM2, "Consumer: Received Length %d\n", length);
    bwprintf(COM2, "Consumer: Recieved Message from %d\n", src);
    bwprintf(COM2, "Consumer: Received Msg: %s\n", msg);

    char *reply = "Hey!";
    int replylength = 5;
    bwprintf(COM2, "Consumer: Replying %s\n", reply);
    int result = Reply(src, reply, replylength);
    bwprintf(COM2, "Consumer: Reply Result %d\n", result);
    Exit();
}

void communication() {
    int tid;
    tid = Create(LOW, producer);
    bwprintf(COM2, "Created Producer: <%d>\n", tid);
    tid = Create(LOW, consumer);
    bwprintf(COM2, "Created Consumer: <%d>\n", tid);
    bwprintf(COM2, "Communication: exiting\n");
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

    Create(MEDIUM, communication);

    Exit();
}
