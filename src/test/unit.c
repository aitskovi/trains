#include <verify.h>

int main() {
    verify_initialize();

    vsuite_run(syscalls_suite());

    return 0;
}
