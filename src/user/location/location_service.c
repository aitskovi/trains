#include <location_service.h>

#include <memory.h>
#include <track.h>
#include <track_node.h>

void print_train(struct TrainLocation *train);

void locationservice_initialize(struct LocationService *service) {
    service->num_trains = 0;

    int i;
    for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
        service->subscribers[i] = 0;
    }

    circular_queue_initialize(&(service->events));
}

/**
 * Associate a specific train with a sensor.
 */
int locationservice_associate(struct LocationService *service,
                              struct TrainLocation *train, 
                              track_node *sensor) {
    train->landmark = sensor;

    int i;
    for (i = 0; i < MAX_PENDING_SENSORS; ++i) {
        train->sensors[i] = 0;
    }
    
    // Find next sensor.
    int num_sensors = track_sensor_search(sensor, train->sensors);

    // Add an event of this train to our event_queue.
    circular_queue_push(&(service->events), (void *)train->number);

    return 0;
}

int locationservice_sensor_event(struct LocationService *service, char name, int number) {
    // Look for a train waiting for that sensor.
    int i;
    for (i = 0; i < service->num_trains; ++i) {
        struct TrainLocation *train = &(service->trains[i]);

        int j;
        for (j = 0; j < MAX_PENDING_SENSORS; ++j) {
            struct track_node *sensor = train->sensors[j];
            if (!sensor) continue;

            if (sensor_eq(sensor, name, number)) {
                ulog("\nUpdated Train %d with %c%d\n", train->number, name, number);
                locationservice_associate(service, train, sensor);
                return 0;
            }
        }
    }

    // We couldn't find any trains. Do we have an unknown train we
    // can associate the data to.
    if (service->num_trains > 0) {
        struct TrainLocation *train = &(service->trains[service->num_trains - 1]);
        if (train->landmark == 0) {
            track_node *sensor = track_get_sensor(name, number);
            locationservice_associate(service, train, sensor);
        }

        return 0;
    }

    return -1;
}

int locationservice_add_train(struct LocationService *service, int train) {
    if (service->num_trains == MAX_TRAINS) return -1;

    ulog("\nAdding Train %d\n", train);

    struct TrainLocation *tlocation = &(service->trains[service->num_trains]);
    tlocation->number = train;
    tlocation->landmark = 0;
    tlocation->distance = 0;
    int i;
    for (i = 0; i < MAX_PENDING_SENSORS; ++i) {
        tlocation->sensors[i] = 0;
    }
    ++service->num_trains;

    print_train(tlocation);

    return 0;
}

void print_train(struct TrainLocation *train) {
    if (train->landmark) {
        ulog("Train %d, %d cm past %c%d\n", train->number, train->distance, train->landmark->name, 
                train->landmark->num);
    } else {
        ulog("Train %d, unassigned\n", train->number);
    }
}
