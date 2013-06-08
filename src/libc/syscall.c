#include <syscall.h>

#include <bwio.h>
#include <request.h>

int MyTid() {
	Request req;
	req.request = MY_TID;
	return syscall(&req);
}

int MyParentTid() {
    Request req;
    req.request = MY_PARENT_TID;
    return syscall(&req);
}

int Create(int priority, void(*code)()) {
    Request req;
    req.request = CREATE;
    req.args[0] = (void *)priority;
    req.args[1] = code;
    return syscall(&req);
}

void Pass() {
    Request req;
    req.request = PASS;
    syscall(&req);
}

void Exit() {
    Request req;
    req.request = EXIT;
    syscall(&req);
}

int Send(tid_t tid, char *msg, int msglen, char *reply, int replylen) {
//    tid_t me = MyTid();
//    bwprintf(COM2, "%d is sending a message to %d\n", me, tid);
    Request req;
    req.request = SEND;
    req.args[0] = (void *)tid;
    req.args[1] = msg;
    req.args[2] = (void *)msglen;
    req.args[3] = reply;
    req.args[4] = (void *)replylen;
    return syscall(&req);
}

int Receive(tid_t *tid, char *msg, int msglen) {
//    tid_t me = MyTid();
//    bwprintf(COM2, "%d is waiting for a message\n", me);
    Request req;
    req.request = RECEIVE;
    req.args[0] = tid;
    req.args[1] = msg;
    req.args[2] = (void *)msglen;
    return syscall(&req);
}

int Reply(tid_t tid, char *reply, int replylen) {
//    tid_t me = MyTid();
//    bwprintf(COM2, "%d is replying to %d\n", me, tid);
    Request req;
    req.request = REPLY;
    req.args[0] = (void *)tid;
    req.args[1] = reply;
    req.args[2] = (void *)replylen;
    return syscall(&req);
}

int AwaitEvent(int event) {
    Request req;
    req.request = AWAIT_EVENT;
    req.args[0] = (void *)event;
    return syscall(&req);
}

int WaitTid(int tid) {
    Request req;
    req.request = WAIT_TID;
    req.args[0] = (void *)tid;
    return syscall(&req);
}

void Shutdown() {
    Request req;
    req.request = SHUTDOWN;
    syscall(&req);
}
