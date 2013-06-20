#include <nameserver.h>

#include <nameservice.h>
#include <bwio.h>
#include <syscall.h>
#include <task.h>
#include <scheduling.h>
#include <log.h>

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
    if (nameserver_tid < 0) {
        return -1;
    }

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
    if (nameserver_tid < 0) {
        return -1;
    }

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

    nameserver_tid = MyTid();

    int src;
    struct NameServerRequest req;
    struct NameServerReply reply;

    for(;;) {
        Receive(&src, (char *)&req, sizeof(req));

        switch(req.operation) {
            case REGISTER_AS: 
                dlog("NameServer: Received RegisterAs\n");
                dlog("NameServer: Registering %d as %s\n", src, req.data);
                reply.result = nameservice_register(&service, req.data, src);
                break;
            case WHO_IS:
                dlog("NameServer: Received WhoIs\n");
                dlog("NameServer: Looking Up %s\n", req.data);
                reply.result = nameservice_lookup(&service, req.data);
                break;
            default:
                log("NameServer: Received Invalid Request\n");
                break;
        }

        Reply(src, (char *)&reply, sizeof(reply));
    }

    bwprintf(COM2, "NameServer: Exit\n");
}
