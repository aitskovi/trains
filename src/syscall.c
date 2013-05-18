#include <bwio.h>

int syscall(unsigned int number) {
    asm("mov r0, #255" "\n\t"
    	"swi 0" "\n\t");

    return 0;
}

void software_interrupt() {
    unsigned int number;
    asm("mov %[syscall_number], r7" : [syscall_number] "=r" (number) :);
    bwprintf(COM2, "Recieved System Call: %d\n\r", number);
}

