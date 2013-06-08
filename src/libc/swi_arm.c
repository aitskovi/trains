#include <syscall.h>
#include <request.h>

int syscall(Request *req) {
    asm("mov r0, %[request]" "\n\t"
    	"swi 0" "\n\t"
    	:
    	: [request] "r" (req));

    register int retval asm("r0");
    return retval;
}
