#include <write_service.h>

#include <ring_buffer.h>
#include <dassert.h>
#include <uart.h>
#include <ts7200.h>

static struct ring_buffer com2_buffer;
static struct ring_buffer com1_buffer;

void writeservice_initialize(struct WriteService *service, int channel) {
    service->channel = channel;
    if (channel == COM1) service->buf = &com1_buffer;
    else service->buf = &com2_buffer;
    service->writable = 0;
    ring_buffer_initialize(service->buf);
}

/**
 * Enqueue a string into the service.
 */
void writeservice_enqueue(struct WriteService *service, char *str, unsigned int size) {
    dassert(service != 0, "Invalid Service");

    ckassert(ring_buffer_write(service->buf, (unsigned char *) str, size) >= 0, "WriteService RingBuf Overrun");
}

/**
 * Attempt to flush a character from the service.
 */
int writeservice_flush(struct WriteService *service) {
    dassert(service != 0, "Invalid Service");

    if (!service->writable) return ring_buffer_size(service->buf);
    else if (ring_buffer_empty(service->buf)) return ring_buffer_size(service->buf);

    service->writable = 0;
    char c;
    ring_buffer_read(service->buf, (unsigned char *) &c, 1);
    uart_write(service->channel, c);

    return ring_buffer_size(service->buf);
}

/**
 * Notify the service that it can write to the UART.
 */
int writeservice_writable(struct WriteService *service) {
    dassert(service != 0, "Invalid Service");

    service->writable = 1;

    return 0;
}

