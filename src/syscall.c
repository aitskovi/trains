#include <syscall.h>

#include <bwio.h>
#include <request.h>

int syscall(Request *req) {
    asm("mov r0, %[request]" "\n\t"
    	"swi 0" "\n\t"
    	:
    	: [request] "r" (req));
}

int MyTid(unsigned int specialNumber) {
	Request req;
	req.request = MY_TID;
	return syscall(&req);
}

int Create(int priority, void(*code)()) {
    Request req;
    req.request = MY_TID;
    req.args[0] = priority;
    req.args[1] = code;
    return syscall(&req);
}
