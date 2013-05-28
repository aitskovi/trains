#include <syscall.h>

#include <bwio.h>
#include <request.h>

int syscall(Request *req) {
    asm("mov r0, %[request]" "\n\t"
    	"swi 0" "\n\t"
    	:
    	: [request] "r" (req));

    register int retval asm("r0");
    return retval;
}

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

int Send(int tid, char *msg, int msglen, char *reply, int replylen) {
    Request req;
    req.request = SEND;
    req.args[0] = (void *)tid;
    req.args[1] = msg;
    req.args[2] = (void *)msglen;
    req.args[3] = reply;
    req.args[4] = (void *)replylen;
    return syscall(&req);
}

int Receive(int *tid, char *msg, int msglen) {
    Request req;
    req.request = RECEIVE;
    req.args[0] = tid;
    req.args[1] = msg;
    req.args[2] = (void *)msglen;
    return syscall(&req);
}

int Reply(int tid, char *reply, int replylen) {
    Request req;
    req.request = REPLY;
    req.args[0] = (void *)tid;
    req.args[1] = reply;
    req.args[2] = (void *)replylen;
    return syscall(&req);
}

