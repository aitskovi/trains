#include <location_service.h>

#include <memory.h>
#include <track.h>
#include <track_node.h>

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
                              int index,
                              struct TrainLocation *train, 
                              track_node *node) {
    train->landmark = node;
    if (train->landmark->type == NODE_SENSOR) {
        train->distance = 0;
    } else if (train->edge) {
        train->distance -= train->edge->dist;
    }

    // Find next sensor.
    train->edge = track_next_edge(train->landmark);
    train->next_sensor = track_next_sensor(train->landmark);

    // Add an event of this train to our event_queue.
    circular_queue_push(&(service->events), (void *)index);

    return 0;
}

int locationservice_sensor_event(struct LocationService *service, char name, int number) {
    // Look for a train waiting for that sensor.
    int i;
    for (i = 0; i < service->num_trains; ++i) {
        struct TrainLocation *train = &(service->trains[i]);
        struct track_node *sensor = train->next_sensor;
        if (!sensor) continue;

        if (sensor_eq(sensor, name, number)) {
            locationservice_associate(service, i, train, sensor);
            return 0;
        }
    }

    // We couldn't find any trains. Do we have an unknown train we
    // can associate the data to.
    if (service->num_trains > 0) {
        struct TrainLocation *train = &(service->trains[service->num_trains - 1]);
        if (train->landmark == 0) {
            track_node *sensor = track_get_sensor(name, number);
            locationservice_associate(service, service->num_trains - 1, train, sensor);
        }
        
        return 0;
    }

    return -1;
}

int locationservice_distance_event(struct LocationService *service, int train) {
    int i;
    for (i = 0; i < service->num_trains; ++i) {
        struct TrainLocation *tlocation = &(service->trains[i]);
        if (!tlocation->landmark) return 0;

        if (tlocation->number == train) {
            tlocation->distance += 10;
            
            if (tlocation->edge->dest &&
                tlocation->distance >= tlocation->edge->dist &&
                tlocation->edge->dest->type != NODE_SENSOR) {
                return locationservice_associate(service, service->num_trains - 1, tlocation, tlocation->edge->dest);
            }

            // Add an event of this train to our event_queue.
            circular_queue_push(&(service->events), (void *)i);
            break;
        }
    }

    return 0;
}

int locationservice_reverse_event(struct LocationService *service, int train) {
    int i;
    for (i = 0; i < service->num_trains; ++i) {
        struct TrainLocation *tlocation = &(service->trains[i]);
        if (!tlocation->landmark) return 0;

        if (tlocation->number == train) {
            // Throw us on the opposite edge.
            tlocation->edge = tlocation->edge->reverse;
            tlocation->distance = tlocation->edge->dist - tlocation->distance;

            // Associate us with the correct landmark and sensor.
            tlocation->landmark = tlocation->edge->src;
            tlocation->next_sensor = track_next_sensor(tlocation->landmark);

            // Add an event of this train to our event_queue.
            circular_queue_push(&(service->events), (void *)i);
            break;
        }
    }

    return 0;
}

int locationservice_add_train(struct LocationService *service, int train) {
    if (service->num_trains == MAX_TRAINS) return -1;

    struct TrainLocation *tlocation = &(service->trains[service->num_trains]);
    tlocation->number = train;
    tlocation->landmark = 0;
    tlocation->distance = 0;
    tlocation->edge = 0;
    tlocation->next_sensor = 0;

    // Add the train to event.
    circular_queue_push(&(service->events), (void *)service->num_trains);

    ++service->num_trains;

    return 0;
}

int locationservice_pop(struct LocationService *service, int *train,
                                                         struct track_node** landmark,
                                                         int *distance,
                                                         int *subscribers) {
    if (circular_queue_empty(&(service->events))) return -1;

    int event = (int)circular_queue_pop(&service->events);

    struct TrainLocation *tlocation = &(service->trains[event]);
    *train = tlocation->number;
    *landmark = tlocation->landmark;
    *distance = tlocation->distance;

    int i;
    for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
        subscribers[i] = service->subscribers[i];
    }

    return 0;
}


int locationservice_subscribe(struct LocationService *service, int tid) {
    int section = tid / 32;
    int bit = 1 << (tid % 32);

    service->subscribers[section] |= bit;

    return 0;
}

int locationservice_unsubscribe(struct LocationService *service, int tid) {
    int section = tid / 32;
    int bit  = 1 << (tid % 32);

    service->subscribers[section] &= ~bit;

    return 0;
}
