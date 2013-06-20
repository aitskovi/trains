#include <read_service.h>

#include <circular_queue.h>
#include <dassert.h>
#include <serial.h>
#include <syscall.h>
#include <task.h>

void readservice_initialize(struct ReadService *service, int channel) {
    service->channel = channel;
    circular_queue_initialize(&service->tids);
    circular_queue_initialize(&service->chars);
}

/**
 * Enqueue a character into the service.
 */
int readservice_putc(struct ReadService *service, char c) {
    dassert(service != 0, "Invalid Service");

    return circular_queue_push(&service->chars, (void *)(int)c);
}

/**
 * Enqueue a task into the service.
 */
int readservice_getc(struct ReadService *service, tid_t tid) {
    dassert(service != 0, "Invalid Service");

    return circular_queue_push(&service->tids, (void *)tid);
}

/**
 * Attempt to flush a character from the service.
 */
int readservice_flush(struct ReadService *service) {
    while(!circular_queue_empty(&service->chars) && !circular_queue_empty(&service->tids)) {
        tid_t tid = (tid_t) circular_queue_pop(&service->tids);
        char c = (char) circular_queue_pop(&service->chars);
        ReadMessage rply;
        rply.type = GETC_RESPONSE;
        rply.data = c;
        Reply(tid, (char *)&rply, sizeof(rply));
    }

    return 0;
}

