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
#include <nameserver.h>
#include <track.h>
#include <memory.h>

#define MAX_TRAINS 5

typedef struct TrainStatus {
    unsigned char train_no;
    tid_t tid;
    int awaiting_instruction; // Waiting
    int has_queued_instruction;
    TrainMessage queued_instruction;
    track_node *dest, *stop;
    track_edge *position;
    unsigned int dist;
} TrainStatus;

void train_status_init(TrainStatus *status, unsigned char train_no, tid_t tid) {
    status->train_no = train_no;
    status->awaiting_instruction = 0;
    status->has_queued_instruction = 0;
    status->tid = tid;
    status->position = 0;
    status->dist = 0;
    status->dest = 0;
    status->stop = 0;
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

static void mission_control_set_train_speed(TrainStatus *status, unsigned char speed) {
    // TODO store speed in train status struct?
    ulog("Mission control setting train speed for train %u to %u", (unsigned int) status->train_no, speed);

    Message msg, reply;
    msg.type = TRAIN_MESSAGE;
    TrainMessage *train_command = &msg.tr_msg;
    train_command->speed = speed;
    train_command->type = COMMAND_SET_SPEED;
    Send(status->tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    cuassert(reply.type == TRAIN_MESSAGE, "Unexpected reply from train task");
    cuassert(reply.tr_msg.type == COMMAND_ACKNOWLEDGED, "Unexpected reply from train task");
}

static void mission_control_reverse_train(TrainStatus *status) {
    status->position = status->position->reverse;

    Message msg, reply;
    msg.type = TRAIN_MESSAGE;
    TrainMessage *train_command = &msg.tr_msg;
    train_command->type = COMMAND_REVERSE;
    Send(status->tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    cuassert(reply.type == TRAIN_MESSAGE, "Unexpected reply from train task");
    cuassert(reply.tr_msg.type == COMMAND_ACKNOWLEDGED, "Unexpected reply from train task");
}

static void mission_control_set_train_dest(TrainStatus *status, track_node *node) {
    status->dest = node;

    Message msg, reply;
    msg.type = TRAIN_MESSAGE;
    TrainMessage *train_command = &msg.tr_msg;
    train_command->type = COMMAND_GOTO;
    train_command->destination = node;
    Send(status->tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    cuassert(reply.type == TRAIN_MESSAGE, "Unexpected reply from train task");
    cuassert(reply.tr_msg.type == COMMAND_ACKNOWLEDGED, "Unexpected reply from train task");
}

static void mission_control_set_train_stop(TrainStatus *status, track_node *node) {
    status->stop = node;

    Message msg, reply;
    msg.type = TRAIN_MESSAGE;
    TrainMessage *train_command = &msg.tr_msg;
    train_command->type = COMMAND_STOP;
    train_command->destination = node;
    Send(status->tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    cuassert(reply.type == TRAIN_MESSAGE, "Unexpected reply from train task");
    cuassert(reply.tr_msg.type == COMMAND_ACKNOWLEDGED, "Unexpected reply from train task");
}

void mission_control() {
    RegisterAs("MissionControl");

    tid_t tid, new_child;
    Message msg, reply;

    ShellMessage *sh_msg = &msg.sh_msg;
    ShellMessage *sh_reply = &reply.sh_msg;

    LocationServerMessage *ls_msg = &msg.ls_msg;
    LocationServerMessage *ls_reply = &reply.ls_msg;

    TrainStatus trains[MAX_TRAINS];
    unsigned int num_trains = 0;
    memset(trains, 0, sizeof(trains));

    TrainStatus *status;

    tid_t location_server_tid = WhoIs("LocationServer");

    // Subscribe.
    location_server_subscribe(location_server_tid);

    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));
        //ulog("Src is %u", tid);
        switch (msg.type) {

        /*
         *  Commands from shell
         */
        case SHELL_MESSAGE:

            switch (sh_msg->type) {
                case SHELL_INIT_TRACK:
                    track_initialize(sh_msg->track);

                    int i;
                    for (i = 1; i < 19; ++i) {
                        SetSwitch(i, STRAIGHT);
                    }
                    SetSwitch(8, CURVED);
                    SetSwitch(11, CURVED);
                    SetSwitch(153, STRAIGHT);
                    SetSwitch(154, STRAIGHT);
                    SetSwitch(155, STRAIGHT);
                    SetSwitch(156, STRAIGHT);

                    break;
                case SHELL_ADD_TRAIN:
                    new_child = Execute(HIGH, train_task, sh_msg->train_no);
                    status = &trains[num_trains++];
                    train_status_init(status, sh_msg->train_no, new_child);
                    AddTrain(sh_msg->train_no);
                    mission_control_set_train_speed(status, 2);
                    break;
                case SHELL_SET_TRAIN_SPEED:
                    status = train_status_by_number(trains, sh_msg->train_no);
                    mission_control_set_train_speed(status, sh_msg->speed);
                    break;
                case SHELL_SET_SWITCH_POSITION:
                    SetSwitch(sh_msg->switch_no, sh_msg->switch_pos);
                    break;

                case SHELL_REVERSE_TRAIN:
                    status = train_status_by_number(trains, sh_msg->train_no);
                    mission_control_reverse_train(status);
                    break;

                case SHELL_GO:
                    status = train_status_by_number(trains, sh_msg->train_no);
                    if (!status) {
                        ulog("\nCould not find train");
                        break;
                    }
                    mission_control_set_train_dest(status, sh_msg->position);

                    break;

                case SHELL_STOP:
                    status = train_status_by_number(trains, sh_msg->train_no);
                    if (!status) {
                        ulog("\nCould not find train");
                        break;
                    }
                    mission_control_set_train_stop(status, sh_msg->position);

                    break;

                default:
                    break;
            }

            reply.type = SHELL_MESSAGE;
            sh_reply->type = SHELL_SUCCESS_REPLY;
            Reply(tid, (char *) &reply, sizeof(reply));

            break;

        /*
         * TODO Location update
         */
        case LOCATION_SERVER_MESSAGE:
            cuassert(ls_msg->type == LOCATION_COURIER_REQUEST, "Mission control received invalid location server request");

            status = train_status_by_number(trains, ls_msg->data.id);

            if(!status){
                ulog("\nGot location for non-existant train!");
                break;
            }

            if (ls_msg->data.edge && !status->position) {
                mission_control_set_train_speed(status, 0);
                status->position = ls_msg->data.edge;
                status->dist = ls_msg->data.distance;
            }

            reply.type = LOCATION_SERVER_MESSAGE;
            ls_reply->type = LOCATION_COURIER_RESPONSE;
            Reply(tid, (char *) &reply, sizeof(reply));

            break;

        default:
            ulog("Src is %u", tid);
            ulog("Message type is %d", msg.type);
            cuassert(0, "Mission control received unknown message");
            break;
        }
    }

}
