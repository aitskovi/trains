#include <fine_timer.h>
#include <ts7200.h>

static volatile unsigned int *timer_low, *timer_enable;

unsigned int get_fine_time () {
    return *timer_low;
}

void initialize_fine_timer () {
    timer_low = (unsigned int *) (TIMER4_LOW);
    timer_enable = (unsigned int *) (TIMER4_ENABLE);

    *timer_enable = 0;
    *timer_enable = 1 << 8;
}

void fine_timer_reset (FineTimer *timer) {
    timer->start_time =  get_fine_time();
}

unsigned int fine_timer_elapsed (FineTimer *timer) {
    unsigned int now = get_fine_time();
    return now - timer->start_time;
}

unsigned int fine_time_to_usec(unsigned int fine_time) {
    return (unsigned int) (1000000.0f * fine_time /  FINE_TIMER_FREQUENCY);
}
