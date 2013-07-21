/*
 * ring_buffer.h
 *
 *  Created on: 2013-05-10
 *      Author: alex
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#define RING_BUFFER_SIZE 16384 * 4

struct ring_buffer {
    unsigned char buf[RING_BUFFER_SIZE];
    unsigned int read_count;
    unsigned int write_count;
};

int ring_buffer_read(struct ring_buffer *buffer, unsigned char *data, unsigned int size);
int ring_buffer_write(struct ring_buffer *buffer, unsigned char *data, unsigned int size);
void ring_buffer_initialize(struct ring_buffer *buffer);
unsigned int ring_buffer_size(struct ring_buffer *buffer);
int ring_buffer_empty(struct ring_buffer *buffer);

#endif /* RING_BUFFER_H_ */
