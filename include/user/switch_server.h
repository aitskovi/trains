/*
 * switch_server.h
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#ifndef SWITCH_SERVER_H_
#define SWITCH_SERVER_H_

typedef unsigned char switch_t;

#define STRAIGHT 33
#define CURVED 34

typedef struct SwitchServerMessage {
    enum {
        SET_SWITCH,
        SET_SWITCH_RESPONSE
    } type;
    switch_t switch_no;
    unsigned char direction;
} SwitchServerMessage;

int SetSwitch(switch_t switch_no, unsigned char direction);

char switch_get_position(int num);

void switch_server();


#endif /* SWITCH_SERVER_H_ */
