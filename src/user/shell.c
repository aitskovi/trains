/*
 * shell.c
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#include <shell.h>
#include <sprintf.h>
#include <memory.h>
#include <write_server.h>
#include <read_server.h>
#include <syscall.h>
#include <mission_control.h>
#include <clock_widget.h>
#include <switch_server.h>
#include <ts7200.h>
#include <string.h>
#include <sensor_widget.h>
#include <track.h>
#include <location_server.h>
#include <train_widget.h>
#include <encoding.h>
#include <nameserver.h>
#include <dassert.h>

const char CLEAR_SCREEN[] = "\033[2J";
const char CLEAR_LINE[] = "\033[K";
const char POS_CURSOR[] = "\033[%u;%uH";
const char SCROLLABLE_AREA[] = "\033[%u;%ur";
const char RESET[] = "\033c";

static char line_buffer[LINE_BUFFER_SIZE];
static unsigned int line_buffer_pos;

/*
static void position_cursor (unsigned int row, unsigned int column) {
    nbprintf (COM2, "\033[%u;%uH", row, column);
}
*/

int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == 0 || c == '\n';
}

int is_numeric(char c) {
    return c >= '0' && c <= '9';
}

int is_direction(char c) {
    return c == 'S' || c == 'C';
}

int is_track(char c) {
    return c == 'A' || c == 'B';
}

int parse_uint(char **src) {
    char* str = *src;

    while(is_whitespace(*str)) ++str;

    if (!is_numeric(*str)) return -1;

    int num = 0;
    while(is_numeric(*str)) {
        int digit = (int)(*str - '0');
        num = num * 10 + digit;
        ++str;
    }

    *src = str;
    return num;
}

int parse_direction(char **src) {
    char* str = *src;

    while(is_whitespace(*str)) ++str;

    if (!is_direction(*str)) return -1;
    int result = *str == 'S' ? STRAIGHT : CURVED;
    ++str;

    *src = str;

    return result;
}

int parse_track(char **src) {
    char* str = *src;

    while(is_whitespace(*str)) ++str;

    if (!is_track(*str)) return -1;
    int result = *str == 'A' ? TRACK_A : TRACK_B;
    ++str;

    *src = str;

    return result;
}

int parse_q(char *str) {
    // Skip Whitespace.
    while(is_whitespace(*str)) ++str;

    if (*str == 'q') return 1;
    else return 0;
}

int parse_tr(char *str, int *train, int *speed) {
    while(is_whitespace(*str)) ++str;

    if (*str != 't') return 0;
    str++;
    if (*str != 'r') return 0;
    str++;

    *train = parse_uint(&str);
    if (*train == -1) return 0;

    *speed = parse_uint(&str);
    if (*speed == -1) return 0;

    return 1;
}

int parse_rv(char *str, int *train) {
    while(is_whitespace(*str)) ++str;

    if (*str != 'r') return 0;
    str++;
    if (*str != 'v') return 0;
    str++;

    *train = parse_uint(&str);
    if (*train == -1) return 0;

    return 1;
}

int parse_sw(char *str, int *number, int *direction) {
    while(is_whitespace(*str)) ++str;

    if (*str != 's') return 0;
    str++;
    if (*str != 'w') return 0;
    str++;

    *number = parse_uint(&str);
    if (*number == -1) return 0;

    *direction = parse_direction(&str);
    if (*direction == -1) return 0;

    return 1;
}

int parse_in(char *str, int *track) {
    while(is_whitespace(*str)) ++str;

    if (*str != 'i') return 0;
    str++;
    if (*str != 'n') return 0;
    str++;

    *track = parse_track(&str);
    if (*track == -1) return 0;

    return 1;
}

int parse_ad(char *str, int *train) {
    while(is_whitespace(*str)) ++str;

    if (*str != 'a') return 0;
    str++;
    if (*str != 'd') return 0;
    str++;

    *train = parse_uint(&str);
    if (*train == -1) return 0;

    return 1;
}

int parse_p(char *str, char *landmark1, char *landmark2) {
    // Skip Whitespace.
    while(is_whitespace(*str)) str++;

    if (*str != 'p') return 0;
    str++;

    while(is_whitespace(*str)) str++;
    while(!is_whitespace(*str)) *landmark1++ = *str++;
    *landmark1 = 0;

    while(is_whitespace(*str)) str++;
    while(!is_whitespace(*str)) *landmark2++ = *str++;
    *landmark2 = 0;

    return 1;
}

int parse_go(char *str, int *train, char *landmark) {
    // Skip Whitespace.
    while(is_whitespace(*str)) str++;

    if (*str++ != 'g') return 0;
    if (*str++ != 'o') return 0;

    *train = parse_uint(&str);
    if (*train == -1) return 0;

    while(is_whitespace(*str)) str++;
    while(!is_whitespace(*str)) *landmark++ = *str++;
    *landmark = 0;

    return 1;
}

void reset_shell() {
    line_buffer_pos = 0;
    memset(line_buffer, 0, sizeof(line_buffer));

    // Build command for positioning cursor, clearing the line, and printing a prompt
    char command[100];
    char *pos = &command[0];
    pos += sprintf(pos, (char *)POS_CURSOR, CONSOLE_HEIGHT, 1);
    pos += strcpy(pos, (char *)CLEAR_LINE);
    pos += strcpy(pos, "> ");

    Write(COM2, command, pos - command);
}

void generate_debug_area() {
    char command[200];
    char *pos = &command[0];

    pos += sprintf(pos, (char *)SCROLLABLE_AREA, CONSOLE_HEIGHT + 1, CONSOLE_HEIGHT + 11);
    Write(COM2, command, pos - command);
}

void shell() {

    tid_t mission_control_tid;
    do {
        mission_control_tid = WhoIs("MissionControl");
    } while (mission_control_tid  < 0);

    // Clear the screen.
    Write(COM2, (char *)CLEAR_SCREEN, strlen((char *)CLEAR_SCREEN));

    // Give a blank prompt
    reset_shell();

    // Start the clock
    Create(LOW, clock_widget);
    // Start the sensor widget.
    Create(LOW, sensor_widget);
    // Start the train widget.
    Create(LOW, train_widget);

    Create(HIGH, switch_server);

    // Create the debug print area.
    generate_debug_area();

    Message msg, reply;
    msg.type = SHELL_MESSAGE;
    ShellMessage *sh_msg = &msg.sh_msg;
    while (1) {
        char command[50 + LINE_BUFFER_SIZE];
        char *pos;
        int train, speed, number, direction, track;
        char landmark_buffer1[5];
        char landmark_buffer2[5];

        // Get a character
        char c = Getc(COM2);

        // Try to interpret the command if enter has been pressed
        if (c == '\n' || c == '\r') {
            if (parse_q(line_buffer)) {
                Write(COM2, (char *)RESET, strlen((char *)RESET));
                Delay(10);
                Exit();
            } else if (parse_tr(line_buffer, &train, &speed)) {
                sh_msg->type = SHELL_SET_TRAIN_SPEED;
                sh_msg->speed = speed;
                sh_msg->train_no = train;
                Send(mission_control_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
                // TODO assert message response
            } else if (parse_rv(line_buffer, &number)) {
                sh_msg->type = SHELL_REVERSE_TRAIN;
                sh_msg->train_no = number;
                Send(mission_control_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
                // TODO assert message response
            } else if (parse_sw(line_buffer, &number, &direction)) {
                sh_msg->type = SHELL_SET_SWITCH_POSITION;
                sh_msg->switch_no = number;
                sh_msg->switch_pos = direction;
                Send(mission_control_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
                // TODO assert message response
            } else if (parse_in(line_buffer, &track)) {
                ulog("\nShell initializing track");

                sh_msg->type = SHELL_INIT_TRACK;
                sh_msg->track = track;
                Send(mission_control_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
                cuassert(reply.type == SHELL_MESSAGE, "Shell received unexpected message");
                cuassert(reply.sh_msg.type == SHELL_SUCCESS_REPLY, "Shell received unexpected message");
            } else if (parse_ad(line_buffer, &number)) {
                sh_msg->type = SHELL_ADD_TRAIN;
                sh_msg->train_no = number;
                Send(mission_control_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
                cuassert(reply.type == SHELL_MESSAGE, "Shell received unexpected message");
                cuassert(reply.sh_msg.type == SHELL_SUCCESS_REPLY, "Shell received unexpected message");
            } else if (parse_p(line_buffer, landmark_buffer1, landmark_buffer2)) {
                ulog("Calculating path between %s and %s", landmark_buffer1, landmark_buffer2);
                configure_track_for_path(track_get_by_name(landmark_buffer1), track_get_by_name(landmark_buffer2));
            } else if (parse_go(line_buffer, &train, landmark_buffer1)) {
                ulog("\nShell making train %u go to %s", train, landmark_buffer1);
                sh_msg->type = SHELL_GO;
                sh_msg->train_no = train;
                sh_msg->position = track_get_by_name(landmark_buffer1);
                if (!sh_msg->position) {
                    ulog("\nCan't find landmark %s", landmark_buffer1);
                } else {
                    Send(mission_control_tid, (char *) &msg, sizeof(msg), (char *) &reply, sizeof(reply));
                    cuassert(reply.type == SHELL_MESSAGE, "Shell received unexpected message");
                    cuassert(reply.sh_msg.type == SHELL_SUCCESS_REPLY, "Shell received unexpected message");
                }
            }

            line_buffer[line_buffer_pos + 1] = 0;
            ulog(line_buffer);
            reset_shell();

            continue;
        } else if (c == '\b') {
            line_buffer_pos--;
            line_buffer[line_buffer_pos] = 0;

            pos = &command[0];
            pos += strcpy(pos, "\b");
            Write(COM2, command, pos - command);

            continue;
        }

        // Otherwise just store it in the buffer and print it
        line_buffer[line_buffer_pos++] = c;

        pos = &command[0];
        pos += sprintf(pos, (char *)POS_CURSOR, CONSOLE_HEIGHT, line_buffer_pos + 2);
        *pos++ = c;
        Write(COM2, command, pos - command);
    }
}
