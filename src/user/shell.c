/*
 * shell.c
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#include <shell.h>
#include <nbio.h>
#include <memory.h>
#include <write_server.h>
#include <read_server.h>
#include <syscall.h>
#include <train_server.h>
#include <clock_widget.h>
#include <sensor_server.h>
#include <switch_server.h>

static char line_buffer[LINE_BUFFER_SIZE];
static unsigned int line_buffer_pos;

static void position_cursor (unsigned int row, unsigned int column) {
    nbprintf (COM2, "\033[%u;%uH", row, column);
}

int is_whitespace(char c) {
    return c == ' ' || c == '\t';
}

int is_numeric(char c) {
    return c >= '0' && c <= '9';
}

int is_direction(char c) {
    return c == 'S' || c == 'C';
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

void reset_shell() {
    line_buffer_pos = 0;
    memset(line_buffer, 0, sizeof(line_buffer));
    position_cursor(CONSOLE_HEIGHT, 1);
    nbprintf(COM2, "\033[K");
    nbprintf(COM2, "> ");
}

void shell() {

    // Clear the screen.
    nbprintf(COM2, "\033[2J");

    // Give a blank prompt
    reset_shell();

    // Start the clock
//    Create(LOW, clock_widget);


    Create(HIGH, sensor_server);
    Create(HIGH, switch_server);

    while (1) {

        int train, speed, number, direction;

        // Get a character
        char c = Getc(COM2);

        // Try to interpret the command if enter has been pressed
        if (c == '\n' || c == '\r') {
            if (parse_q(line_buffer)) {
                Exit();
            } else if (parse_tr(line_buffer, &train, &speed)) {
                SetSpeed(train, speed);
            } else if (parse_rv(line_buffer, &number)) {
                Reverse(train);
            } else if (parse_sw(line_buffer, &number, &direction)) {
                SetSwitch(number, direction);
            }

            reset_shell();

            continue;
        } else if (c == '\b') {
            line_buffer_pos--;
            line_buffer[line_buffer_pos] = 0;
            position_cursor(CONSOLE_HEIGHT, line_buffer_pos + 2);
            nbprintf(COM2, "\033[K"); // Delete to end of line
        }

        // Otherwise just store it in the buffer and print it
        line_buffer[line_buffer_pos++] = c;

        position_cursor(CONSOLE_HEIGHT, line_buffer_pos + 2);
        nbputc(COM2, c);
    }

}

