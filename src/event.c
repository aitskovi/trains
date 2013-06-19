#include <event.h>

#include <circular_queue.h>
#include <interrupt.h>
#include <log.h>
#include <task.h>
#include <uart.h>

#define INVALID_EVENT -1
#define CORRUPTED_VOLATILE_DATA -2

#define NO_WAITERS -1
#define TOO_MANY_WAITERS -4

static Task *waiters[NUM_EVENTS];
static struct circular_queue queues[NUM_EVENTS];

int kawait(Task *task, int event) {
   // Verify the event is valid.
   if (!is_valid_event(event)) {
       task_set_return_value(task, INVALID_EVENT);
       make_ready(task);
       return INVALID_EVENT;
   }

   if (waiters[event]) {
       task_set_return_value(task, TOO_MANY_WAITERS);
       make_ready(task);
       return TOO_MANY_WAITERS;
   }

   struct circular_queue *queue = &queues[event];
   if (circular_queue_empty(queue)) {
       if (event != 0) dlog("AwaitEvent: Blocking %d\n", event);
       waiters[event] = task;
       task->state = EVT_BLOCKED;
   } else {
       if (event != 0) dlog("AwaitEvent: Data Available %d\n", event);
       int data = (int)circular_queue_pop(queue);
       task_set_return_value(task, data);
       make_ready(task);
   }

   return 0;
}

int kevent(int event, int data) {
    if (event < 0 || event >= NUM_EVENTS) {
        dlog("Invalid Event %d\n", event);
        return -1;
    }

    if (event != 0) log("Recieved Event: %d\n", event);

    if (!waiters[event]) {
        struct circular_queue *queue = &queues[event];
        int error = circular_queue_push(queue, (void *)data);
        if (error) {
            dlog("Filled queue for event:%d\n", event);
        }
        return NO_WAITERS;
    }

    if (event != 0) dlog("AwaitEvent: Unblocking %d\n", event);
    Task *task = waiters[event];
    task_set_return_value(task, data);
    make_ready(task);

    waiters[event] = 0;
    
    return 0;
}

int is_valid_event(int event) {
   return (event >= 0 || event < NUM_EVENTS);
}

int enable_event(int event) {
    if (!is_valid_event(event)) return INVALID_EVENT;

    switch(event) {
        case TIMER_3_EVENT:
            enable_interrupt(TIMER_3_INTERRUPT);
            break;
        case UART_2_TX_EVENT:
            enable_interrupt(UART_2_INTERRUPT);
            uart_enable_interrupt(COM2, T_INTERRUPT);
            break;
        case UART_1_TX_EVENT:
            enable_interrupt(UART_1_INTERRUPT);
            uart_enable_interrupt(COM1, T_INTERRUPT);
            break;
        case UART_1_CTS_EVENT:
            enable_interrupt(UART_1_INTERRUPT);
            uart_enable_interrupt(COM1, MS_INTERRUPT);
            break;
        case UART_1_RCV_EVENT:
            enable_interrupt(UART_1_INTERRUPT);
            uart_enable_interrupt(COM1, R_INTERRUPT);
            break;
        default:
            log("Enabling Invalid Interrupt\n");
            break;
    }

    return 0;
}

void initialize_events() {
    initialize_interrupts();

    int i;
    for (i = 0; i < NUM_EVENTS; ++i) {
        waiters[i] = 0;
        circular_queue_initialize(&queues[i]);
    }
}
