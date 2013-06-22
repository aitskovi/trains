/*
 * train_server.h
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#ifndef TRAIN_SERVER_H_
#define TRAIN_SERVER_H_

#define NUM_TRAINS 100

typedef unsigned char speed_t;
typedef unsigned char train_t;

typedef struct TrainServerMessage {
    enum {
        REVERSE,
        SET_SPEED,
        REVERSE_RESPONSE,
        SET_SPEED_RESPONSE
    } type;
    train_t train_no;
    speed_t speed;
} TrainServerMessage;

// TODO In the future we want a task per train so the delays don't interfere
int SetSpeed(train_t train, speed_t speed);
int Reverse(train_t train);

void train_server();

#endif /* TRAIN_SERVER_H_ */
