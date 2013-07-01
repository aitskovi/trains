#ifndef _DISTANCE_SERVICE_H_
#define _DISTANCE_SERVICE_H_

#include <heap_priority_queue.h>
#include <circular_queue.h>
#include <task.h>

struct DistanceService {
    HeapPriorityQueue timeouts;

    struct circular_queue events;
    int subscribers[MAX_SUBSCRIBERS];
};

void distanceservice_initialize(struct DistanceService *service, PriorityQueueElement *buffer, int size);

/**
 * Notify the time 
 */
int distanceservice_notify(struct DistanceService *service, int time);

/**
 * Returns the next timeout.
 */
int distanceservice_timeout(struct DistanceService *service);

/**
 * Get a distance event.
 */ 
int distanceservice_event(struct DistanceService *service, int *train, int *subscribers);

/**
 * Update a train speed.
 */
int distanceservice_update_train(struct DistanceService *service, int train, int speed);

/**
 * Subscription Management.
 */
int distanceservice_subscribe(struct DistanceService *service, int tid);
int distanceservice_unsubscribe(struct DistanceService *service, int tid);

#endif
