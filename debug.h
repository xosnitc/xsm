#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "memory_constants.h"
#include "interrupt.h"

#define DISABLE 0
#define ENABLE 1


int db_mode = DISABLE;
int step_flag = DISABLE;
/*
  This function invokes a command line interface for debugging.
*/
void debug_interface();	

/*
 This function processes each command the user enters
*/
int runCommand(char command[]);

/*
 This fuction prints all the registers
 */
void printRegisters();

#endif
