/*
 * train_task.h
 *
 *  Created on: Jun 30, 2013
 *      Author: aianus
 */

#ifndef TRAIN_TASK_H_
#define TRAIN_TASK_H_

#include <track_node.h>

typedef unsigned char speed_t;
typedef int train_t;

typedef struct TrainMessage {
    enum {
        COMMAND_SET_SPEED,
        COMMAND_REVERSE,
        COMMAND_AWAITING,
        COMMAND_GOTO,
        COMMAND_STOP,
        COMMAND_ACKNOWLEDGED,
        COMMAND_RESET
    } type;
    speed_t speed;
    train_t train;
    track_node *destination;
} TrainMessage;

void train_task(int train_no);

#endif /* TRAIN_TASK_H_ */
