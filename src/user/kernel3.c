#include <log.h>

#include <idle.h>
#include <scheduling.h>
#include <syscall.h>
#include <clock_server.h>
#include <dassert.h>

typedef struct K3Message {
    enum {
        DELAY_DETAILS_REQUEST,
        DELAY_DETAILS_RESPONSE
    } type;
    time_t delay;
    unsigned int num_delays;
} K3Message;

void clock_client() {
    tid_t parent_tid = MyParentTid();
    tid_t my_tid = MyTid();

    K3Message msg, reply;
    msg.type = DELAY_DETAILS_REQUEST;
    Send(parent_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == DELAY_DETAILS_RESPONSE, "Unexpected reply");

    unsigned int i;
    for (i = 0; i < reply.num_delays; ++i) {
        Delay(reply.delay);
        log("Delay number %u completed for task %u having delay interval %u\n", i + 1, my_tid, reply.delay);
    }

    Exit();
}

void first() {
    // Setup the timer interrupt.

    log("First: Initializing\n");
    log("First: Creating Idle Task\n");
    Create(LOWEST, idle);
    log("First: Created Idle Task\n");

    log("First: Creating ClockServer\n");
    Create(HIGHEST, clock_server);

    log("First: Creating Clock Clients\n");
    tid_t three = Create(HIGH, clock_client);
    tid_t four = Create(MEDIUM, clock_client);
    tid_t five = Create(LOW, clock_client);
    tid_t six = Create(LOWER, clock_client);

    unsigned int i;
    tid_t tid;
    K3Message msg, reply;
    reply.type = DELAY_DETAILS_RESPONSE;
    for (i = 0; i < 4; ++i) {
        Receive(&tid, (char *) &msg, sizeof(msg));
        dassert(msg.type == DELAY_DETAILS_REQUEST, "First: Unexpected message type received\n");
        if (tid == three) {
            reply.delay = 10;
            reply.num_delays = 20;
        } else if (tid == four) {
            reply.delay = 23;
            reply.num_delays = 9;
        } else if (tid == five) {
            reply.delay = 33;
            reply.num_delays = 6;
        } else if (tid == six) {
            reply.delay = 71;
            reply.num_delays = 3;
        } else {
            log("First received delay request from unexpected task\n");
        }
        Reply(tid, (char *) &reply, sizeof(reply));
    }

    log("First: Exiting\n");
    Exit();
}
