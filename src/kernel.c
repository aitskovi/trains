#include <bwio.h>
#include <switch.h>
#include <syscall.h>

void interrupt() {
    bwprintf(COM2, "Hello Interrupt!");
}

int main() {
    bwsetfifo(COM2, OFF);
    bwprintf(COM2, "Hello World!\n\r");

    void (**syscall_handler)() = 0x28;
    *syscall_handler = software_interrupt;
    //syscall = interrupt;
    //*(0x28) = syscall;
    
    /*
    struct Request r;
    r.syscall = 1;
    sys(&r);
    */

    //asm("swi 0");
    syscall(1);

    return 0;
}
