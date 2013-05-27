#ifndef TIME_H_
#define TIME_H_

struct Time {
    unsigned int seconds;
    unsigned int useconds;
};

struct Timer {
    struct Time startTime;
};

typedef struct Time Time;
typedef struct Timer Timer;

void initialize_time();
void time_update();
void get_time (Time *time);
// Assumes a > b
Time time_difference(Time a, Time b);
Time timer_elapsed (Timer *a);
void timer_reset (Timer *a);

#endif
