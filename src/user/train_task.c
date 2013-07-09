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
#include <track_data.h>
#include <switch_server.h>

#define CRUISING_SPEED 11
#define D_STRAIGHT 0
#define D_CURVED 1
#define D_REVERSE 2

#define min(a,b) (a) < (b) ? (a) : (b)

typedef struct {
    track_edge *edge;
    unsigned int distance;
} Position;

typedef struct {
    // The train number
    train_t train_no;

    // The current position and speed of the train
    Position position;
    speed_t speed;
    int velocity;
    unsigned int stopping_distance;

    // The previous position of the train
    Position old_position;

    // The path the train is following
    track_node *path[TRACK_MAX];
    unsigned int path_length;
    // The furthest index in the path array that we have reached
    unsigned int path_pos;
    // The closest index in the path array that we have not yet reserved
    unsigned int path_reserved_pos;
    // The amount of track ahead of us, in um, that we have reserved
    int path_reserved_distance;

    // If this is set, the train will immediately stop when the sensor is triggered (used for stopping distance calibration)
    track_node *stop_sensor;

    // Reversing flags
    enum {
        REVERSING_NOT,
        REVERSING_STOPPING,
        REVERSING_WAITING_FOR_LOCATION,
    } reversing_status;

    // Speed to return to after reversing is completed
    speed_t saved_speed;
} TrainStatus;

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

static void train_set_speed(TrainStatus *status, speed_t speed) {
    static tid_t server_tid = -2;
    if (server_tid < 0) {
        server_tid = WhoIs("LocationServer");
    }
    char set_speed_command[2] = { speed, status->train_no };
    Write(COM1, set_speed_command, sizeof(set_speed_command));
    status->speed = speed;
    notify_speed_change(status->train_no, speed, server_tid);
}

static void train_start_reversing(TrainStatus *status) {
    status->saved_speed = status->speed;
    status->reversing_status = REVERSING_STOPPING;
    train_set_speed(status, 0);
}

static int distance_between_nodes (track_node *node1, track_node *node2) {
    if (!node1 || !node2) {
        ulog("distance_between_nodes called with null pointer");
        return -2;
    }

    int result = -1;
    unsigned int j;
    for (j = 0; j < NUM_NODE_EDGES[node1->type]; ++j) {
        if (node1->edge[j].dest == node2) {
            result = node1->edge[j].dist;
            break;
        }
    }
    if (node1->reverse == node2) {
        return 0;
    }

    return result;
}

static int direction_between_nodes(track_node *node1, track_node *node2) {
    if (!node1 || !node2) {
        ulog("direction_between_nodes called with null pointer");
        return 0;
    }

    int result = -1;
    unsigned int j;
    for (j = 0; j < NUM_NODE_EDGES[node1->type]; ++j) {
        if (node1->edge[j].dest == node2) {
            result = j;
            break;
        }
    }
    /*
    if (node1->reverse == node2) {
        result = D_REVERSE;
    }
    */
    return result;
}

static unsigned int distance_travelled(Position *position1, Position *position2) {
    track_edge *edge1 = position1->edge;
    track_edge *edge2 = position2->edge;
    unsigned int distance1 = position1->distance;
    unsigned int distance2 = position2->distance;

    if (edge1 == edge2) {
        return distance2 - distance1;
    }

    unsigned int j;
    for (j = 0; j < NUM_NODE_EDGES[edge1->dest->type]; ++j) {
        if (&edge1->dest->edge[j] == edge2) {
            return (edge1->dist - distance1) + distance2;
        }
    }

    ulog("\nCould not calculate distance travelled");
    return 0;
}

static int calculate_path_reserved_distance(TrainStatus *status) {
    int result = 0;
    result -= status->position.distance;
    unsigned int i = status->path_pos;
    while (1) {
        if (status->path_reserved_pos < status->path_pos) {
            return 0;
        }

        if (i >= status->path_length) {
            break;
        }

        if (i >= status->path_reserved_pos) {
            break;
        }

        track_node *current = status->path[i];
        track_node *next    = status->path[i+1];

        int thing = distance_between_nodes(current, next);
        if (thing == -2) {
            ulog("Failed in path_reserved_distance i: %u, current: %x, next: %x, path_length: %u", i, current, next, status->path_length);
            return 10000000;
        }
        result += thing;
        ++i;
    }
    return result;
}

static void reset_train(TrainStatus *status) {
    status->path_length = 0;
    status->path_reserved_distance = 0;
    status->path_pos = 0;
    status->path_reserved_pos = 0;
    status->reversing_status = REVERSING_NOT;
//    train_set_speed(status, 0);
}

static void update_position(TrainStatus *status, LocationServerMessage *ls_msg) {
    status->old_position = status->position;
    status->position.distance = ls_msg->data.distance;
    status->position.edge = ls_msg->data.edge;
    status->velocity = ls_msg->data.velocity;
    status->stopping_distance = ls_msg->data.stopping_distance;

    if (REVERSING_WAITING_FOR_LOCATION == status->reversing_status) {
        status->reversing_status = REVERSING_NOT;
        /*
        if (status->path_length) {
            track_node *dest = status->path[status->path_length - 1];
            reset_train(status);
            calculate_path(status->position.edge->src, dest, status->path, &status->path_length);
            train_set_speed(status, CRUISING_SPEED);
        }
        */
    }
}

static void perform_path_actions(TrainStatus *status) {
    // To do if we're not en route somewhere
    if (!status->path_length) {
        return;
    }

    // Wait for reversing to be over before we do anything else
    if (REVERSING_NOT != status->reversing_status) {
        return;
    }

    // Figure out where we are in the path
    unsigned int i, j;
    int found = 0;
    for (i = status->path_pos; i < status->path_length; ++i) {
        if (status->position.edge->src == status->path[i]) {
            status->path_pos = i;
            found = 1;
            break;
        }
    }

    if (!found) {
        ulog("Train %u got lost %u um ahead of %s, stopping", status->train_no, status->position.distance, status->position.edge->src->name);
        track_node *dest = status->path[status->path_length - 1];
        reset_train(status);
        train_set_speed(status, 0);
        calculate_path(status->position.edge->src, dest, status->path, &status->path_length);
        train_set_speed(status, CRUISING_SPEED);
        return;
    }

    status->path_reserved_distance = calculate_path_reserved_distance(status);

    // Reserve up to us + stopping distance
    j = status->path_reserved_pos;
    while (1) {
        if (status->path_reserved_distance > status->stopping_distance) {
            break;
        }

        if (j >= status->path_length) {
            break;
        }

        track_node *current = status->path[j];

        if (j == status->path_length - 1) {
            ulog("Train arriving at %s while %u um ahead of %s", current->name, status->position.distance, status->position.edge->src->name);
            // We have arrived
            reset_train(status);
            train_set_speed(status, 0);
            return;
        }

        track_node *next    = status->path[j+1];


        int dist = distance_between_nodes(current, next);
        if (dist == -2) {
            ulog("Failed in perform path actions j: %u, current: %x, next: %x, path_length: %u", j, current, next, status->path_length);
            return;
        }
        cuassert(dist >= 0, "Disconnected nodes in path!");

        ulog("Train reserved node %s while %u um ahead of %s, buffer space is now %u um", current->name, status->position.distance, status->position.edge->src->name, status->path_reserved_distance);

        // We're reversing
        if (dist == 0) {
            ulog("Train reversing at %s while %u um ahead of %s", current->name, status->position.distance, status->position.edge->src->name);
            train_start_reversing(status);
            return;
        }

        if (current->type == NODE_BRANCH) {
            int direction = direction_between_nodes(current, next);
            cuassert(direction >= 0, "Disconnected nodes in path!");

            if (D_STRAIGHT == direction) {
                ulog("%s switched to straight", current->name);
                SetSwitch(current->num, STRAIGHT);
            } else if (D_CURVED == direction) {
                ulog("%s switched to curved", current->name);
                SetSwitch(current->num, CURVED);
            } else {
                ulog ("INVALID DIRECTION");
            }
        }

        status->path_reserved_distance += dist;
        status->path_reserved_pos++;
        ++j;
    }
}

static void perform_reversing_actions(TrainStatus *status) {
    switch (status->reversing_status) {
    case REVERSING_NOT:
    case REVERSING_WAITING_FOR_LOCATION:
        return;
    case REVERSING_STOPPING:
        if (status->velocity == 0) {
            status->reversing_status = REVERSING_WAITING_FOR_LOCATION;
            train_set_speed(status, 15);
            train_set_speed(status, status->saved_speed);
        }
        return;
    }
}

static void perform_stop_action(TrainStatus *status) {
    if (status->position.edge != status->old_position.edge && status->stop_sensor && status->position.edge->src == status->stop_sensor) {
        train_set_speed(status, 0);
    }
}

void train_task(int train_no) {
    TrainStatus status;
    memset(&status, 0, sizeof(TrainStatus));
    status.train_no = train_no;

    reset_train(&status);

    // Subscribe.
    tid_t location_server_tid = WhoIs("LocationServer");
    location_server_subscribe(location_server_tid);

    tid_t tid;
    Message msg, reply;
    TrainMessage *tr_command = &msg.tr_msg;
    TrainMessage *tr_reply = &reply.tr_msg;
    LocationServerMessage *ls_msg = &msg.ls_msg;
    LocationServerMessage *ls_reply = &reply.ls_msg;

    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch (msg.type) {

        case TRAIN_MESSAGE:

            switch (tr_command->type) {
            case (COMMAND_SET_SPEED):
                train_set_speed(&status, tr_command->speed);
                break;
            case (COMMAND_REVERSE):
                if (status.path_length) {
                    ulog("Cannot reverse: train is following a path");
                    break;
                }
                train_start_reversing(&status);
                break;
            case (COMMAND_GOTO):
                ulog("Train received command to goto %s from %s", tr_command->destination->name, status.position.edge->src->name);
                reset_train(&status);
                int result = calculate_path(status.position.edge->src, tr_command->destination, status.path, &status.path_length);
                if (result) {
                    ulog("Could not calculate path");
                    status.path_length = 0;
                } else {
                    train_set_speed(&status, CRUISING_SPEED);
                }
                break;
            case (COMMAND_STOP):
                ulog("Train received command to stop at %s", tr_command->destination->name);
                status.stop_sensor = tr_command->destination;
                break;
            default:
                cuassert(0, "Invalid Train Task Command");
                break;
            }

            reply.type = TRAIN_MESSAGE;
            tr_reply->type = COMMAND_ACKNOWLEDGED;
            Reply(tid, (char *) &reply, sizeof(reply));
            break;

        case LOCATION_SERVER_MESSAGE:

            // TODO remove this when no longer necessary
            ls_msg->data.distance = min(ls_msg->data.distance, ls_msg->data.edge->dist);

            update_position(&status, ls_msg);
            perform_stop_action(&status);
            perform_reversing_actions(&status);
            perform_path_actions(&status);

            reply.type = LOCATION_SERVER_MESSAGE;
            ls_reply->type = LOCATION_COURIER_RESPONSE;
            Reply(tid, (char *) &reply, sizeof(reply));

            break;

        }

    }
}
