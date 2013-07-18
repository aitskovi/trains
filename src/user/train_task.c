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
#include <pubsub.h>
#include <reservation_server.h>
#include <circular_queue.h>

#define CRUISING_SPEED 11
#define D_STRAIGHT 0
#define D_CURVED 1
#define D_REVERSE 2
#define FAILED_RESERVATIONS_BEFORE_RETRY 3000

#define min(a,b) (a) < (b) ? (a) : (b)

typedef struct {
    track_edge *edge;
    int distance;
} Position;

typedef struct {
    // The train number
    train_t train_no;

    // The current position and speed of the train
    Position position;
    speed_t speed;
    int velocity;
    int stopping_distance;

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

    // Stopping flags
    enum {
        STOPPING_NOT,
        STOPPING_STOPPING
    } stopping_status;

    // Speed to return to after reversing is completed
    speed_t saved_speed;

    // Number of times we've tried to reserve the same node and failed
    // When this gets high enough we try a different path
    unsigned int failed_reservations;

    // Reserved nodes
    struct circular_queue reserved_nodes;
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
    if (speed != status->speed) {
        char set_speed_command[2] = { speed, status->train_no };
        Write(COM1, set_speed_command, sizeof(set_speed_command));
        status->speed = speed;
        notify_speed_change(status->train_no, speed, server_tid);
    }
}

static void train_start_stopping(TrainStatus *status) {
    if (status->velocity != 0) {
        status->stopping_status = STOPPING_STOPPING;
        train_set_speed(status, 0);
    }
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
        result += thing;
        ++i;
    }
    return result;
}


static void reserve_current_position(TrainStatus *status) {
    int result;
    result = Reserve(status->train_no, status->position.edge->src);
    if (result == RESERVATION_ERROR || result == RESERVATION_FAILURE) {
        ulog("Failed to reserve track segment that train is on (first position update)!");
    } else if (result == RESERVATION_SUCCESS) {
        circular_queue_push(&status->reserved_nodes, status->position.edge->src);
    }
}

static void train_reset(TrainStatus *status) {
    status->path_length = 0;
    status->path_reserved_distance = 0;
    status->path_pos = 0;
    status->path_reserved_pos = 0;
    status->failed_reservations = 0;
    status->reversing_status = REVERSING_NOT;
    status->stopping_status = STOPPING_NOT;

    int result;

    unsigned int j;
    for (j = circular_queue_size(&status->reserved_nodes); j > 0; j--) {
        track_node *node = (track_node *) circular_queue_pop(&status->reserved_nodes);
        if (status->position.edge && node == status->position.edge->src) {
            circular_queue_push(&status->reserved_nodes, node);
        } else {
            result = Release(status->train_no, node);
            if (result != RESERVATION_SUCCESS) {
                ulog("Failed to release track segments on reserved queue while resetting!");
            }
        }
    }

    if (circular_queue_empty(&status->reserved_nodes) && status->position.edge) {
        ulog("Train thinks it has no spots reserved after resetting");
        reserve_current_position(status);
    }
}

static void update_position(TrainStatus *status, LocationServerMessage *ls_msg) {
    status->old_position = status->position;
    status->position.distance = ls_msg->data.distance;
    status->position.edge = ls_msg->data.edge;
    status->velocity = ls_msg->data.velocity;
    status->stopping_distance = ls_msg->data.stopping_distance;

    if (REVERSING_WAITING_FOR_LOCATION == status->reversing_status) {
        status->reversing_status = REVERSING_NOT;
    }

    if (status->position.edge) {
        if (status->position.edge != status->old_position.edge) {
            if (status->old_position.edge) {

                if (status->old_position.edge->src->owner != status->train_no) {
                    ulog("Train %u left a track segment (%s) that didn't belong to it!", status->train_no,
                            status->old_position.edge->src->name);
                    return;
                }
                if (circular_queue_empty(&status->reserved_nodes)) {
                    ulog("Reserved nodes was empty when train %u was leaving segment after %s", status->train_no,
                            status->old_position.edge->src->name);
                    return;
                }
                track_node *node = circular_queue_pop(&status->reserved_nodes);
                if (node != status->old_position.edge->src) {
                    ulog("Train %u: Leaving track segment (%s) which is not same as head of queue (%s)",
                            status->train_no, status->old_position.edge->src->name, node->name);
                }
                int result = Release(status->train_no, node);
                if (result != RESERVATION_SUCCESS) {
                    ulog("Failed to release %s on reserved queue while moving!", node->name);
                }
            }

            reserve_current_position(status);
        }
    }
}

static void recalculate_path(TrainStatus *status) {
    if (!status->path_length) {
        ulog("Error: Recalculating non-existant path");
        return;
    }
    track_node *dest = status->path[status->path_length - 1];
    train_reset(status);
    int result = calculate_path(status->position.edge->src, dest, status->path, &status->path_length);
    if (result) {
        ulog("Train %u could not find a path to %s from %s", status->train_no, status->position.edge->src->name, dest->name);
        train_reset(status);
    }
}

static void perform_path_actions(TrainStatus *status) {
    int result;

    // To do if we're not en route somewhere
    if (!status->path_length) {
        return;
    }

    // Wait for reversing to be over before we do anything else
    if (REVERSING_NOT != status->reversing_status) {
        return;
    }

    // Wait for stopping to be over
    if (STOPPING_NOT != status->stopping_status) {
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
        ulog("Train %u got lost %d um ahead of %s, recalculating", status->train_no, status->position.distance, status->position.edge->src->name);
        train_set_speed(status, 0);
        recalculate_path(status);
        return;
    }

    status->path_reserved_distance = calculate_path_reserved_distance(status);

    if (status->path_reserved_distance > status->stopping_distance + 50000) {
//        train_set_speed(status, CRUISING_SPEED);
        return;
    }

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

        // If we're reserving the final node, we have arrived
        if (j == status->path_length - 1) {
            ulog("Train arriving at %s while %d um ahead of %s", current->name, status->position.distance, status->position.edge->src->name);
            // We have arrived
            status->path_length = 0;
            train_start_stopping(status);
            return;
        }

        track_node *next    = status->path[j+1];

        int dist = distance_between_nodes(current, next);
        cuassert(dist >= 0, "Disconnected nodes in path!");

        result = Reserve(status->train_no, current);
        if (result == RESERVATION_ERROR) {
            ulog ("Reservation error occurred");
            train_start_stopping(status);
            train_reset(status);
            return;
        } else if (result == RESERVATION_FAILURE) {
            if (!status->failed_reservations) {
                ulog ("Train %u: Reservation of %s failed, waiting", status->train_no, current->name);
                train_start_stopping(status);
            }
            status->failed_reservations++;
            if (status->failed_reservations > FAILED_RESERVATIONS_BEFORE_RETRY) {
                ulog ("Train %u: Timed out reserving %s, recalculating", status->train_no, current->name);
                recalculate_path(status);
            }
            return;
        } else {
            status->failed_reservations = 0;
            ulog("Train reserved node %s while %d um ahead of %s, buffer space is now %d um", current->name, status->position.distance, status->position.edge->src->name, status->path_reserved_distance);
            if (result == RESERVATION_SUCCESS) {
                circular_queue_push(&status->reserved_nodes, current);
            }
            train_set_speed(status, CRUISING_SPEED);
        }

        // We're reversing
        if (dist == 0) {
            ulog("Train reversing at %s while %d um ahead of %s", current->name, status->position.distance, status->position.edge->src->name);
            status->path_reserved_pos++;
            status->path_reserved_distance = 0;
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

static void perform_stopping_actions(TrainStatus *status) {
    switch (status->stopping_status) {
    case STOPPING_NOT:
        return;
    case STOPPING_STOPPING:
        if (status->velocity == 0) {
            status->stopping_status = STOPPING_NOT;
            if (!status->path_length) {
                train_reset(status);
            }
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
    circular_queue_initialize(&status.reserved_nodes);

    train_reset(&status);

    // Subscribe.
    Subscribe("LocationServerStream", PUBSUB_HIGH);

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
            // Unblock caller right away.
            reply.type = TRAIN_MESSAGE;
            tr_reply->type = COMMAND_ACKNOWLEDGED;
            Reply(tid, (char *) &reply, sizeof(reply));

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
                train_reset(&status);
                int result = calculate_path(status.position.edge->src, tr_command->destination, status.path, &status.path_length);
                if (result) {
                    ulog("Could not calculate path");
                    status.path_length = 0;
                } else {
                    perform_path_actions(&status);
                }
                break;
            case (COMMAND_STOP):
                ulog("Train received command to stop at %s", tr_command->destination->name);
                status.stop_sensor = tr_command->destination;
                break;
            case (COMMAND_RESET):
                ulog("Train received command to reset");
                train_reset(&status);
                train_set_speed(&status, 0);
                break;
            default:
                cuassert(0, "Invalid Train Task Command");
                break;
            }

            break;

        case LOCATION_SERVER_MESSAGE:
            // Unblock the courier right away.
            reply.type = LOCATION_SERVER_MESSAGE;
            ls_reply->type = LOCATION_COURIER_RESPONSE;
            Reply(tid, (char *) &reply, sizeof(reply));

            if (ls_msg->data.id == status.train_no) {
                ls_msg->data.distance = min(ls_msg->data.distance, ls_msg->data.edge->dist);
                update_position(&status, ls_msg);
            }

            // TODO remove this when no longer necessary

            perform_stop_action(&status);
            perform_reversing_actions(&status);
            perform_stopping_actions(&status);
            perform_path_actions(&status);

            break;

        }

    }
}
