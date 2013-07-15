/*
 * reservation_server.h
 *
 *  Created on: Jul 14, 2013
 *      Author: aianus
 */

#ifndef RESERVATION_SERVER_H_
#define RESERVATION_SERVER_H_

#define RESERVATION_FAILURE 1
#define RESERVATION_ERROR 2
#define RESERVATION_SUCCESS 0
#define RESERVATION_ALREADY_OWNER 3

#include <track_node.h>

typedef struct ReservationServerMessage {
    enum {
        RESERVATION_RESERVE,
        RESERVATION_RELEASE,
        RESERVATION_SUCCESS_RESPONSE,
        RESERVATION_FAILURE_RESPONSE,
        RESERVATION_ERROR_RESPONSE,
        RESERVATION_ALREADY_OWNER_RESPONSE
    } type;
    unsigned int train_no;
    track_node *node;
} ReservationServerMessage;


int Reserve(unsigned int train_no, track_node *node);
int Release(unsigned int train_no, track_node *node);
void reservation_server();

#endif /* RESERVATION_SERVER_H_ */
