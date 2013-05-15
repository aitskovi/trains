#ifndef _KMEMORY_H_
#define _KMEMORY_H_

/**
 * Allocate a fixed chunk of memory.
 */
void *kmalloc(size_t size);

/**
 * Deallocate a fixed chunk of memory given by kmalloc.
 */
void *kfree(void *ptr);

#endif
