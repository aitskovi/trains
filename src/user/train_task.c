/*
 * train_task.c
 *
 *  Created on: Jun 30, 2013
 *      Author: aianus
 */

#include <write_server.h>
#include <ts7200.h>
#include <train_task.h>
#include <task.h>
#include <syscall.h>
#include <encoding.h>
#include <dassert.h>

static void train_set_speed_internal(int train, speed_t speed) {
    char set_speed_command[2] = { speed, train };
    Write(COM1, set_speed_command, sizeof(set_speed_command));
}

static void train_set_speed(int train, speed_t speed) {
    train_set_speed_internal(train, speed);
}

static void train_reverse(int train, speed_t new_speed) {
    // Stop the Train.
    train_set_speed_internal(train, 0);

    // Block for a while (1.5s) to let train stop.
    // TODO adjust this for speed
    Delay(150);

    // Reverse the Train.
    train_set_speed_internal(train, 15);

    // Reset the train to it's original speed.
    train_set_speed_internal(train, new_speed);
}

void train_task(int train_no) {

    tid_t mission_control_tid = MyParentTid();
    speed_t our_speed = 0;

    Message msg, reply;
    TrainMessage *tr_msg = &msg.tr_msg;
    TrainMessage *command = &reply.tr_msg;
    msg.type = TRAIN_MESSAGE;
    tr_msg->type = COMMAND_AWAITING;

    while (1) {
        Send(mission_control_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
        cuassert(TRAIN_MESSAGE == reply.type, "Train task received invalid msg");

        switch (command->type) {
        case (COMMAND_SET_SPEED):
            our_speed = command->speed;
            train_set_speed(train_no, command->speed);
            break;
        case (COMMAND_REVERSE):
            train_reverse(train_no, our_speed);
            break;
        }
    }

}
