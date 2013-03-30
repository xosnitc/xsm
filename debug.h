#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xosconstants.h"

#define DISABLE 0
#define ENABLE 1


int db_mode; 		// flag to check whether machine is run in debug mode or not
int step_flag;		// flag to check whether debugging in single step mode

/*
  This function invokes a command line interface for debugging.
*/
void debug_interface();	

/*
 * function processes each command the user enters
 */
int runCommand(char command[]);

/*
 * Function to get register number from argument
 */
int getRegArg(char *);

/*
 * This fuction prints all the registers from arg1 to arg2
 */
void printRegisters(int, int);

/*
 * This fuction prints the memory page passed as argument.
 */
void printMemory(int);

/*
 * This fuction prints the PCB of process with given process ID.
 */
void printPCB(int);

/*
 * This fuction prints the page table of process with given process ID.
 */
void printPageTable(int);

/* 
 * This function prints the system wide open file table
 */
 void printFileTable();
 
 /* 
 * This function prints the memory free list
 */
 void printMemFreeList();

/* 
 * This function prints the disk free list
 */
 void printDiskFreeList();
 
 /* 
 * This function prints the File Allocation table
 */
 void printFAT();

#endif
