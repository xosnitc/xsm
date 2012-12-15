#ifndef UTILITY_H
#define UTILITY_H
#include "data.h"
#include "memory_constants.h"
#include "interrupt.h"
#include <string.h>
#include <stdio.h>
#include<stdlib.h>
void emptyPage(int page_no);
struct address translate (int virtual_addr);
int getInteger(char* str );
void storeInteger(char *str, int num);
void printRegisters();
void exception(char str[50]);

#endif