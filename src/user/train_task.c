/*
 * train_task.c
 *
 *  Created on: Jun 30, 2013
 *      Author: aianus
 */

#include <dassert.h>
#include <write_server.h>
#include <ts7200.h>
#include <train_task.h>
#include <task.h>
#include <syscall.h>
#include <encoding.h>
#include <dassert.h>
#include <clock_server.h>
#include <nameserver.h>

static void notify_speed_change(int train, speed_t speed, tid_t tid) {
    // Notify the distance server of the change.
    Message msg, reply;
    msg.type = TRAIN_MESSAGE;
    msg.tr_msg.type = COMMAND_SET_SPEED;
    msg.tr_msg.speed = speed;
    msg.tr_msg.train = train;
    Send(tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    cuassert(TRAIN_MESSAGE == reply.type, "Train task received invalid msg");
}

static void train_set_speed(int train, speed_t speed, tid_t server_tid) {
    char set_speed_command[2] = { speed, train };
    Write(COM1, set_speed_command, sizeof(set_speed_command));
    notify_speed_change(train, speed, server_tid);
}

static void train_reverse(int train, speed_t new_speed, tid_t location_server) {
    // Stop the Train.
    train_set_speed(train, 0, location_server);

    // Block for a while (1.5s) to let train stop.
    // TODO adjust this for speed
    Delay(150);

    // Reverse the Train.
    train_set_speed(train, 15, location_server);

    // Reset the train to it's original speed.
    train_set_speed(train, new_speed, location_server);
}

void train_task(int train_no) {

    // Find Mission Control Tid.
    tid_t mission_control_tid = MyParentTid();
    tid_t location_server_tid = WhoIs("LocationServer");
  
    speed_t our_speed = 0;

    Message msg, reply;
    TrainMessage *tr_msg = &msg.tr_msg;
    TrainMessage *command = &reply.tr_msg;
    msg.type = TRAIN_MESSAGE;
    tr_msg->type = COMMAND_AWAITING;

    while (1) {
        //ulog("\nTrain awaiting command");
        Send(mission_control_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
        //ulog("\nTrain got command");
        cuassert(TRAIN_MESSAGE == reply.type, "Train task received invalid msg");

        switch (command->type) {
        case (COMMAND_SET_SPEED):
            our_speed = command->speed;
            train_set_speed(train_no, command->speed, location_server_tid);
            break;
        case (COMMAND_REVERSE):
            train_reverse(train_no, our_speed, location_server_tid);
            break;
        default:
            cuassert(0, "Invalid Train Task Command");
            break;
        }
    }
}
