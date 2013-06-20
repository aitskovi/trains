#ifndef _WRITE_SERVICE_H_
#define _WRITE_SERVICE_H_

#include <circular_queue.h>

struct WriteService {
    int channel;
    int writable;
    struct circular_queue queue;
};

void writeservice_initialize(struct WriteService *service, int channel);

/**
 * Enqueue a character into the service.
 */
int writeservice_enqueue(struct WriteService *service, char c);

/**
 * Attempt to flush a character from the service.
 */
int writeservice_flush(struct WriteService *service);

/**
 * Notify the service that it can write to the UART.
 */
int writeservice_writable(struct WriteService *service);

#endif
