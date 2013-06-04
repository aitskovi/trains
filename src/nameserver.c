#include <nameserver.h>

#include <nameservice.h>
#include <bwio.h>
#include <syscall.h>
#include <task.h>
#include <scheduling.h>

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

    struct NameServerReply reply;
    int result = Send(nameserver_tid, (char *)&req, sizeof(req), (char *)&reply, sizeof(reply));
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

    struct NameServerReply reply;
    int result = Send(nameserver_tid, (char *)&req, sizeof(req), (char *)&reply, sizeof(reply));
    if (result < 0) {
        return result;
    }

    return reply.result;
}

void NameServer() {
    struct NameService service;
    nameservice_initialize(&service);
//    bwprintf(COM2, "NameServer: Starting Up\n");
    int src;
    struct NameServerRequest req;

    struct NameServerReply reply;

    for(;;) {
        Receive(&src, (char *)&req, sizeof(req));

        switch(req.operation) {
            case REGISTER_AS: 
//                bwprintf(COM2, "NameServer: Received RegisterAs\n");
//                bwprintf(COM2, "NameServer: Registering %d as %s\n", src, req.data);
                reply.result = nameservice_register(&service, req.data, src);
                break;
            case WHO_IS:
//                bwprintf(COM2, "NameServer: Received WhoIs\n");
//                bwprintf(COM2, "NameServer: Looking Up %s\n", req.data);
                reply.result = nameservice_lookup(&service, req.data);
                break;
            default:
//                bwprintf(COM2, "NameServer: Received Invalid Request\n");
                break;
        }

        Reply(src, (char *)&reply, sizeof(reply));
    }

    bwprintf(COM2, "NameServer: Exit\n");
}

void initialize_nameserver() {
    // Initalize the NameServerTask
//    bwprintf(COM2, "Creating NameServer\n", nameserver_tid);
    Task *task = task_create(NameServer, -1, HIGH);
    nameserver_tid = task->tid;
    make_ready(task);
//    bwprintf(COM2, "NameServer created in task <%d>\n", nameserver_tid);

}
