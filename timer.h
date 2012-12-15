#ifndef TIMER_H
#define TIMER_H

#include <stdio.h>

#define TIMESLICE 10

int time_counter = TIMESLICE;

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