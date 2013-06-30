#include <sensor_notifier.h>

#include <dassert.h>
#include <syscall.h>
#include <read_server.h>
#include <write_server.h>
#include <sensor_server.h>
#include <ts7200.h>
#include <log.h>
#include <nameserver.h>

void dump_sensors() {
    Putc(COM1, (char) 133);
}

void enable_sensor_reset() {
    Putc(COM1, (char) 192);
}

void sensor_notifier() {
    // Find the sensor server.
    int sensor_server_tid = -2;
    do {
        sensor_server_tid = WhoIs("SensorServer");
        dlog("Sensor Server Tid %d\n", sensor_server_tid);
    } while (sensor_server_tid < 0);

    // Set up sensors.
    enable_sensor_reset();

    // Poll sensor data and forward it.
    SensorServerMessage msg;
    SensorServerMessage rply;
    msg.type = SENSOR_EVENT_REQUEST;
    for(;;) {
        dump_sensors();
        
        int i = 0;
        for (; i < SENSOR_DATA_SIZE; ++i) {
            msg.data[i] = (char)Getc(COM1);
        }

        Send(sensor_server_tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
        dassert(rply.type == SENSOR_EVENT_RESPONSE, "Invalid Response from SensorServer");
    }
}
