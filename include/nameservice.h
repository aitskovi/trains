#ifndef _NAMESERVICE_H_
#define _NAMESERVICE_H_

/**
 * Initialize the nameservice.
 */
void initialize_nameservice();

/**
 * Register a name with the nameservice.
 *
 *  0 Means success
 * -1 Means Name was cut off.
 * -2 means Not Enough Slots
 */
int nameservice_register(char *name, int tid);

/**
 * Lookup a name in the nameservice.
 *
 * Returns either the tid or -1 for not found.
 */
int nameservice_lookup(char *name);

#endif
