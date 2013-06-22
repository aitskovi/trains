/*
 * ring_buffer.c
 *
 *  Created on: 2013-05-10
 *      Author: alex
 */

#include <ring_buffer.h>

int ring_buffer_write(struct ring_buffer *buffer, unsigned char *data, unsigned int size) {
    // Check if we'd overrun our queue
    if (size > RING_BUFFER_SIZE - ring_buffer_size(buffer)) {
        return -1;
    }

    unsigned int i;
    for (i = 0; i < size; i++ ) {
        buffer->buf[buffer->write_count++ % RING_BUFFER_SIZE] = data[i];
    }

    return size;
};

int ring_buffer_read(struct ring_buffer *buffer, unsigned char *data, unsigned int size) {
    unsigned int available = buffer->write_count - buffer->read_count;

    unsigned int i = 0;
    while (i < size && i < available) {
        data[i] = buffer->buf[buffer->read_count++ % RING_BUFFER_SIZE];
        i++;
    }

    return i;
}

void ring_buffer_initialize(struct ring_buffer *buffer) {
    buffer->read_count = 0;
    buffer->write_count = 0;
}

unsigned int ring_buffer_size(struct ring_buffer *buffer) {
    return buffer->write_count - buffer->read_count;
}

int ring_buffer_empty(struct ring_buffer *buffer) {
    return buffer->write_count == buffer->read_count;
}
