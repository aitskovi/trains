#include <nameservice.h>

#include <string.h>
#include <memory.h>
#include <nameserver.h>
#include <syscall.h>
#include <dassert.h>

int nameservice_register(struct NameService *service, char *name, int tid) {
    // Verify Name Length
    int name_length = strlen(name);
    if (name_length > MAX_NAME_LENGTH - 1) return -1;

    struct Registration *registrations = service->registrations;

    int i;
    for (i = 0; i < MAX_REGISTRATIONS; ++i) {
        struct Registration *reg = &registrations[i];

        // Found an empty slot for the registration.
        if (reg->tid == -1) {
            reg->tid = tid;
            // TODO write strcpy/strcpyn?
            memcpy(reg->name, name, name_length + 1);
            return 0;
        }

        // Overwrote an existing registration.
        if (streq(reg->name, name)) {
            reg->tid = tid;

            // Unblock all waiters.
            struct NameServerReply reply;
            reply.result = reg->tid;
            int j;
            for (j = 0; j < reg->num_waiters; ++j) {
                int waiter = reg->waiters[j];
                Reply(waiter, (char *)&reply, sizeof(reply));
            }

            return 0;
        }
    }

    return -2;
}

int nameservice_lookup(struct NameService *service, char *name) {
   struct Registration *registrations = service->registrations;

   int i; 
   for (i = 0; i < MAX_REGISTRATIONS; ++i) {
       struct Registration *reg = &registrations[i];
       if (reg->tid == -1) return -1;
       if (streq(reg->name, name)) return reg->tid;
   }
   return -1;
}

int nameservice_wait(struct NameService *service, char *name, int tid) {
    int i; 
    for (i = 0; i < MAX_REGISTRATIONS; ++i) {
        struct Registration *reg = &service->registrations[i];
        if (reg->tid == -1) return -1;
        if (streq(reg->name, name)) {
            reg->waiters[reg->num_waiters] = tid;
            reg->num_waiters++;
            cuassert(reg->num_waiters < MAX_WAITERS, "WARNING: Too Many Registration Waiters");
        }
    }
    return 0;
}

void nameservice_initialize(struct NameService *service) {
    struct Registration *registrations = service->registrations;

    // Initialize the Registration Structs.
    int i;
    for (i = 0; i < MAX_REGISTRATIONS; ++i) {
        char *name = registrations[i].name;
        int j;
        for (j = 0; j < MAX_NAME_LENGTH; ++j) {
            name[j] = 0;
        }
        registrations[i].tid = -1;
    }
}
