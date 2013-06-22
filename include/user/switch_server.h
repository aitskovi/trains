/*
 * switch_server.h
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#ifndef SWITCH_SERVER_H_
#define SWITCH_SERVER_H_

typedef unsigned char switch_t;
enum direction {
    STRAIGHT = 33,
    CURVED = 34
};

typedef struct SwitchServerMessage {
    enum {
        SET_SWITCH,
        SET_SWITCH_RESPONSE
    } type;
    switch_t switch_no;
    enum direction direction;
} SwitchServerMessage;


int SetSwitch(switch_t switch_no, enum direction direction);
void switch_server();

#endif /* SWITCH_SERVER_H_ */
