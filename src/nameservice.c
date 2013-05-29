#include <nameservice.h>

#include <strings.h>
#include <memory.h>

#define MAX_REGISTRATIONS 50
#define MAX_NAME_LENGTH 10

struct Registration {
    char name[MAX_NAME_LENGTH];
    int tid;
};

static struct Registration registrations[MAX_REGISTRATIONS];

int nameservice_register(char *name, int tid) {
    // Verify Name Length
    int name_length = strlen(name);
    if (name_length > MAX_NAME_LENGTH) return -1;

    int i;
    for (i = 0; i < MAX_REGISTRATIONS; ++i) {
        struct Registration *reg = &registrations[i];

        // Found an empty slot for the registration.
        if (reg->tid == -1) {
            reg->tid = tid;
            memcpy(reg->name, name, name_length);
            return 0;
        }

        // Overwrote an existing registration.
        if (streq(reg->name, name)) {
            reg->tid = tid;
            return 0;
        }
    }

    return -2;
}

int nameservice_lookup(char *name) {
   int i; 
   for (i = 0; i < MAX_REGISTRATIONS; ++i) {
       struct Registration *reg = &registrations[i];
       if (reg->tid == -1) return -1;
       if (streq(reg->name, name)) return reg->tid;
   }
   return -1;
}

void initialize_nameservice() {
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
