#ifndef _TRAIN_WIDGET_H_
#define _TRAIN_WIDGET_H_

struct TrainLocation;

int train_display_init();
int train_display_update(int index, struct TrainLocation *train);

#endif
