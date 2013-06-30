#include <sensor_widget.h>

#include <dassert.h>
#include <encoding.h>
#include <log.h>
#include <memory.h>
#include <nameserver.h>
#include <syscall.h>
#include <sprintf.h>
#include <sensor_server.h>
#include <ts7200.h>
#include <write_server.h>

void sensor_list_print(char *sensors, int *numbers) {

    char command[512];
    char *pos = &command[0];

    pos += sprintf(pos, "\0337\033[%u;%uH\033[K", SENSOR_LIST_HEIGHT, 1);
    pos += sprintf(pos, "Recently Triggered:");

    int i;
    for (i = 0; i < NUM_READINGS; ++i) {
        if (numbers[i] == 0) break;
        pos += sprintf(pos, " %c%d ", sensors[i], numbers[i]);
    }

    pos += sprintf(pos, "\0338");

    Write(COM2, command, pos - command);
}

int sensor_list_add(char *sensors, int *numbers, char sensor, int number) {
    // Dedupe, we get a lot of fast triggers for same sensor.
    if (sensors[0] == sensor && numbers[0] == number) return -1;

    int i;
    for (i = NUM_READINGS - 1; i > 0; --i) {
        sensors[i] = sensors[i - 1];
        numbers[i] = numbers[i - 1];
    }
    sensors[0] = sensor;
    numbers[0] = number;

    return 0;
}

void sensor_widget() {
    char triggered_sensor[NUM_READINGS];
    int triggered_number[NUM_READINGS];

    // Reset the list.
    memset(triggered_sensor, 0, sizeof(triggered_sensor));
    memset(triggered_number, 0, sizeof(triggered_number));

    // Find the sensor server.
    int sensor_server_tid = -2;
    do {
        sensor_server_tid = WhoIs("SensorServer");
        dlog("Sensor Server Tid %d\n", sensor_server_tid);
    } while (sensor_server_tid < 0);

    // Subscribe.
    sensor_server_subscribe(sensor_server_tid);

    // Print our initial thing.
    sensor_list_print(triggered_sensor, triggered_number);

    // Deal with our subscription.
    int tid;
    struct Message msg, rply;
    sensor_list_print(triggered_sensor, triggered_number);

    for(;;) {
        // Recieve a Sensor Message.
        Receive(&tid, (char *) &msg, sizeof(msg));
        dassert(msg.type == SENSOR_SERVER_MESSAGE, "Invalid Message");
        dassert(msg.ss_msg.type == SENSOR_COURIER_REQUEST, "Invalid Sensor Widget Request");

        // Ack Sensor Message.
        rply.type = SENSOR_SERVER_MESSAGE;
        rply.ss_msg.type = SENSOR_COURIER_RESPONSE;
        Reply(tid, (char *) &rply, sizeof(rply));

        // Update Sensor List.
        int error = sensor_list_add(triggered_sensor, triggered_number, msg.ss_msg.sensor, msg.ss_msg.number);
        if (!error) sensor_list_print(triggered_sensor, triggered_number);
    }

    Exit();
}
