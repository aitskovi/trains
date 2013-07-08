#ifndef _WRITE_SERVICE_H_
#define _WRITE_SERVICE_H_

#include <ring_buffer.h>

struct WriteService {
    int channel;
    int writable;
    struct ring_buffer buf;
};

void writeservice_initialize(struct WriteService *service, int channel);

/**
 * Enqueue a string into the service.
 */
void writeservice_enqueue(struct WriteService *service, char *str, unsigned int size);

/**
 * Attempt to flush a character from the service.
 */
int writeservice_flush(struct WriteService *service);

/**
 * Notify the service that it can write to the UART.
 */
int writeservice_writable(struct WriteService *service);

#endif
