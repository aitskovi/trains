#include <write_service.h>

#include <circular_queue.h>
#include <dassert.h>
#include <uart.h>

void writeservice_initialize(struct WriteService *service, int channel) {
    service->channel = channel;
    service->writable = 0;
    circular_queue_initialize(&service->queue);
}

/**
 * Enqueue a character into the service.
 */
int writeservice_enqueue(struct WriteService *service, char c) {
    dassert(service != 0, "Invalid Service");

    return circular_queue_push(&service->queue, (void *)(int)c);
}

/**
 * Attempt to flush a character from the service.
 */
int writeservice_flush(struct WriteService *service) {
    dassert(service != 0, "Invalid Service");

    if (!service->writable) return -2;
    else if (circular_queue_empty(&service->queue)) return 0;

    service->writable = 0;
    log("Writing to the uart\n");
    return uart_write(service->channel, (char)circular_queue_pop(&service->queue));
}

/**
 * Notify the service that it can write to the UART.
 */
int writeservice_writable(struct WriteService *service) {
    dassert(service != 0, "Invalid Service");

    service->writable = 1;

    return 0;
}

