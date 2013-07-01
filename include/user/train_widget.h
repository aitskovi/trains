#ifndef _TRAIN_WIDGET_H_
#define _TRAIN_WIDGET_H_

struct track_node;

int train_display_init();
int train_display_update(int index, int number, struct track_node *landmark, int distance);
void train_widget();

#endif
