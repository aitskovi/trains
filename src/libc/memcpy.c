/*
 * memcpy.c
 *
 *  Created on: Jun 6, 2013
 *      Author: aianus
 */


#include <memcpy.h>

// Must be called with len % 4 == 0
void *memcpy4(void *destination, void *source, unsigned int len) {
    int *dst = (int *) destination;
    int *src = (int *) source;

    while (len) {
        *dst++ = *src++;
        len -= 4;
    }

    return destination;
}

void *memcpy(void *destination, void *source, unsigned int len) {
    char *dst = (char *)destination;
    char *src = (char *)source;

    while (len % 4) {
        *dst++ = *src++;
        len--;
    }

    // Now the length is a multiple of the word size
    return memcpy4((void *) dst, (void *) src, len);
}

/*
void *memcpy(void *destination, void *source, unsigned int len) {
    char *dst = (char *)destination;
    char *src = (char *)source;

    unsigned int i;
    for (i = 0; i < len; ++i) {
        *dst++ = *src++;
    }

    return destination;
}
*/
