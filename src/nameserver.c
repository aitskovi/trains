#include <nameserver.h>

#include <dassert.h>
#include <nameservice.h>
#include <bwio.h>
#include <syscall.h>
#include <task.h>
#include <scheduling.h>
#include <log.h>

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
    cuassert(result >= 0, "RegisterAs Failed");
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
    cuassert(result >= 0, "WhoIs Failed");
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
                Reply(src, (char *)&reply, sizeof(reply));
                break;
            case WHO_IS:
                dlog("NameServer: Received WhoIs\n");
                dlog("NameServer: Looking Up %s\n", req.data);
                int tid = nameservice_lookup(&service, req.data);

                if (tid > 0) {
                    // We found a reply.
                    reply.result = tid;
                    Reply(src, (char *)&reply, sizeof(reply));
                } else {
                    // Otherwise block.
                    if (tid == -1) nameservice_register(&service, req.data, WAITING_TID);
                    nameservice_wait(&service, req.data, src);
                }
                break;
            default:
                ulog("NameServer: Received Invalid Request\n");
                break;
        }

    }

    Exit();
}
