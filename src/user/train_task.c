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
    Delay(300);

    // Reverse the Train.
    train_set_speed(train, 15, location_server);

    // Reset the train to it's original speed.
    train_set_speed(train, new_speed, location_server);
}

static int distance_between_nodes (track_node *node1, track_node *node2) {
    int result = -1;
    unsigned int j;
    for (j = 0; j < NUM_NODE_EDGES[node1->type]; ++j) {
        if (node1->edge[j].dest == node2) {
            result = node1->edge[j].dist;
            break;
        }
    }
    /*
    if (node1->reverse == node2) {
        result = 0;
    }
    */
    return result;
}

static int direction_between_nodes(track_node *node1, track_node *node2) {
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

static unsigned int distance_travelled(track_edge *edge1, unsigned int distance1, track_edge *edge2, unsigned int distance2) {
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
}

void train_task(int train_no) {

    // Find Mission Control Tid.
    tid_t mission_control_tid = MyParentTid();
    tid_t location_server_tid = WhoIs("LocationServer");


    // Subscribe.
    location_server_subscribe(location_server_tid);

    speed_t our_speed = 0;

    tid_t tid;
    Message msg, reply;
    TrainMessage *tr_command = &msg.tr_msg;
    TrainMessage *tr_reply = &reply.tr_msg;
    LocationServerMessage *ls_msg = &msg.ls_msg;
    LocationServerMessage *ls_reply = &reply.ls_msg;

    track_node *path[TRACK_MAX];
    unsigned int path_length = 0;
    unsigned int path_pos = 0;
    unsigned int path_reserved_pos = 0;
    int path_reserved_distance = 0;

    track_edge *our_position, *old_position;
    unsigned int our_position_distance, old_position_distance;

    track_node *stop_sensor = 0;

    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));

        switch (msg.type) {

        case TRAIN_MESSAGE:

            switch (tr_command->type) {
            case (COMMAND_SET_SPEED):
                our_speed = tr_command->speed;
                train_set_speed(train_no, tr_command->speed, location_server_tid);
                break;
            case (COMMAND_REVERSE):
                train_reverse(train_no, our_speed, location_server_tid);
                break;
            case (COMMAND_GOTO):
                ulog("Train received command to goto %s", tr_command->destination->name);
                path_reserved_pos = path_pos = 0;
                path_reserved_distance = 0;
                // TODO calculate in a different thread
                calculate_path(our_position->src, tr_command->destination, path, &path_length);
                train_set_speed(train_no, CRUISING_SPEED, location_server_tid);
                break;
            case (COMMAND_STOP):
                ulog("Train received command to stop at %s", tr_command->destination->name);
                stop_sensor = tr_command->destination;
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

            if (!ls_msg->data.edge) {
                goto DONE_PROCESSING_LOCATION_SERVER_MESSAGE;
            }

            old_position_distance = our_position_distance;
            old_position = our_position;

            ls_msg->data.distance = min(ls_msg->data.distance, ls_msg->data.edge->dist);
            our_position = ls_msg->data.edge;
            our_position_distance = ls_msg->data.distance;

            if (our_position != old_position && stop_sensor && our_position->src == stop_sensor) {
                train_set_speed(train_no, 0, location_server_tid);
            }

            // Nothing left to do if we're not en route somewhere
            if (!path_length) {
                goto DONE_PROCESSING_LOCATION_SERVER_MESSAGE;
            }

            // Figure out where we are in the path
            unsigned int i, j;
            int found = 0;
            for (i = path_pos; i < path_length; ++i) {
                if (our_position->src == path[i]) {
                    path_pos = i;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                ulog("Train %u got lost %u um ahead of %s, stopping", train_no, our_position_distance, our_position->src->name);
                path_reserved_pos = path_pos = 0;
                path_length = 0;
                path_reserved_distance = 0;
                train_set_speed(train_no, 0, location_server_tid);
                //calculate_path(our_position->src, tr_command->destination, path, &path_length);
            }


            // Reserve up to us + stopping distance
            path_reserved_distance = -our_position_distance;
            j = path_pos;
            while (1) {
                if (path_reserved_distance > ls_msg->data.stopping_distance) {
                    break;
                }

                if (j >= path_length) {
                    break;
                }

                if (j == path_length - 1) {
                    ulog("Train arriving at %s while %u um ahead of %s", path[j]->name, our_position_distance, our_position->src->name);
                    train_set_speed(train_no, 0, location_server_tid);
                    // We have arrived
                    path_length = 0;
                    break;
                }

                int dist = distance_between_nodes(path[j], path[j+1]);
                cuassert(dist >= 0, "Disconnected nodes in path!");

                //ulog("Train reserved node %s while %u um ahead of %s, buffer space is now %u um", path[j]->name, our_position_distance, our_position->src->name, path_reserved_distance);

                if (path[j]->type == NODE_BRANCH) {
                    int direction = direction_between_nodes(path[j], path[j+1]);
                    cuassert(direction >= 0, "Disconnected nodes in path!");

                    if (D_STRAIGHT == direction) {
                        //ulog("Node %s switched to straight", path[j]->name);
                        SetSwitch(path[j]->num, STRAIGHT);
                    } else if (D_CURVED == direction) {
                        //ulog("Node %s switched to curved", path[j]->name);
                        SetSwitch(path[j]->num, CURVED);
                    } else {
                        ulog ("INVALID DIRECTION");
                    }
                }

                path_reserved_distance += dist;
                ++j;
            }

            DONE_PROCESSING_LOCATION_SERVER_MESSAGE:

            reply.type = LOCATION_SERVER_MESSAGE;
            ls_reply->type = LOCATION_COURIER_RESPONSE;
            Reply(tid, (char *) &reply, sizeof(reply));
            break;

        }

    }
}
