/*
 * memcpy.c
 *
 *  Created on: Jun 6, 2013
 *      Author: aianus
 */


#include <memcpy.h>
#include <log.h>

// Must be called with len % 4 == 0
void *memcpy4(void *destination, void *source, unsigned int len) {
    int *dst = (int *) destination;
    int *src = (int *) source;

    if ((unsigned int) destination % 4
            || (unsigned int) source % 4
            || len % 4) {
        log("memcpy4 called with invalid arguments!\n");
    }

    while (len) {
        *dst++ = *src++;
        len -= 4;
    }

    return destination;
}

void *memcpy(void *destination, void *source, unsigned int len) {
    char *dst = (char *)destination;
    char *src = (char *)source;

    if ((unsigned int) destination % 4
            || (unsigned int) source % 4) {
        log("Memcpy called with unaligned pointers!");
    } else {
        unsigned int bulk_size = len - len % 4;
        memcpy4(destination, source, bulk_size);
        dst += bulk_size;
        src += bulk_size;
        len -= bulk_size;
    }

    unsigned int i;
    for (i = 0; i < len; ++i) {
        *dst++ = *src++;
    }

    return destination;
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
