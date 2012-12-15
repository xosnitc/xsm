#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "disk.h"
#include "data.h"
#include "interrupt.h"
#include "utility.h"
#include <stdlib.h>


/*
  This function runs the scheduler which is the INT 0 code.
*/
void runInt0Code();

#endif