#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xosconstants.h"
#include "data.h"
#include "utility.h"

#define DISABLE 0
#define ENABLE 1

#define COMMAND_LENGTH 50


int db_mode; 		// flag to check whether machine is run in debug mode or not
int step_flag;		// flag to check whether debugging in single step mode

char command[COMMAND_LENGTH],prev_command[COMMAND_LENGTH]; //buffer to store command and previous command.

int watch_flag;		// flag to check whether debugging in watch mode
char watch_value[WORD_SIZE];	// previous value of watched location
struct address watch_addr; // address of watched location

/*
 * This function initializes all debug flags and buffers
 */
void initialize_debug();

/*
  This function invokes a command line interface for debugging.
*/
void debug_interface();	

/*
 * function processes each command the user enters
 * returns 1 if step or continue
 * returns 0 on success
 * returns -1 on error
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
 
/* 
 * This function translates an address without
 * invoking execution on errors.
 * returns page_no and word_no as -1 on failure
 */ 
struct address translate_debug (int virtual_addr);
 
/* 
 * This function prints the memory location
 */
void printLocation(struct address);

/*
 * This function check whether the value of watch location has changed
 * If changed returns ENABLE after printing old and new value of location
 * If unchanged returns DISABLE
 */
int checkWatch();
 
#endif
