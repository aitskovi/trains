#include <bwio.h>

int syscall(unsigned int number) {
    asm("mov r7, %[syscall_number]" "\n\t"
        "swi 0" "\n\t"
        :
        : [syscall_number] "r" (number)
        : "r7");

    return 0;
}

void software_interrupt() {
    unsigned int number;
    asm("mov %[syscall_number], r7" : [syscall_number] "=r" (number) :);
    bwprintf(COM2, "Recieved System Call: %d\n\r", number);
}

