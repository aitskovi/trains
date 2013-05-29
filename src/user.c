#include <user.h>

#include <bwio.h>
#include <syscall.h>
#include <task.h>
#include <time.h>
#include <nameserver.h>

void first() {
    int tid;
    tid = Create(LOW, second);
    bwprintf(COM2, "Created: <%d>\n", tid);
    tid = Create(LOW, second);
    bwprintf(COM2, "Created: <%d>\n", tid);
    tid = Create(HIGH, second);
    bwprintf(COM2, "Created: <%d>\n", tid);
    tid = Create(HIGH, second);
    bwprintf(COM2, "Created: <%d>\n", tid);
    bwprintf(COM2, "First: exiting\n");
    Exit();
}

void second() {
    int tid = MyTid();
    int parent_tid = MyParentTid();
    bwprintf(COM2, "My Tid: %d, My Parent Tid: %d\n", tid, parent_tid);
    Pass();
    bwprintf(COM2, "My Tid: %d, My Parent Tid: %d\n", tid, parent_tid);
    Exit();
}

void kernel_timing() {
    Timer timer;

    unsigned int i;
    for (i = 0; i < 4; ++i) {
        timer_reset(&timer);
        Pass();
        Time elapsed = timer_elapsed(&timer);
        bwprintf(COM2, "Round trip took %u usec\n", elapsed.useconds);
    }

    Exit();
}

void producer() {
    char *msg = "Hello!";
    int msglen = 7;
    char reply[5];
    int replylen = 5;
    bwprintf(COM2, "Producer: Sending %s\n", msg);
    int result = Send(2, msg, msglen, reply, replylen);
    bwprintf(COM2, "Producer: Send Result: %d\n", result);
    bwprintf(COM2, "Producer: Reply: %s\n", reply);
    Exit();
}

void consumer() {
    int src;
    char msg[7];
    int msglen = 7;
    bwprintf(COM2, "Consumer: Receiving\n");
    int length = Receive(&src, msg, msglen);
    bwprintf(COM2, "Consumer: Received Length %d\n", length);
    bwprintf(COM2, "Consumer: Recieved Message from %d\n", src);
    bwprintf(COM2, "Consumer: Received Msg: %s\n", msg);

    char *reply = "Hey!";
    int replylength = 5;
    bwprintf(COM2, "Consumer: Replying %s\n", reply);
    int result = Reply(src, reply, replylength);
    bwprintf(COM2, "Consumer: Reply Result %d\n", result);
    Exit();
}

void communication() {
    int tid;
    tid = Create(LOW, producer);
    bwprintf(COM2, "Created Producer: <%d>\n", tid);
    tid = Create(LOW, consumer);
    bwprintf(COM2, "Created Consumer: <%d>\n", tid);
    bwprintf(COM2, "Communication: exiting\n");
    Exit();
}

void registration() {
    bwprintf(COM2, "Registration: Starting\n");
    bwprintf(COM2, "Registration: Registering as Driver\n");
    RegisterAs("Driver");
    bwprintf(COM2, "Registration: Registered as Driver\n");

    bwprintf(COM2, "Registration: Looking up Driver\n");
    int tid = WhoIs("Driver");
    bwprintf(COM2, "Registration: Driver is %d\n", tid);

    bwprintf(COM2, "Registration: Exiting\n");
    Exit();
}
