#include <sensor_service.h>

#include <sensor_server.h>
#include <circular_queue.h>
#include <ts7200.h>

#define INVALID_SENSOR -3

char int_to_sensor(int i) {
    if (i < 0 && i >= NUM_SENSORS) return 'U';
    return (char)(i + (int)'A');
}

int sensor_to_int(char c) {
    if (c < 'A' || c > 'E') return -1;
    return (char)(c - (int)'A');
}

void sensorservice_initialize(struct SensorService *service) {
    int i;

    for (i = 0; i < NUM_SENSORS; ++i) {
        circular_queue_initialize(&(service->sensor_data[i]));
    }
}

int sensorservice_process_data(struct SensorService *service, char *data) {
    int i, j;
    for (i = 0; i < NUM_SENSORS; ++i) {
        for (j = 0; j < 2; ++j) {
            int sensor = i * 2 + j;
            int bit = 128;
            int d = data[sensor];

            int k = 0;
            for (; k < 8; ++k) {
                if (d >= bit) {
                    char sensor = int_to_sensor(i);
                    int number = 8 * j + k + 1;
                    sensorservice_push(service, sensor, number);
                    d -= bit;
                }
                bit /= 2;
            }
        }
    }

    return 0;
}

int sensorservice_push(struct SensorService *service, char sensor, int number) {
    int sensor_num = sensor_to_int(sensor);
    if (sensor_num == -1) return INVALID_SENSOR;

    return circular_queue_push(&(service->sensor_data[sensor_num]), (void *)number);
}

int sensorservice_pop(struct SensorService *service, char *sensor, int *number) {
    int i;
    for (i = 0; i < NUM_SENSORS; ++i) {
        struct circular_queue *queue = &(service->sensor_data[i]);
        if (!circular_queue_empty(queue)) {
            *sensor = int_to_sensor(i); 
            *number = (int)circular_queue_pop(queue);

            return 0;
        }
    }

    return -1;
}
