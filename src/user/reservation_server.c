/*
 * reservation_server.c
 *
 *  Created on: Jul 14, 2013
 *      Author: aianus
 */

#include <reservation_server.h>
#include <syscall.h>
#include <encoding.h>
#include <dassert.h>


static tid_t server_tid = -1;

int Reserve(unsigned int train_no, track_node *node) {
    Message msg, reply;
    ReservationServerMessage *rs_msg = &msg.rs_msg;
    ReservationServerMessage *rs_reply = &reply.rs_msg;
    msg.type = RESERVATION_SERVER_MESSAGE;

    rs_msg->type = RESERVATION_RESERVE;
    rs_msg->node = node;
    rs_msg->train_no = train_no;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));

    cuassert(reply.type == RESERVATION_SERVER_MESSAGE, "Received invalid reply message type from reservation server!");

    switch (rs_reply->type) {
    case RESERVATION_ERROR_RESPONSE:
        return RESERVATION_ERROR;
    case RESERVATION_FAILURE_RESPONSE:
        return RESERVATION_FAILURE;
    case RESERVATION_SUCCESS_RESPONSE:
        return RESERVATION_SUCCESS;
    case RESERVATION_ALREADY_OWNER_RESPONSE:
        return RESERVATION_ALREADY_OWNER;
    }

    ulog("Received invalid reply from reservation server!");
    return RESERVATION_ERROR;
}

int Release(unsigned int train_no, track_node *node) {
    Message msg, reply;
    ReservationServerMessage *rs_msg = &msg.rs_msg;
    ReservationServerMessage *rs_reply = &reply.rs_msg;
    msg.type = RESERVATION_SERVER_MESSAGE;

    rs_msg->type = RESERVATION_RELEASE;
    rs_msg->node = node;
    rs_msg->train_no = train_no;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));

    cuassert(reply.type == RESERVATION_SERVER_MESSAGE, "Received invalid reply message type from reservation server on reserve!");

    switch (rs_reply->type) {
    case RESERVATION_ERROR_RESPONSE:
        return RESERVATION_ERROR;
    case RESERVATION_SUCCESS_RESPONSE:
        return RESERVATION_SUCCESS;
    }

    ulog("Received invalid reply from reservation server on release!");
    return RESERVATION_ERROR;
}

static int release_track_node(unsigned int train_no, track_node *track) {
    if (track->owner != 0) {
        if (track->owner != train_no) {
            ulog ("Train %u tried to release %s which belongs to %u!", train_no, track->name, track->owner);
            return RESERVATION_ERROR;
        }
    } else {
        ulog ("Train %u tried to release %s which is already free!", train_no, track->name);
        return RESERVATION_ERROR;
    }
    unsigned int j;
    for (j = 0; j < NUM_NODE_EDGES[track->type]; ++j) {
        if (track->edge[j].dest) {
            track_node *other_direction = track->edge[j].dest->reverse;
            if (other_direction->owner != 0) {
                if (other_direction->owner != train_no) {
                    ulog ("Train %u tried to release %s which belongs to %u!", train_no, other_direction->name, other_direction->owner);
                    return RESERVATION_ERROR;
                }
            } else {
                ulog ("Train %u tried to release %s which is already free!", train_no, other_direction->name);
                return RESERVATION_ERROR;
            }
        }
    }

    track->owner = 0;
    for (j = 0; j < NUM_NODE_EDGES[track->type]; ++j) {
        if (track->edge[j].dest) {
            track_node *other_direction = track->edge[j].dest->reverse;
            other_direction->owner = 0;
        }
    }
    ulog ("Train %u successfully released %s", train_no, track->name);
    return RESERVATION_SUCCESS;
}

static int reserve_track_node(unsigned int train_no, track_node *track) {
    if (track->owner != 0) {
        if (track->owner == train_no) {
            ulog ("Train %u tried reserving %s twice!", train_no, track->name);
            return RESERVATION_ALREADY_OWNER;
        }
        ulog ("Train %u failed to reserve %s which is owned by %u", train_no, track->name, track->owner);
        return RESERVATION_FAILURE;
    }
    unsigned int j;
    for (j = 0; j < NUM_NODE_EDGES[track->type]; ++j) {
        if (track->edge[j].dest) {
            track_node *other_direction = track->edge[j].dest->reverse;
            if (other_direction->owner != 0) {
                ulog ("Train %u failed to reserve %s which is owned by %u", train_no, other_direction->name, other_direction->owner);
                return RESERVATION_FAILURE;
            }
        }
    }
    track->owner = train_no;
    for (j = 0; j < NUM_NODE_EDGES[track->type]; ++j) {
        if (track->edge[j].dest) {
            track_node *other_direction = track->edge[j].dest->reverse;
            other_direction->owner = train_no;
        }
    }
    ulog ("Train %u successfully reserved %s", train_no, track->name);
    return RESERVATION_SUCCESS;
}

// TODO path finding should be done here to avoid reserving in the middle of pathfinding

void reservation_server() {
    server_tid = MyTid();

    RegisterAs("ReservationServer");

    tid_t tid;
    Message msg, reply;
    ReservationServerMessage *rs_msg = &msg.rs_msg;
    ReservationServerMessage *rs_reply = &reply.rs_msg;
    reply.type = RESERVATION_SERVER_MESSAGE;
    int result;
    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));
        switch (msg.type) {
        case RESERVATION_SERVER_MESSAGE:
            switch (rs_msg->type) {
            case RESERVATION_RESERVE:
                result = reserve_track_node(rs_msg->train_no, rs_msg->node);
                break;
            case RESERVATION_RELEASE:
                result = release_track_node(rs_msg->train_no, rs_msg->node);
                break;
            }
            switch (result) {
            case RESERVATION_SUCCESS:
                rs_reply->type = RESERVATION_SUCCESS_RESPONSE;
                break;
            case RESERVATION_ERROR:
                rs_reply->type = RESERVATION_ERROR_RESPONSE;
                break;
            case RESERVATION_FAILURE:
                rs_reply->type = RESERVATION_FAILURE_RESPONSE;
                break;
            case RESERVATION_ALREADY_OWNER:
                rs_reply->type = RESERVATION_ALREADY_OWNER_RESPONSE;
                break;
            default:
                rs_reply->type = RESERVATION_ERROR_RESPONSE;
                break;
            }
            Reply(tid, (char *) &reply, sizeof(reply));
            break;
        default:
            ulog("Reservation server received invalid message type");
            break;
        }
    }
}
