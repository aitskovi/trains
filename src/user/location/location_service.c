#include <location_service.h>

#include <common.h>
#include <constants.h>
#include <dassert.h>
#include <memory.h>
#include <track.h>
#include <track_node.h>
#include <calibration.h>
#include <location_server.h>


static void update_velocity(TrainLocation *train) {
    if (!train->accelerating) return;

    train->velocity += acceleration(train->id, train->acceleration.start, train->acceleration.end, train->acceleration.ticks);
    train->acceleration.ticks++;

    if (train->acceleration.start > train->acceleration.end) {
        if (train->velocity <= train->acceleration.end) {
            train->velocity = train->acceleration.end;
            train->accelerating = 0;
        }
    } else {
        if (train->velocity >= train->acceleration.end) {
            train->velocity = train->acceleration.end;
            train->accelerating = 0;
        }
    }
}

static void update_speed(TrainLocation *train, int speed) {
    // TODO: Update acceleration.
    train->accelerating = 1;
    train->acceleration.start = velocity(train->id, train->speed, train->edge);
    train->acceleration.end = velocity(train->id, speed, train->edge);
    train->acceleration.ticks = 0;

    train->speed = speed;
    update_velocity(train);
}

static void update_max_velocity(TrainLocation *train, int velocity) {
    if (train->accelerating) {
        train->acceleration.end = velocity;
    } else {
        train->velocity = velocity;
    }
}

static TrainLocation *get_train_location(LocationService *service, int train) {
    int index = service->train_to_index[train];
    if (index < 0) return 0;
    return &service->trains[index];
}

static TrainLocation *add_train_location(LocationService *service, int train) {
    int index = service->num_trains;
    service->trains[index].id = train;
    service->num_trains++;
    service->train_to_index[train] = index;
    return &service->trains[index];
}

void locationservice_initialize(struct LocationService *service) {
    memset(service->trains, 0, sizeof(service->trains));
    service->num_trains = 0;

    int i;
    for (i = 0; i < MAX_TRAIN_IDS; ++i) {
        service->train_to_index[i] = -1;
    }

    for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
        service->subscribers[i] = 0;
    }

    circular_queue_initialize(&(service->events));
}

void locationservice_add_event(LocationService *service, TrainLocation *train) {
    int error = circular_queue_push(&service->events, (void *)train->id); 
    cuassert(!error, "Overrun LocationService Event Queue");
}

/**
 * Associate a specific train with a sensor.
 */
void locationservice_associate(LocationService *service, TrainLocation *train, track_edge *edge) {

    train->distance -= (train->edge ? train->edge->dist : 0);
    train->edge = edge;

    // TODO: Deal with acceleration as we switch track segments.
    int max_velocity = velocity(train->id, train->speed, train->edge);
    update_max_velocity(train, max_velocity);

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
            locationservice_add_event(service, train);
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
            locationservice_add_event(service, train);
        }
        
        return 0;
    }

    return -1;
}

int locationservice_distance_event(struct LocationService *service) {
    int i;
    for (i = 0; i < service->num_trains; ++i) {
        TrainLocation *train = &service->trains[i];

        if (!train->edge) return 0;
        if (!train->edge->dest) return 0;

        int old_velocity = train->velocity;
        train->distance += train->velocity;
        update_velocity(train);

        if (train->edge->dest->type == NODE_EXIT) {
            train->distance = min(train->edge->dist, train->distance);
        } else if (train->distance >= train->edge->dist && train->edge->dest->type != NODE_SENSOR) {
            track_edge *next_edge = track_next_edge(train->edge->dest);
            locationservice_associate(service, train, next_edge);
        }

        if (old_velocity > 0) locationservice_add_event(service, train);
    }

    return 0;
}

static int locationservice_reverse_event(struct LocationService *service, int train_number) {
    TrainLocation *train = get_train_location(service, train_number);
    cuassert(train, "Reverse event for invalid train");

    if (!train->edge) return 0;

    // When we're reversing we want to reverse intellegently based on where we are over the
    // sensor, e.g. if we will trigger it again.
    if (train->distance < PICKUP_LENGTH_UM && train->edge->src->type == NODE_SENSOR) {
        train->edge = &train->edge->src->reverse->edge[0];
        train->distance *= -1;
    } else {
        train->edge = train->edge->reverse;
        train->distance = train->edge->dist - train->distance;
    }

    // Associate us with the correct landmark and sensor.
    train->next_sensor = track_next_sensor(train->edge->src);

    locationservice_add_event(service, train);

    return 0;
}

int locationservice_speed_event(struct LocationService *service, int train_number, int speed) {
    if (speed == 15) return locationservice_reverse_event(service, train_number);

    TrainLocation *train = get_train_location(service, train_number);
    cuassert(train, "Speed event for invalid train");

    // TODO: Set-up acceleration stuff. For now, just set our speed.
    update_speed(train, speed);
    if (train->accelerating) {
        //ulog("Train is Accelerating!");
    }

    locationservice_add_event(service, train);
    return 0;
}

int locationservice_add_train(struct LocationService *service, int train_number) {
    cuassert(service->num_trains < MAX_TRAINS, "Attempting to add too many trains");

    // Ensure a duplicate train doesn't exit.
    if (get_train_location(service, train_number)) return -2;

    TrainLocation *train = add_train_location(service, train_number);
    
    locationservice_add_event(service, train);
    return 0;
}

int locationservice_pop_event(LocationService *service, TrainData *data, int *subscribers) {
    if (circular_queue_empty(&(service->events))) return -1;

    int id = (int)circular_queue_pop(&service->events);
    struct TrainLocation *train = get_train_location(service, id);

    data->id = id;
    data->speed = train->speed;
    data->velocity = train->velocity;
    data->edge = train->edge;
    data->distance = train->distance;
    data->stopping_distance = stopping_distance(train->id, train->velocity);
    data->error = calibration_error(train->id);

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
