/*
 * shell.h
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#ifndef SHELL_H_
#define SHELL_H_

#include <track_node.h>
#include <switch_server.h>

#define CONSOLE_HEIGHT 30
#define LINE_BUFFER_SIZE 70

typedef struct ShellMessage {

    enum {
        INIT_TRACK,
        ADD_TRAIN,
        REVERSE_TRAIN,
        SET_TRAIN_SPEED,
        SET_TRAIN_POSITION,
        SET_SWITCH_POSITION,
        SHELL_SUCCESS_REPLY
    } type;

    char track;

    union {
        unsigned char train_no;
        unsigned char switch_no;
    };

    union {
        unsigned char switch_pos;
        unsigned char speed;
    };

    struct track_node *position;
    int distance;

} ShellMessage;

void shell();

#endif /* SHELL_H_ */
