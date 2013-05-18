#include <bwio.h>
#include <request.h>

#define MY_TID_CALL_NO 1

int syscall(Request *req) {
    asm("mov r0, %[request]" "\n\t"
    	"swi 0" "\n\t"
    	:
    	: [request] "r" (req));
}

int MyTid(unsigned int specialNumber) {
	Request req;
	req.request = MY_TID_CALL_NO;
	req.args[0] = specialNumber;
	return syscall(&req);
}
