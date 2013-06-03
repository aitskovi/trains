#ifndef _MEMORY_H_
#define _MEMORY_H_

#define HEAP_SIZE 1024 * 1024

/**
 * Memory Utilities.
 */
void *memcpy(void *destination, void *source, unsigned int len);
void *memset(void *destination, unsigned char value, unsigned int len);

/**
 * Memory Management
 */
void initialize_memory();

/**
 * Allocate some space from our heap.
 *
 * ONLY USE THIS INSIDE THE KERNEL because interrupts should be off.
 *
 * Allocated space cannot be deallocated at this point for simplicity.
 *
 * @return Some adequately sized memory or 0 if not enough space is available.
 */
void *kmalloc(unsigned int size);

#endif

