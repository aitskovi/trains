#include <verify.h>

int main() {
    verify_initialize();

    log("\n");
    vsuite_run(syscalls_suite());
    vsuite_run(messaging_suite());
    vsuite_run(circular_queue_suite());
    vsuite_run(memory_suite());
    vsuite_run(priority_queue_suite());
    vsuite_run(scheduling_suite());

    return 0;
}
