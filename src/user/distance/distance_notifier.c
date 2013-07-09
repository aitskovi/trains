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
        msg.type = LOCATION_SERVER_MESSAGE;
        msg.ls_msg.type = LOCATION_TICK_REQUEST;
        Send(tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
        cuassert(rply.type == LOCATION_SERVER_MESSAGE, "Invalid Message Type");
        cuassert(rply.ls_msg.type == LOCATION_TICK_RESPONSE, "Invalid DistanceServer Reply");

        time += 1;
        DelayUntil(time);
    }

    Exit();
}
