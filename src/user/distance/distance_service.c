#include <distance_service.h>

#include <clock_server.h>

int speed_to_timeout(int train, int speed) {
    return 2;
}

void distanceservice_initialize(struct DistanceService *service, PriorityQueueElement *buffer, int size) {
    service->timeouts = priority_queue_create(buffer, size);
    circular_queue_initialize(&(service->events));
    int i;
    for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
        service->subscribers[i] = 0;
    }

    for (i = 0; i < MAX_TRAIN_IDS; ++i) {
        service->train_to_speed[i] = 0;
    }
}

int distanceservice_event(struct DistanceService *service, int *train, int *subscribers) {
    if (circular_queue_empty(&(service->events))) {
        return -1;
    }

    *train = (int)circular_queue_pop(&(service->events));
    int i;
    for (i = 0; i < MAX_SUBSCRIBERS; ++i) {
        subscribers[i] = service->subscribers[i];
    }

    return 0;
}

int distanceservice_notify(struct DistanceService *service, int time) {
    HeapPriorityQueue *queue = &(service->timeouts);
    while (priority_queue_size(queue)) {
        PriorityQueueElement element = priority_queue_peek(queue);
        if (element.priority <= time) {
            priority_queue_extract(queue);
            circular_queue_push(&(service->events), element.data);
            int train = (int)element.data;
            element.priority += speed_to_timeout(train, service->train_to_speed[train]);
            priority_queue_insert(queue, element);
        } else {
            break;
        }
    }

    return 0;
}

int distanceservice_timeout(struct DistanceService *service) {
    HeapPriorityQueue *queue = &(service->timeouts);
    if (!priority_queue_size(queue)) return -1;

    return (int)priority_queue_peek(queue).priority;
}

int distanceservice_update_train(struct DistanceService *service, int train, int speed) {
    HeapPriorityQueue *queue = &(service->timeouts);
    priority_queue_delete(queue, (void *)train);

    service->train_to_speed[train] = speed;

    if (speed > 0) {
        PriorityQueueElement element;
        element.priority = Time() + speed_to_timeout(train, speed);
        element.data = (void *)train;
        priority_queue_insert(queue, element);
    }

    return 0;
}

int distanceservice_subscribe(struct DistanceService *service, int tid) {
    int section = tid / 32;
    int bit = 1 << (tid % 32);

    service->subscribers[section] |= bit;

    return 0;
}

int distanceservice_unsubscribe(struct DistanceService *service, int tid) {
    int section = tid / 32;
    int bit  = 1 << (tid % 32);

    service->subscribers[section] &= ~bit;

    return 0;
}
