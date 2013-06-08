/*
 * clock.h
 *
 *  Created on: Jun 8, 2013
 *      Author: aianus
 */

#ifndef CLOCK_H_
#define CLOCK_H_

typedef unsigned int time_t;

enum ClockMessageType {
    TICK_REQUEST,
    TICK_RESPONSE,
    DELAY_REQUEST,
    DELAY_RESPONSE,
    TIME_REQUEST,
    TIME_RESPONSE,
    DELAY_UNTIL_REQUEST
};

typedef struct ClockMessage {
    enum ClockMessageType type;
    union {
        time_t delay;
        time_t time;
    };
} ClockMessage;

#endif /* CLOCK_H_ */
