/*
 * sensor_server.c
 *
 *  Created on: Jun 22, 2013
 *      Author: aianus
 */

#include <sensor_server.h>
#include <sprintf.h>
#include <read_server.h>
#include <write_server.h>
#include <ts7200.h>

typedef int bool;

#define true 1
#define false 0

static bool waiting;
static char data[SENSOR_DATA_SIZE];
static unsigned int data_index;

static char triggered_sensor[NUM_READINGS];
static int triggered_number[NUM_READINGS];

void process_sensors();

void sensor_list_print() {

    char command[512];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337\033[%u;%uH\033[K", SENSOR_LIST_HEIGHT, 1);
    pos += sprintf(pos, "Recently Triggered:");

    int i;
    for (i = 0; i < NUM_READINGS; ++i) {
        if (triggered_number[i] == 0) break;
        pos += sprintf(pos, " %c%d ", triggered_sensor[i], triggered_number[i]);
    }

    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

char int_to_sensor(int i) {
    switch(i) {
        case 0:
            return 'A';
        case 1:
            return 'B';
        case 2:
            return 'C';
        case 3:
            return 'D';
        case 4:
            return 'E';
        default:
            return 'U';
    }
}

void sensor_list_add(char sensor, int number) {
    // Dedupe, we get a lot of fast triggers for same sensor.
    if (triggered_sensor[0] == sensor && triggered_number[0] == number) return;

    int i;
    for (i = NUM_READINGS - 1; i > 0; --i) {
        triggered_sensor[i] = triggered_sensor[i - 1];
        triggered_number[i] = triggered_number[i - 1];
    }
    triggered_sensor[0] = sensor;
    triggered_number[0] = number;
}

void process_sensors() {
    bool changed = false;
    int i, j;
    for (i = 0; i < 5; ++i) {
        for (j = 0; j < 2; ++j) {
            int sensor = i * 2 + j;
            int bit = 128;
            int d = data[sensor];

            int k = 0;
            for (; k < 8; ++k) {
                if (d >= bit) {
                    char sensor = int_to_sensor(i);
                    int number = 8 * j + k + 1;
                    sensor_list_add(sensor, number);
                    d -= bit;
                    changed = true;
                }
                bit /= 2;
            }
        }
    }

    if (changed) sensor_list_print();
}

void dump_sensors() {
    Putc(COM1, (char) 133);
}

void enable_sensor_reset() {
    Putc(COM1, (char) 192);
}

void sensors_init() {
    data_index = 0;
    waiting = false;

    enable_sensor_reset();

    memset(data, 0, sizeof(data));
    memset(triggered_sensor, 0, sizeof(triggered_sensor));
    memset(triggered_number, 0, sizeof(triggered_number));

    sensor_list_print();
}

void sensor_server() {
    sensors_init();

    while (1) {
        if (waiting == false) {
            dump_sensors();
            waiting = true;
        }

        int c = Getc(COM1);
        if (c == -1) return;

        // Read some interesting data.
        data[data_index] = (char)c;
        data_index++;

        if (data_index == SENSOR_DATA_SIZE) {
            process_sensors();
            waiting = false;
            data_index = 0;
        }
    }
}
