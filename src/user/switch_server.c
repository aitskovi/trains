/*
 * switch_server.c
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#include <sprintf.h>
#include <switch_server.h>
#include <syscall.h>
#include <dassert.h>
#include <ts7200.h>

#define AUXILLARY_SWITCH_BASE 0x99
#define AUXILLARY_SWITCH_COUNT 4
#define NUM_SWITCHES 18
#define SWITCH_TABLE_HEIGHT 5

typedef int bool;

#define true 1
#define false 0

static tid_t server_tid = -1;

int switch_update(int number, enum direction position);

void switches_init() {
    char command[1024];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337\033[%u;%uH", SWITCH_TABLE_HEIGHT, 1);
    pos += sprintf(pos, "Switches: ");

    pos += sprintf(pos, "\033[%u;%uH", SWITCH_TABLE_HEIGHT + 1, 1);
    int i;
    // Draw Regular Switches.
    for (i = 0; i < NUM_SWITCHES; ++i) {
        char num[4];
        ui2a(i + 1, 10,  num);
        pos += sputw(pos, 4, ' ', num);
    }

    // Draw Auxillary Switches.
    for (i = 0; i < AUXILLARY_SWITCH_COUNT; ++i) {
        char num[4];
        ui2a(AUXILLARY_SWITCH_BASE + i, 10,  num);
        pos += sputw(pos, 4, ' ', num);
    }

    pos += sprintf(pos, "\033[%u;%uH", SWITCH_TABLE_HEIGHT + 2, 1);

    // Draw States.
    for (i = 0; i < NUM_SWITCHES + AUXILLARY_SWITCH_COUNT; ++i) {
        pos += sputw(pos, 4, ' ', "?");
    }

    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

bool is_auxillary_switch(int number) {
    return number >=  AUXILLARY_SWITCH_BASE
        && number < AUXILLARY_SWITCH_BASE + AUXILLARY_SWITCH_COUNT;
}

bool is_normal_switch(int number) {
    return number >= 1 && number <= NUM_SWITCHES;
}

int switch_update(int number, enum direction position) {
    int index;
    if (is_normal_switch(number)) {
        index = number - 1;
    } else if (is_auxillary_switch(number)) {
        index = number - AUXILLARY_SWITCH_BASE + NUM_SWITCHES;
    } else {
        return -1;
    }

    index = index * 4;

    char command[128];
    char *pos = &command[0];
    pos += sprintf(pos, "\0337\033[%u;%uH", SWITCH_TABLE_HEIGHT + 2, index + 1);
    pos += sputw(pos, 4, ' ', position == CURVED ? "C" : "S");
    pos += sprintf(pos, "\0338");
    Write(COM2, command, pos - command);

    return 0;
}

void switch_set(int turnout, enum direction state) {
    char command[3] = {state, turnout, 32};
    Write(COM1, command, 3);
    switch_update(turnout, state);
}

int SetSwitch(switch_t switch_no, enum direction direction) {
    if (server_tid < 0) {
        return -1;
    }
    SwitchServerMessage msg, reply;
    msg.type = SET_SWITCH;
    msg.switch_no = switch_no;
    msg.direction = direction;
    Send(server_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
    dassert(reply.type == SET_SWITCH_RESPONSE, "Invalid response from switch server");
    return 0;
}

void switch_server() {
    server_tid = MyTid();

    switches_init();

    tid_t tid;
    SwitchServerMessage msg, reply;
    while (1) {
        Receive(&tid, (char *) &msg, sizeof(msg));
        switch (msg.type) {
        case SET_SWITCH:
            reply.type = SET_SWITCH_RESPONSE;
            Reply(tid, (char *) &reply, sizeof(reply));
            switch_set(msg.switch_no, msg.direction);
            break;
        default:
            break;
        }
    }
}
