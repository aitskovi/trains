#include <time.h>
#include <ts7200.h>

#define TICKS_PER_USECOND

static unsigned int lastTick, residualTicks;
static volatile unsigned int *timer_low, *timer_enable;

static Time currentTime;

void time_update () {
    unsigned int elapsed;
    unsigned int currentTick;

    currentTick  = *timer_low;
    
    residualTicks += currentTick - lastTick;

    elapsed = (residualTicks * 1000) / 983;
    residualTicks -= elapsed * 983 / 1000;

    currentTime.useconds += elapsed;
    currentTime.seconds += currentTime.useconds / 1000000;
    currentTime.seconds %= 1000000;

    lastTick = currentTick;
}


Time time_difference (Time a, Time b) {
    Time result;

    if (a.useconds < b.useconds) {
        a.useconds += 1000000;
        a.seconds -= 1;
    }

    result.seconds = a.seconds - b.seconds;
    result.useconds = a.useconds - b.useconds;

    return result;
}

void get_time (Time *time) {
    time_update();
    *time = currentTime;
}

void initialize_time () {
    residualTicks = 0;
    
    currentTime.seconds = 0;
    currentTime.useconds  = 0;

    timer_low = (unsigned int *) (TIMER4_LOW);
    timer_enable = (unsigned int *) (TIMER4_ENABLE);

    lastTick = 0;
    *timer_enable = 0;
    *timer_enable = 1 << 8;
}

void timer_reset (Timer *timer) {
    get_time (&timer->startTime);
}

Time timer_elapsed (Timer *timer) {
    Time now;
    get_time(&now);
    return time_difference(now, timer->startTime);
}
