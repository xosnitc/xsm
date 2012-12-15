#ifndef BOOT_H
#define BOOT_H
#include "memory_constants.h"
#include "disk.h"
#include "data.h"
/*
  This code loads the OS Startup code from the disk to proper location in memory.
*/
void loadStartupCode();

/*
  This function initialises all the registers to zero
*/
void initializeRegs();
#endif