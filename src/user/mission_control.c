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

#define MAX_TRAINS 5

typedef struct TrainStatus {
    unsigned char train_no;
    tid_t tid;
    int awaiting_instruction; // Waiting
    int has_queued_instruction;
    TrainMessage queued_instruction;
    track_node *dest;
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

static void queue_train_command(TrainStatus *status, TrainMessage *message) {
    status->queued_instruction = *message;
    status->has_queued_instruction = 1;
}

static void flush_train_status(TrainStatus *status) {
    Message reply;
    TrainMessage *tr_reply = &reply.tr_msg;
    if (status->awaiting_instruction && status->has_queued_instruction) {
        *tr_reply = status->queued_instruction;
        reply.type = TRAIN_MESSAGE;
        //ulog("\nFlushed train status");
        Reply(status->tid, (char *) &reply, sizeof(reply));
        status->has_queued_instruction = 0;
        status->awaiting_instruction = 0;
    }
}

static void mission_control_set_train_speed(TrainStatus *status, unsigned char speed) {
    TrainMessage train_command;
    train_command.speed = speed;
    train_command.type = COMMAND_SET_SPEED;
    queue_train_command(status, &train_command);
    flush_train_status(status);
}

static void mission_control_reverse_train(TrainStatus *status) {
    TrainMessage train_command;
    train_command.type = COMMAND_REVERSE;
    status->position = status->position->reverse;
    queue_train_command(status, &train_command);
    flush_train_status(status);
}

static void mission_control_set_train_dest(TrainStatus *status, track_node *node) {
    status->dest = node;
}

void mission_control() {
    RegisterAs("MissionControl");

    tid_t tid, new_child;
    Message msg, reply;

    ShellMessage *sh_msg = &msg.sh_msg;
    ShellMessage *sh_reply = &reply.sh_msg;

    TrainMessage *tr_msg = &msg.tr_msg;
    TrainMessage *tr_reply = &reply.tr_msg;

    LocationServerMessage *ls_msg = &msg.ls_msg;
    LocationServerMessage *ls_reply = &reply.ls_msg;

    TrainStatus trains[MAX_TRAINS];
    unsigned int num_trains = 0;
    memset(trains, 0, sizeof(trains));

    TrainStatus *status;

    // Find the location server.
    tid_t location_server_tid;
    do {
        location_server_tid = WhoIs("LocationServer");
    } while (location_server_tid < 0);

    // Subscribe.
    location_server_subscribe(location_server_tid);

    track_node *B4;

    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));
        switch (msg.type) {

        /*
         *  Commands from shell
         */
        case SHELL_MESSAGE:

            switch (sh_msg->type) {
                case SHELL_INIT_TRACK:
                    track_initialize(sh_msg->track);

                    B4 = track_get_by_name("B4");
                    ulog("\nMission control B4 was %s", B4->name);

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
                    mission_control_set_train_speed(status, 2);
                    AddTrain(sh_msg->train_no);
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
                    if (!status->position) {
                        ulog("\nCould not find train position");
                        break;
                    }
                    if (configure_track_for_path(status->position->src, sh_msg->position)) {
                        ulog("\nCould not find forward path to destination");
                        mission_control_reverse_train(status);
                        cuassert(!configure_track_for_path(status->position->src, sh_msg->position), "Could not find path either way!!!");
                        break;
                    }
                    mission_control_set_train_dest(status, sh_msg->position);
                    mission_control_set_train_speed(status, 11);

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
            //ulog("\nGot train awaiting command");
            cuassert(COMMAND_AWAITING == tr_msg->type, "Mission control received invalid message from train");
            status = train_status_by_tid(trains, tid);
            status->awaiting_instruction = 1;
            flush_train_status(status);
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

            if (ls_msg->data.edge && ls_msg->data.edge->dest == status->dest) {
                mission_control_set_train_speed(status, 0);
            }

            reply.type = LOCATION_SERVER_MESSAGE;
            ls_reply->type = LOCATION_COURIER_RESPONSE;

            Reply(tid, (char *) &reply, sizeof(reply));

            break;



        default:
            ulog("Message type is %d", msg.type);
            cuassert(0, "Mission control received unknown message");
            break;
        }
    }

}
