#include <bwio.h>
#include <switch.h>

void interrupt() {
    bwprintf(COM2, "Hello Interrupt!");
}

int main() {
    bwsetfifo(COM2, OFF);
    bwprintf(COM2, "Hello World!\n\r");

    void (**syscall)() = 0x28;
    *syscall = interrupt;
    //syscall = interrupt;
    //*(0x28) = syscall;

    asm("swi 0");

    return 0;
}
