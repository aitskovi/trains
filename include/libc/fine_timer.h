#ifndef TIME_H_
#define TIME_H_

#define FINE_TIMER_FREQUENCY 983000

typedef struct FineTimer {
    unsigned int start_time;
} FineTimer;

void initialize_fine_timer ();
unsigned int get_fine_time ();
unsigned int fine_timer_elapsed (FineTimer *a);
void fine_timer_reset (FineTimer *a);
unsigned int fine_time_to_usec(unsigned int fine_time);

#endif
