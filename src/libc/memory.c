#include <memory.h>
#include <log.h>

#define HEAP_SIZE 1024 * 1024

static char heap[HEAP_SIZE];
static char *free;

void *memset(void *destination, unsigned char value, unsigned int len) {
    unsigned char *dst = (unsigned char *)destination;

    unsigned int i;
    for (i = 0; i < len; ++i) {
        *dst++ = value;
    }

    return destination;
}

void initialize_memory() {
    dlog("Initializing Memory System\n");
    free = heap;
    dlog("Initialized Memory System\n");
}

void *kmalloc(unsigned int size) {
    if (free + size > heap + HEAP_SIZE) {
        dlog("Heap unable to serve request for: %d\n", size);
        return 0;
    }

    void *ret = free;
    free += size;

    return ret;
}
