#include <location_service.h>

#include <dassert.h>
#include <memory.h>
#include <track.h>
#include <track_node.h>

#define CM 10

void locationservice_initialize(struct LocationService *service) {
    service->num_trains = 0;

    int i;
    for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
        service->subscribers[i] = 0;
    }

    circular_queue_initialize(&(service->events));
}

static void locationservice_event(LocationService *service, int train_index) {
    int error = circular_queue_push(&service->events, (void *)train_index); 
    cuassert(!error, "Overrun LocationService Event Queue");
}

/**
 * Associate a specific train with a sensor.
 */
void locationservice_associate(LocationService *service, TrainLocation *train, track_edge *edge) {

    train->distance -= (train->edge ? train->edge->dist : 0);
    train->edge = edge;

    // We've reached a sensor. Reset our distance measurement.
    if (train->edge->src->type == NODE_SENSOR) {
        train->distance = 0;
    }

    // Find next sensor.
    train->next_sensor = track_next_sensor(train->edge->src);
}

int locationservice_sensor_event(struct LocationService *service, char name, int number) {
    track_node *sensor = track_get_sensor(name, number);
    track_edge *sensor_edge = &sensor->edge[DIR_STRAIGHT];

    // Look for a train waiting for that sensor.
    int i;
    for (i = 0; i < service->num_trains; ++i) {
        TrainLocation *train = &service->trains[i];
        if (train->next_sensor == sensor) {
            locationservice_associate(service, train, sensor_edge);
            locationservice_event(service, i);
            return 0;
        }
    }

    // We couldn't find any trains. Do we have an unknown train we
    // can associate the data to.
    if (service->num_trains > 0) {
        int last_train = service->num_trains - 1;
        TrainLocation *train = &service->trains[last_train];

        if (!train->edge) {
            locationservice_associate(service, train, sensor_edge);
            locationservice_event(service, last_train);
        }
        
        return 0;
    }

    return -1;
}

int locationservice_distance_event(struct LocationService *service, int train_number) {
    int index;
    for (index = 0; index < service->num_trains; ++index) {
        if (service->trains[index].number == train_number) break;
    }

    cuassert(index < service->num_trains, "Distance event for Invalid Train");

    TrainLocation *train = &service->trains[index];
    if (!train->edge) return 0;
    if (!train->edge->dest) return 0;

    train->distance += CM;

    if (train->distance >= train->edge->dist && train->edge->dest->type != NODE_SENSOR) {
        track_edge *next_edge = track_next_edge(train->edge->dest);
        locationservice_associate(service, train, next_edge);
    }

    locationservice_event(service, index);

    return 0;
}

int locationservice_reverse_event(struct LocationService *service, int train_number) {
    int index;
    for (index = 0; index < service->num_trains; ++index) {
        if (service->trains[index].number == train_number) break;
    }

    cuassert(index < service->num_trains, "Reverse event for Invalid Train");

    struct TrainLocation *train = &service->trains[index];
    if (!train->edge) return 0;

    // Throw us on the opposite edge, and reverse our distance.
    train->edge = train->edge->reverse;
    train->distance = train->edge->dist - train->distance;

    // Associate us with the correct landmark and sensor.
    train->next_sensor = track_next_sensor(train->edge->src);

    locationservice_event(service, index);

    return 0;
}

int locationservice_add_train(struct LocationService *service, int train_number) {
    if (service->num_trains == MAX_TRAINS) return -1;

    // Ensure a duplicate train doesn't exit.
    int index;
    for (index = 0; index < service->num_trains; ++index) {
        if (service->trains[index].number == train_number) return -2;
    }

    index = service->num_trains;
    service->num_trains++;

    // Initialize train train location.
    TrainLocation *train = &service->trains[index];
    train->number = train_number;
    train->distance = 0;
    train->edge = 0;
    train->next_sensor = 0;

    locationservice_event(service, index);

    return 0;
}

int locationservice_pop(struct LocationService *service, int *train,
                                                         struct track_node** landmark,
                                                         struct track_edge** edge,
                                                         int *distance,
                                                         int *subscribers) {
    if (circular_queue_empty(&(service->events))) return -1;

    int event = (int)circular_queue_pop(&service->events);

    struct TrainLocation *tlocation = &(service->trains[event]);
    *train = tlocation->number;
    *landmark = tlocation->edge ? tlocation->edge->src : 0;
    *edge = tlocation->edge;
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
