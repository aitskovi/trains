/*
 * mission_control.c
 *
 *  Created on: Jun 30, 2013
 *      Author: aianus
 */

#include <encoding.h>
#include <shell.h>
#include <dassert.h>
#include <syscall.h>
#include <train_task.h>

#define MAX_TRAINS 5

typedef struct TrainStatus {
    unsigned char train_no;
    tid_t tid;
    int awaiting_instruction; // Waiting
    int has_queued_instruction;
    TrainMessage queued_instruction;
} TrainStatus;

void train_status_init(TrainStatus *status, unsigned char train_no, tid_t tid) {
    status->train_no = train_no;
    status->awaiting_instruction = 0;
    status->has_queued_instruction = 0;
    status->tid = tid;
}

TrainStatus * train_status_by_number(TrainStatus *status, unsigned char train_no) {
    unsigned int i;
    for (i = 0; i < MAX_TRAINS; ++i) {
        if (status->train_no == train_no) {
            return status;
        }
        status++;
    }
    return 0;
}

TrainStatus * train_status_by_tid(TrainStatus *status, tid_t tid) {
    unsigned int i;
    for (i = 0; i < MAX_TRAINS; ++i) {
        if (status->tid == tid) {
            return status;
        }
        status++;
    }
    return 0;
}

void mission_control() {
    RegisterAs("MissionControl");

    tid_t tid;
    Message msg, reply;

    ShellMessage *sh_msg = &msg.sh_msg;
    ShellMessage *sh_reply = &reply.sh_msg;

    TrainMessage *tr_msg = &msg.sh_msg;
    TrainMessage *tr_reply = &reply.sh_msg;

    TrainMessage *queued_instruction;

    TrainStatus trains[MAX_TRAINS];
    unsigned int num_trains = 0;

    TrainStatus *status;

    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));
        switch (msg.type) {

        /*
         *  Commands from shell
         */
        case SHELL_MESSAGE:

            switch (sh_msg->type) {
                case INIT_TRACK:
                    ulog("\nMission control initializing track");
                    track_initialize(sh_msg->track);
                    break;
                case ADD_TRAIN:
                    AddTrain(sh_msg->train_no);
                    tid_t new_child = Execute(HIGH, train_task, sh_msg->train_no);
                    train_status_init(&trains[num_trains++], sh_msg->train_no, new_child);
                    break;
                case SET_TRAIN_SPEED:
                    status = train_status_by_number(trains, sh_msg->train_no);
                    queued_instruction = &status->queued_instruction;
                    queued_instruction->speed = sh_msg->speed;
                    queued_instruction->type = COMMAND_SET_SPEED;
                    status->has_queued_instruction = 1;

                    if (status->awaiting_instruction) {
                        *tr_reply = *queued_instruction;
                        reply.type = TRAIN_MESSAGE;
                        Reply(status->tid, (char *) &reply, sizeof(reply));
                        status->has_queued_instruction = 0;
                    }

                    break;
                case SET_SWITCH_POSITION:
                    SetSwitch(sh_msg->switch_no, sh_msg->switch_pos);
                    break;

                case REVERSE_TRAIN:
                    status = train_status_by_number(trains, sh_msg->train_no);
                    queued_instruction = &status->queued_instruction;
                    queued_instruction->type = COMMAND_REVERSE;
                    status->has_queued_instruction = 1;

                    if (status->awaiting_instruction) {
                        *tr_reply = *queued_instruction;
                        reply.type = TRAIN_MESSAGE;
                        Reply(status->tid, (char *) &reply, sizeof(reply));
                        status->has_queued_instruction = 0;
                    }
                    break;

                default:
                    break;
            }

            reply.type = SHELL_MESSAGE;
            sh_reply->type = SHELL_SUCCESS_REPLY;
            Reply(tid, (char *) &reply, sizeof(reply));

            break;

        /*
         * Train is waiting for command
         */
        case TRAIN_MESSAGE:
            cuassert(COMMAND_AWAITING == tr_msg->type, "Mission control received invalid message from train");
            status = train_status_by_tid(trains, tid);

            if (status->has_queued_instruction) {
                reply.type = TRAIN_MESSAGE;
                reply.tr_msg = status->queued_instruction;
                Reply(status->tid, (char *) &reply, sizeof(reply));
                status->has_queued_instruction = 0;
            } else {
                status->awaiting_instruction = 1;
            }

            break;

        /*
         * TODO Location update
         */
        }
    }

}
