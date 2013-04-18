#ifndef UTILITY_H
#define UTILITY_H
#include "data.h"
#include "memory_constants.h"
#include "interrupt.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Macro to flush stdin
#define FLUSH_STDIN(x) {if(x[strlen(x)-1]!='\n'){do fgets(Junk,16,stdin);while(Junk[strlen(Junk)-1]!='\n');}else x[strlen(x)-1]='\0';}
char Junk[16]; // buffer for discarding excessive user input, 
               // used by "FLUSH_STDIN" macro

/*
 * Gets the instruction pointed by IP, to the argument
 * Return 0 on success
 * Returns -1 on error after setting IP to exception handler
 */
int getInstruction(char *); 

void emptyPage(int page_no);
struct address translate (int virtual_addr);
int getInteger(char* str );
void storeInteger(char *str, int num);
int getType(char* str);
void exception(char str[50], int ex_status, int fault_pageno);

#endif
