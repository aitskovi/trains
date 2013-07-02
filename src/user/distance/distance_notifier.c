#include <distance_notifier.h>

#include <nameserver.h>
#include <clock_server.h>
#include <encoding.h>
#include <syscall.h>
#include <dassert.h>

void distance_notifier() {
    // Find the sensor server.
    int distance_server_tid = -2;
    do {
        distance_server_tid = WhoIs("DistanceServer");
    } while (distance_server_tid < 0);

    int time = Time();
    struct Message msg, rply;
    for(;;) {
        msg.type = DISTANCE_SERVER_MESSAGE;
        msg.ds_msg.type = DISTANCE_TIMEOUT_REQUEST;
        msg.ds_msg.time = time;
        Send(distance_server_tid, (char *)&msg, sizeof(msg), (char *)&rply, sizeof(rply));
        cuassert(rply.type == DISTANCE_SERVER_MESSAGE, "Invalid Message Type");
        cuassert(rply.ds_msg.type == DISTANCE_TIMEOUT_RESPONSE, "Invalid DistanceServer Reply");

        time = rply.ds_msg.time;
        DelayUntil(time);
    }

    Exit();
}
