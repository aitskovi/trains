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
typedef unsigned char switch_t;

enum direction {
    STRAIGHT = 33,
    CURVED = 34
};

typedef struct TrainServerMessage {
    enum {
        REVERSE,
        SET_SPEED,
        SUCCESS
    } type;
    union {
        train_t train_no;
        switch_t switch_no;
    };
    union {
        speed_t speed;
        enum direction state;
    };
} TrainServerMessage;

// TODO In the future we want a task per train so the delays don't interfere
int SetSpeed(train_t train, speed_t speed);
int Reverse(train_t train);

void train_server();

#endif /* TRAIN_SERVER_H_ */
