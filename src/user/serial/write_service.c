#include <write_service.h>

#include <circular_queue.h>
#include <dassert.h>
#include <uart.h>

void writeservice_initialize(struct WriteService *service, int channel) {
    service->channel = channel;
    service->writable = 0;
    ring_buffer_initialize(&service->buf);
}

/**
 * Enqueue a string into the service.
 */
void writeservice_enqueue(struct WriteService *service, char *str, unsigned int size) {
    dassert(service != 0, "Invalid Service");

    cuassert(ring_buffer_write(&service->buf, (unsigned char *) str, size), "WriteService RingBuf Overrun");
}

/**
 * Attempt to flush a character from the service.
 */
int writeservice_flush(struct WriteService *service) {
    dassert(service != 0, "Invalid Service");

    if (!service->writable) return -2;
    else if (ring_buffer_empty(&service->buf)) return 0;

    service->writable = 0;
    char c;
    ring_buffer_read(&service->buf, (unsigned char *) &c, 1);
    return uart_write(service->channel, c);
}

/**
 * Notify the service that it can write to the UART.
 */
int writeservice_writable(struct WriteService *service) {
    dassert(service != 0, "Invalid Service");

    service->writable = 1;

    return 0;
}

