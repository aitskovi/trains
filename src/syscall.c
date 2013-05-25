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
