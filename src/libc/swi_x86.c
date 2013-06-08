#include <syscall.h>

#include <request.h>

/** Fake syscall so we can link against this in tests **/
int syscall(Request *req) {
    return 0;
}
