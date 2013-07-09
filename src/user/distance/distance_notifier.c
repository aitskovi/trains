#include <distance_notifier.h>

#include <nameserver.h>
#include <clock_server.h>
#include <encoding.h>
#include <syscall.h>
#include <dassert.h>

void distance_notifier() {
    int tid = MyParentTid();

    int time = Time();
    struct Message msg, rply;
    for(;;) {
        msg.type = DISTANCE_SERVER_MESSAGE;
        msg.ds_msg.type = DISTANCE_TIMEOUT_REQUEST;
        Send(tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
        cuassert(rply.type == DISTANCE_SERVER_MESSAGE, "Invalid Message Type");
        cuassert(rply.ds_msg.type == DISTANCE_TIMEOUT_RESPONSE, "Invalid DistanceServer Reply");

        time += 1;
        DelayUntil(time);
    }

    Exit();
}
