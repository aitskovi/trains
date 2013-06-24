/*
 * train_server.c
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#include <train_server.h>
#include <clock_server.h>
#include <write_server.h>
#include <dassert.h>
#include <syscall.h>
#include <ts7200.h>

static char train_speeds[NUM_TRAINS];

static tid_t server_tid = -1;

int SetSpeed(train_t train, speed_t speed) {
    if (server_tid < 0) {
        return -1;
    }
    TrainServerMessage msg, reply;
    msg.type = SET_SPEED;
    msg.train_no = train;
    msg.speed = speed;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == SET_SPEED_RESPONSE, "Invalid response from train server");
    return 0;
}

int Reverse(train_t train) {
    if (server_tid < 0) {
        return -1;
    }
    TrainServerMessage msg, reply;
    msg.type = REVERSE;
    msg.train_no = train;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == REVERSE_RESPONSE, "Invalid response from train server");
    return 0;
}

static void train_set_speed_internal(int train, int speed) {
    char set_speed_command[2] = { speed, train };
    Write(COM1, set_speed_command, sizeof(set_speed_command));
}

static void train_set_speed(int train, int speed) {
    train_speeds[train] = speed;
    train_set_speed_internal(train, speed);
}

static void train_reverse(int train) {
    // Stop the Train.
    train_set_speed_internal(train, 0);

    // Block for a while (1.5s) to let train stop.
    Delay(150);

    // Reverse the Train.
    train_set_speed_internal(train, 15);

    // Reset the train to it's original speed.
    train_set_speed_internal(train, train_speeds[train]);
}

void train_server() {
    server_tid = MyTid();

    // Initialize train speeds
    memset(train_speeds, 0, sizeof(train_speeds));

    tid_t tid;
    TrainServerMessage msg, reply;
    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch(msg.type) {
        case SET_SPEED:
            train_set_speed(msg.train_no, msg.speed);
            reply.type = SET_SPEED_RESPONSE;
            Reply(tid, (char *) &reply, sizeof(reply));
            break;
        case REVERSE:
            reply.type = REVERSE_RESPONSE;
            Reply(tid, (char *) &reply, sizeof(reply));
            train_reverse(msg.train_no);
            break;
        }
    }
}
