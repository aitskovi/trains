#include <verify.h>

int main() {
    verify_initialize();

    vsuite_run(syscalls_suite());
    vsuite_run(messaging_suite());

    return 0;
}
