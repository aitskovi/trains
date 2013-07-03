#ifndef _READ_SERVICE_H_
#define _READ_SERVICE_H_

#include <circular_queue.h>

struct ReadService {
    int channel;

    // Task List.
    struct circular_queue tids;

    // Character List.
    struct circular_queue chars;
};

void readservice_initialize(struct ReadService *service, int channel);

/**
 * Enqueue a character into the service.
 */
int readservice_putc(struct ReadService *service, char c);

/**
 * Enqueue a task waiting for a character into the service.
 */
int readservice_getc(struct ReadService *service, int tid);

/**
 * Attempt to flush a character from the service.
 */
int readservice_flush(struct ReadService *service);

/**
 * Clear data in the service.
 */
void readservice_clear(struct ReadService *service);


#endif
