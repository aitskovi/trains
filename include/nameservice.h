#ifndef _NAMESERVICE_H_
#define _NAMESERVICE_H_

#define MAX_NAME_LENGTH 40
#define MAX_REGISTRATIONS 50
#define MAX_WAITERS 20
#define WAITING_TID -2

struct Registration {
    char name[MAX_NAME_LENGTH];
    int tid;

    int num_waiters;
    int waiters[MAX_WAITERS];
};

struct NameService {
    struct Registration registrations[MAX_REGISTRATIONS];
};

/**
 * Initialize the nameservice.
 */
void nameservice_initialize(struct NameService *service);

/**
 * Register a name with the nameservice.
 *
 *  0 Means success
 * -1 Means Name was cut off.
 * -2 means Not Enough Slots
 */
int nameservice_register(struct NameService *service, char *name, int tid);

/**
 * Lookup a name in the nameservice.
 *
 * Returns either the tid or -1 for not found.
 */
int nameservice_lookup(struct NameService *service, char *name);

int nameservice_wait(struct NameService *service, char *name, int tid);

#endif
