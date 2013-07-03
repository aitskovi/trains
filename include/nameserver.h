#ifndef _NAMESERVER_H_
#define _NAMESERVER_H_

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

/**
 * Register the task under the specified name.
 *
 *  0 Success
 * -1 Invalid NameServer Tid.
 * -2 NameServer Task Tid is not NameServer.
 */
int RegisterAs(char *name);

/**
 * Returns the tid of the named task.
 *
 * >=0 Tid
 *  -1 Invalid NameServer Tid.
 *  -2 NameServer Task Tid is not NameServer.
 *  -3 Invalid name
 */
int WhoIs(char *name);

void NameServer();

#endif
