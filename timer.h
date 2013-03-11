#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>

#define DEFAULT_TIMESLICE 10

int TIMESLICE = DEFAULT_TIMESLICE;
int time_counter;

int is_time_zero() {
	return (time_counter == 0);
}

void tick() {
	time_counter--;
}

void reset_timer() {
	time_counter = TIMESLICE;
}

#endif
