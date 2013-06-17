#include <verify.h>

int main() {
    verify_initialize();

    vsuite_run(syscalls_suite());
    vsuite_run(messaging_suite());
    vsuite_run(circular_queue_suite());
    vsuite_run(memory_suite());

    return 0;
}
