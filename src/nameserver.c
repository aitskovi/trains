#define MAX_NAME_LENGTH 10
#define MAX_REGISTRATIONS 50

#include <bwio.h>
#include <strings.h>
#include <memory.h>
#include <syscall.h>
#include <task.h>
#include <scheduling.h>

struct Registration {
    char name[MAX_NAME_LENGTH];
    int tid;
};

static struct Registration registrations[MAX_REGISTRATIONS];

enum nameserver_operation {
    REGISTER_AS,
    WHO_IS,
    NUM_NAMESERVER_OPERATIONS,
};

struct NameServerRequest {
    enum nameserver_operation operation;
    char *data;
};

struct NameServerReply {
    int result;
};

static int nameserver_tid;

/**
 *  0 Means success
 * -1 Means Name was cut off.
 * -2 means Not Enough Slots
 */
int nameserver_register(char *name, int tid) {
    // Verify Name Length
    int name_length = strlen(name);
    bwprintf(COM2, "Registering name of length %d", name_length);
    if (name_length > MAX_NAME_LENGTH) return -1;

    int i;
    for (i = 0; i < MAX_REGISTRATIONS; ++i) {
        struct Registration *reg = &registrations[i];

        if (reg->tid == -1) {
            reg->tid = tid;
            memcpy(reg->name, name, name_length);
            return 0;
        }

        if (streq(reg->name, name)) {
            reg->tid = tid;
            return 0;
        }
    }

    return -2;
}

/**
 * Returns either the tid or -1 for not found.
 */
int nameserver_lookup(char *name) {
   int i; 
   for (i = 0; i < MAX_REGISTRATIONS; ++i) {
       struct Registration *reg = &registrations[i];
       if (reg->tid == -1) return -1;
       if (streq(reg->name, name)) return reg->tid;
   }
   return -1;
}

/**
 * Register the task under the specified name.
 *
 *  0 Success
 * -1 Invalid NameServer Tid.
 * -2 NameServer Task Tid is not NameServer.
 */
int RegisterAs(char *name) {
    struct NameServerRequest req;
    req.operation = REGISTER_AS;
    req.data = name;
    int reqlen = sizeof(struct NameServerRequest);

    struct NameServerReply reply;
    int replylen = sizeof(struct NameServerReply);
    int result = Send(nameserver_tid, (char *)&req, reqlen, (char *)&reply, replylen);
    if (result < 0) {
        return result;
    }

    return reply.result;
}

/**
 * Returns the tid of the named task.
 *
 * >=0 Tid
 *  -1 Invalid NameServer Tid.
 *  -2 NameServer Task Tid is not NameServer.
 *  -3 Invalid name
 */
int WhoIs(char *name) {
    struct NameServerRequest req;
    req.operation = WHO_IS;
    req.data = name;
    int reqlen = sizeof(struct NameServerRequest);

    struct NameServerReply reply;
    int replylen = sizeof(struct NameServerReply);
    int result = Send(nameserver_tid, (char *)&req, reqlen, (char *)&reply, replylen);
    if (result < 0) {
        return result;
    }

    return reply.result;
}

void NameServer() {
    bwprintf(COM2, "NameServer: Starting Up\n");
    int src;
    struct NameServerRequest req;
    int reqlen = sizeof(struct NameServerRequest);

    struct NameServerReply reply;
    int replylen = sizeof(struct NameServerReply);

    for(;;) {
        Receive(&src, (char *)&req, reqlen);

        switch(req.operation) {
            case REGISTER_AS: 
                bwprintf(COM2, "NameServer: Recieved RegisterAs\n");
                bwprintf(COM2, "NameServer: Registering %d as %s\n", src, req.data);
                reply.result = nameserver_register(req.data, src);
                break;
            case WHO_IS:
                bwprintf(COM2, "NameServer: Recieved WhoIs\n");
                bwprintf(COM2, "NameServer: Looking Up %s\n", req.data);
                reply.result = nameserver_lookup(req.data);
                break;
            default:
                bwprintf(COM2, "NameServer: Recieved Invalid Request\n");
                break;
        }

        Reply(src, (char *)&reply, replylen);
    }

    bwprintf(COM2, "NameServer: Exit\n");
}

int initialize_nameserver() {
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

    // Initalize the NameServerTask
    bwprintf(COM2, "Creating NameServer\n", nameserver_tid);
    Task *task = task_create(NameServer, -1, HIGH);
    nameserver_tid = task->tid;
    make_ready(task);
    bwprintf(COM2, "NameServer created in task <%d>\n", nameserver_tid);
    return 0;
}
