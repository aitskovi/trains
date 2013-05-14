#include <bwio.h>

int main() {
    bwsetfifo(COM2, OFF);
    bwprintf(COM2, "Hello World!");
    return 0;
}
