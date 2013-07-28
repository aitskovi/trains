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
#include <location_server.h>
#include <constants.h>
#include <stack.h>

#define CRUISING_SPEED 11
#define D_STRAIGHT 0
#define D_CURVED 1
#define D_REVERSE 2
#define WAIT_TIME_FOR_RESERVED_TRACK 300
#define MAX_OCCUPIED_NODES 9
#define STOPPING_BUFFER 200000

#define min(a,b) (a) < (b) ? (a) : (b)

typedef struct {
    track_edge *edge;
    int distance;
    enum TRAIN_ORIENTATION orientation;
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

    // Time at which we failed to reserve piece of track
    // After a certain amount of time, try to recalculate a different path
    int reservation_failed_time;

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

static void publish_destination(TrainStatus *status) {
    static tid_t train_stream = -1;

    if (train_stream < 0) {
        train_stream = WhoIs("TrainStream");
    }

    Message msg;
    TrainMessage *tr_msg = &msg.tr_msg;
    msg.type = TRAIN_MESSAGE;

    tr_msg->type = COMMAND_GOTO;
    tr_msg->train = status->train_no;
    if (status->path_length) {
        tr_msg->destination = status->path[status->path_length - 1];
    } else {
        tr_msg->destination = 0;
    }
    Publish(train_stream, &msg);
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
    status->stopping_status = STOPPING_STOPPING;
    train_set_speed(status, 0);
}

static void train_start_reversing(TrainStatus *status) {
    status->saved_speed = status->speed;
    status->reversing_status = REVERSING_STOPPING;
    train_set_speed(status, 0);
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

static void determine_occupied_nodes(TrainStatus *status, track_node **result, int *num_nodes) {
    *num_nodes = 0;

    if (!status->position.edge) return;

    int distance_backward;
    if (status->position.orientation == TRAIN_FORWARD) distance_backward = PICKUP_BACK_TO_TRAIN_BACK_UM + PICKUP_LENGTH_UM;
    else distance_backward = PICKUP_LENGTH_UM + PICKUP_FRONT_TO_TRAIN_FRONT_UM;

    int distance_travelled = status->position.distance;
    track_node *landmark = status->position.edge->src;
    while(distance_travelled < distance_backward) {
        track_node *behind = track_previous_landmark(landmark);
        if (!behind) {
            break;
        }

        result[(*num_nodes)++] = behind;

        distance_travelled += distance_between_nodes(behind, landmark);
        landmark = behind;
    }

    // Our node.
    result[(*num_nodes)++] = status->position.edge->src;

    int distance_forward;
    if (status->position.orientation == TRAIN_FORWARD) distance_forward = PICKUP_FRONT_TO_TRAIN_FRONT_UM;
    else distance_forward = PICKUP_BACK_TO_TRAIN_BACK_UM;

    // Reserve behind/ahead of it if train pointed forward and it's necessary
    distance_travelled = status->position.edge->dist - status->position.distance;
    landmark = status->position.edge->src;
    while(distance_travelled < distance_forward) {
        track_node *ahead = track_next_landmark(landmark);
        if (!ahead) {
            break;
        }

        result[(*num_nodes)++] = ahead;

        distance_travelled += distance_between_nodes(landmark, ahead);
        landmark = ahead;
    }
}

static void reserve_current_position(TrainStatus *status) {
    track_node *occupied_nodes[MAX_OCCUPIED_NODES];
    int num_occupied_nodes;

    determine_occupied_nodes(status, occupied_nodes, &num_occupied_nodes);

    int j, error;
    for (j = 0; j < num_occupied_nodes; ++j) {
        track_node *node = occupied_nodes[j];
        error = Reserve(status->train_no, node);
        if (error == RESERVATION_ERROR || error == RESERVATION_FAILURE) {
            ulog("Train %u failed to reserve occupied track segment(%s)!", status->train_no, node->name);
        } else if (error == RESERVATION_SUCCESS) {
            circular_queue_push(&status->reserved_nodes, node);
        }
    }
}

static void train_reset_reserved_nodes(TrainStatus *status) {
    track_node *occupied_nodes[MAX_OCCUPIED_NODES];
    int num_occupied_nodes;
    int error;

    determine_occupied_nodes(status, occupied_nodes, &num_occupied_nodes);

    unsigned int j, k, should_release;
    for (j = circular_queue_size(&status->reserved_nodes); j > 0; j--) {
        track_node *node = (track_node *) circular_queue_pop(&status->reserved_nodes);
        should_release = 1;
        for (k = 0; k < num_occupied_nodes; ++k) {
            if (occupied_nodes[k] == node) {
                circular_queue_push(&status->reserved_nodes, node);
                should_release = 0;
                break;
            }
        }
        if (should_release) {
            error = Release(status->train_no, node);
            if (error != RESERVATION_SUCCESS) {
                ulog("Failed to release all reserved nodes!");
            }
        }
    }
}

static void train_reset(TrainStatus *status) {
    status->path_length = 0;
    status->path_reserved_distance = 0;
    status->path_pos = 0;
    status->path_reserved_pos = 0;
    status->reservation_failed_time = 0;
    status->reversing_status = REVERSING_NOT;
    status->stopping_status = STOPPING_NOT;

    int result;

    train_reset_reserved_nodes(status);

    if (circular_queue_empty(&status->reserved_nodes) && status->position.edge) {
        ulog("Train thinks it has no spots reserved after resetting");
    }
}

static void update_position(TrainStatus *status, LocationServerMessage *ls_msg) {
    status->old_position = status->position;
    status->position.distance = ls_msg->data.distance;
    status->position.edge = ls_msg->data.edge;
    status->position.orientation = ls_msg->data.orientation;
    status->velocity = ls_msg->data.velocity;
    status->stopping_distance = ls_msg->data.stopping_distance;

    if (REVERSING_WAITING_FOR_LOCATION == status->reversing_status) {
        status->reversing_status = REVERSING_NOT;
        ulog("Changing reserved nodes upon reverse");

        stack s;
        stack_initialize(&s);

        unsigned int j;
        for (j = circular_queue_size(&status->reserved_nodes); j > 0; j--) {
            stack_push(&s, circular_queue_pop(&status->reserved_nodes));
        }

        track_node *node, *swapped;
        while ((node = stack_pop(&s))) {
            swapped = SwapForReverse(status->train_no, node);
            if (!swapped) {
                ulog("Train %u failed to swap occupied track segment(%s)!", node->name);
            } else {
                circular_queue_push(&status->reserved_nodes, swapped);
            }
        }
    }

    if (status->position.edge) {
        if (status->position.edge != status->old_position.edge) {
            if (status->old_position.edge) {
                if (circular_queue_empty(&status->reserved_nodes)) {
                    ulog("Reserved nodes was empty when train %u was leaving segment after %s", status->train_no,
                            status->old_position.edge->src->name);
                    return;
                }
                track_node *node = circular_queue_peek(&status->reserved_nodes);

                if (!node) {
                    ulog("Somehow head of queue was null");
                    return;
                }

                // Release oldest reserved node if we're past it by at least train length
                if (is_node_ahead_of_node(node, status->position.edge->src)) {
                    int clearance = distance_between_nodes(node, status->position.edge->src) + status->position.distance;
                    if (clearance > TRAIN_LENGTH_UM) {
                        circular_queue_pop(&status->reserved_nodes);
                        int result = Release(status->train_no, node);
                        if (result != RESERVATION_SUCCESS) {
                            ulog("Failed to release %s on reserved queue while moving!", node->name);
                        }
                    }
                }
            }

            // TODO is this necessary?
            reserve_current_position(status);
        }
    }

    if (status->velocity == 0) {
        reserve_current_position(status);
        train_reset_reserved_nodes(status);
    }
}

static void recalculate_path(TrainStatus *status, int avoid_others) {
    if (!status->path_length) {
        ulog("Error: Recalculating non-existant path");
        return;
    }
    track_node *dest = status->path[status->path_length - 1];
    train_reset(status);

    track_node *occupied_nodes[MAX_OCCUPIED_NODES];
    int num_occupied_nodes;
    determine_occupied_nodes(status, occupied_nodes, &num_occupied_nodes);

    int result = calculate_path(status->train_no, occupied_nodes, num_occupied_nodes, avoid_others, status->position.edge->src, dest, status->path, &status->path_length);

    if (result) {
        ulog("Train %u could not find a path to %s from %s", status->train_no, status->position.edge->src->name, dest->name);
        train_reset(status);
        return;
    }

    publish_destination(status);
}

static int is_reverse_step(track_node *node1, track_node *node2) {
    track_node * next_landmark = track_next_landmark(node1);
    return node2 == next_landmark->reverse || node1 == node2->reverse;
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
        ulog("Train %u got lost %d um ahead of %s, resetting", status->train_no, status->position.distance, status->position.edge->src->name);
        ulog("Train %u at %s, expected %s, %s, %s", status->train_no, status->path[status->path_pos]->name, status->path[status->path_pos + 1]->name, status->path[status->path_pos + 2]->name, status->path[status->path_pos + 3]->name);
        train_set_speed(status, 0);
        train_reset(status);
//        recalculate_path(status, 0);
//       perform_path_actions(&status);
        return;
    }

    if (status->path_pos > status->path_reserved_pos) {
        status->path_reserved_pos = status->path_pos;
    }

    status->path_reserved_distance = calculate_path_reserved_distance(status);

    // Reserve up to us + stopping distance
    j = status->path_reserved_pos;
    while (1) {
        if (!status->reservation_failed_time
                && status->path_reserved_distance > status->stopping_distance + STOPPING_BUFFER) {
            break;
        }

        if (j >= status->path_length) {
            break;
        }

        track_node *current = status->path[j];
        track_node *previous = j > 0 ? status->path[j-1] : 0;

        // If we're reserving the final node, we have arrived
        if (j == status->path_length - 1 && status->path_reserved_distance <= status->stopping_distance) {
            ulog("Train arriving at %s while %d um ahead of %s", current->name, status->position.distance, status->position.edge->src->name);
            // We have arrived
            status->path_length = 0;
            train_start_stopping(status);
            return;
        } else if (j == status->path_length - 1) {
            return;
        }

        track_node *next    = status->path[j+1];

        int dist = distance_between_nodes(current, next);

        // We're reversing
        if (is_reverse_step(current, next)) {
            if (status->path_reserved_distance <= status->stopping_distance) {
                ulog("Train reversing at %s while %d um ahead of %s", current->name, status->position.distance, status->position.edge->src->name);
                status->path_reserved_pos++;
                status->path_reserved_distance = 0;
                train_start_reversing(status);
            }
            return;
        }

        /*

        // No need to reserve the section we're already on if it's the start of the path (this frequently causes us to get stuck)
        if (current == status->position.edge->src && current == status->path[0]) {
            status->path_reserved_distance += dist;
            status->path_reserved_pos++;
            ++j;
            continue;
        }

        */

        result = Reserve(status->train_no, current);
        if (result == RESERVATION_ERROR) {
            ulog ("Reservation error occurred");
            train_start_stopping(status);
            train_reset(status);
            return;
        } else if (result == RESERVATION_FAILURE) {
            if (!status->reservation_failed_time) {
                ulog ("Train %u: Reservation of %s failed, waiting %d ahead of %s, buffer is %d", status->train_no, current->name, status->position.distance, status->position.edge->src->name, status->path_reserved_distance);
                status->reservation_failed_time = Time();
                train_set_speed(status, 0);
                //train_start_stopping(status);
            }
            if (Time() > status->reservation_failed_time + WAIT_TIME_FOR_RESERVED_TRACK) {
                ulog ("Train %u: Timed out reserving %s, recalculating", status->train_no, current->name);
                recalculate_path(status, 1);
            }
            return;
        } else {
            status->reservation_failed_time = 0;
            ulog("Cruise speed set after train reserved node %s while %d um ahead of %s, buffer space is now %d um", current->name, status->position.distance, status->position.edge->src->name, status->path_reserved_distance);
            if (result == RESERVATION_SUCCESS) {
                circular_queue_push(&status->reserved_nodes, current);
            }
            train_set_speed(status, CRUISING_SPEED);
        }

        if (current->type == NODE_BRANCH) {
            int direction = direction_between_nodes(current, next);
            cuassert(direction >= 0, "Disconnected nodes in path!");

            if (D_STRAIGHT == direction) {
//                ulog("%s switched to straight", current->name);
                SetSwitch(current->num, STRAIGHT);
            } else if (D_CURVED == direction) {
//                ulog("%s switched to curved", current->name);
                SetSwitch(current->num, CURVED);
            } else {
//                ulog ("INVALID DIRECTION");
            }
        }

        // Switch merges as we drive over them to enable reverses
        if (previous && current->type == NODE_MERGE) {
            int direction = direction_between_nodes(current->reverse, previous->reverse);
            cuassert(direction >= 0, "Disconnected nodes in merge swapping!");

            if (D_STRAIGHT == direction) {
//                ulog("%s switched to straight", current->reverse->name);
                SetSwitch(current->reverse->num, STRAIGHT);
            } else if (D_CURVED == direction) {
//                ulog("%s switched to curved", current->reverse->name);
                SetSwitch(current->reverse->num, CURVED);
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
            train_reset_reserved_nodes(status);
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
            ulog("Train %u finished stopping %d ahead of %s", status->train_no, status->position.distance, status->position.edge->src->name);
            status->stopping_status = STOPPING_NOT;
            if (!status->path_length) {
                train_reset(status);
            }

            publish_destination(status);
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
                if (status.path_length > 0) {
                    ulog("Train ignoring goto");
                    break;
                }

                ulog("Train received command to goto %s from %s", tr_command->destination->name, status.position.edge->src->name);
                train_reset(&status);
                status.path_length = 1;
                status.path[0] = tr_command->destination;
                recalculate_path(&status, 0);
                perform_path_actions(&status);
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
