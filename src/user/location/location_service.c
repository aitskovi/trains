#include <location_service.h>

#include <common.h>
#include <constants.h>
#include <dassert.h>
#include <memory.h>
#include <track.h>
#include <track_node.h>
#include <calibration.h>
#include <location_server.h>
#include <encoding.h>

#define SENSOR_ERROR_OFFSET 10 * CM

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

static void locationservice_add_event(LocationService *service, TrainLocation *train, enum TRAIN_CONFIDENCE confidence) {
    Message msg;
    msg.type = LOCATION_SERVER_MESSAGE;
    msg.ls_msg.type = LOCATION_COURIER_REQUEST;
    msg.ls_msg.data.id = train->id;
    msg.ls_msg.data.speed = train->speed;
    msg.ls_msg.data.velocity = train->velocity;
    msg.ls_msg.data.edge = train->edge;
    msg.ls_msg.data.distance = train->distance;
    msg.ls_msg.data.stopping_distance = stopping_distance(train->id, train->velocity);
    msg.ls_msg.data.error = calibration_error(train->id);
    msg.ls_msg.data.orientation = train->orientation;
    msg.ls_msg.data.confidence = confidence;

    Publish(service->stream, &msg);
}

void locationservice_initialize(struct LocationService *service, tid_t stream) {
    memset(service->trains, 0, sizeof(service->trains));
    service->num_trains = 0;
    service->stream = stream;

    int i;
    for (i = 0; i < MAX_TRAIN_IDS; ++i) {
        service->train_to_index[i] = -1;
    }
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
    train->num_pending_sensors = track_sensor_search(train->edge->src, train->next_sensors);
}

int locationservice_sensor_event(struct LocationService *service, char name, int number) {
    track_node *sensor = track_get_sensor(name, number);
    track_edge *sensor_edge = &sensor->edge[DIR_STRAIGHT];

    unsigned int num_matching_trains = 0;
    int matching_trains[MAX_TRAINS];

    // Look for a train waiting for that sensor.
    int i;
    for (i = 0; i < service->num_trains; ++i) {
        TrainLocation *train = &service->trains[i];

        int j;
        for (j = 0; j < train->num_pending_sensors; ++j) {
            if (train->next_sensors[j] == sensor) {
                matching_trains[num_matching_trains] = i;
                num_matching_trains++;
            }
        }

        if (train->missed_sensor == sensor) {
            train->missed_sensor = 0;
            locationservice_associate(service, train, sensor_edge);
            locationservice_add_event(service, train, CONFIDENCE_LOW);
            return 0;
        }
    }

    // Reduce Matching Trains.
    int max_velocity = -1;
    for (i = 0; i < num_matching_trains; ++i) {
        TrainLocation *train = &service->trains[matching_trains[i]];
        if (train->velocity > max_velocity) {
            max_velocity = train->velocity;
            matching_trains[0] = matching_trains[i];
        }
    }

    if (num_matching_trains > 0) {
        TrainLocation *train = &service->trains[matching_trains[0]];
        train->missed_sensor = 0;
        locationservice_associate(service, train, sensor_edge);
        locationservice_add_event(service, train, CONFIDENCE_HIGH);
        return 0;
    }

    // We couldn't find any trains. Do we have an unknown train we
    // can associate the data to.
    if (service->num_trains > 0) {
        int last_train = service->num_trains - 1;
        TrainLocation *train = &service->trains[last_train];

        if (!train->edge) {
            locationservice_associate(service, train, sensor_edge);
            locationservice_add_event(service, train, CONFIDENCE_HIGH);
        }
        
        return 0;
    }

    return -1;
}

int locationservice_distance_event(struct LocationService *service) {
    int i;
    for (i = 0; i < service->num_trains; ++i) {
        TrainLocation *train = &service->trains[i];

        enum TRAIN_CONFIDENCE confidence = train->accelerating ? CONFIDENCE_LOW : CONFIDENCE_MEDIUM;
        int old_velocity = train->velocity;
        train->distance += train->velocity;
        update_velocity(train);

        if (!train->edge) return 0;
        if (!train->edge->dest) return 0;

        // Check for trains on the same edge as us. We shouldn't cross them.
        int j;
        for (j = 0; j < service->num_trains; ++j) {
            TrainLocation *other = &service->trains[j];

            if (train->edge->reverse == other->edge) {
                if (train->distance > other->edge->dist - other->distance) {
                    train->distance = max(0, other->edge->dist - other->distance);
                }
            }
        }

        if (train->edge->dest->type == NODE_EXIT) {
            train->distance = min(train->edge->dist, train->distance);
        } else if (train->distance >= train->edge->dist) {
            if (train->edge->dest->type != NODE_SENSOR) {
                track_edge *next_edge = track_next_edge(train->edge->dest);
                locationservice_associate(service, train, next_edge);
            } else if (train->distance >= train->edge->dist + SENSOR_ERROR_OFFSET) {
                //cuassert(!train->missed_sensor, "Missed Multiple Sensors in a Row");
                if (train->missed_sensor) {
                    continue;
                }
                track_edge *next_edge = track_next_edge(train->edge->dest);
                locationservice_associate(service, train, next_edge);
                train->missed_sensor = next_edge->src;
                confidence = CONFIDENCE_LOW;
            }
        }


        // TODO remove this!!!
        static int tick = 0;
        if (old_velocity > 0 || tick++ % 100 == 0 || train->velocity > 0) locationservice_add_event(service, train, confidence);
    }

    return 0;
}

static int locationservice_reverse_event(struct LocationService *service, int train_number) {
    TrainLocation *train = get_train_location(service, train_number);
    cuassert(train, "Reverse event for invalid train");

    if (!train->edge) return 0;

    // When we're reversing we want to reverse intellegently based on where we are over the
    // sensor, e.g. if we will trigger it again.
    if (train->distance <= 0 && train->edge->src->type == NODE_SENSOR) {
        train->edge = &train->edge->src->reverse->edge[0];
        train->distance = 0;
    } else {
        train->edge = train->edge->reverse;
        train->distance = train->edge->dist - train->distance;
    }

    if (train->orientation == TRAIN_FORWARD) train->orientation = TRAIN_BACKWARD;
    else if (train->orientation == TRAIN_BACKWARD) train->orientation = TRAIN_FORWARD;

    // Associate us with the correct landmark and sensor.
    train->num_pending_sensors = track_sensor_search(train->edge->src, train->next_sensors);
    // Clear missed sensors on reverse.
    train->missed_sensor = 0;

    locationservice_add_event(service, train, CONFIDENCE_MEDIUM);

    return 0;
}

int locationservice_speed_event(struct LocationService *service, int train_number, int speed) {
    if (speed == 15) return locationservice_reverse_event(service, train_number);

    TrainLocation *train = get_train_location(service, train_number);
    cuassert(train, "Speed event for invalid train");

    // TODO: Set-up acceleration stuff. For now, just set our speed.
    update_speed(train, speed);

    locationservice_add_event(service, train, CONFIDENCE_MEDIUM);
    return 0;
}

int locationservice_orientation_event(LocationService *service,
                                      int train_number,
                                      enum TRAIN_ORIENTATION orientation) {
    TrainLocation *train = get_train_location(service, train_number);
    cuassert(train, "Orientation event for invalid train");

    train->orientation = orientation;

    locationservice_add_event(service, train, CONFIDENCE_MEDIUM);
    return 0;
}

int locationservice_add_train(struct LocationService *service, int train_number) {
    cuassert(service->num_trains < MAX_TRAINS, "Attempting to add too many trains");

    // Ensure a duplicate train doesn't exit.
    if (get_train_location(service, train_number)) return -2;

    TrainLocation *train = add_train_location(service, train_number);
    
    locationservice_add_event(service, train, CONFIDENCE_MEDIUM);
    return 0;
}
