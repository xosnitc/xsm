#ifndef SIMULATOR_H
#define SIMULATOR_H
#include "lex.yy.c"
#include "timer.h"
#include "boot.h"
#include "scheduler.h"
#include "utility.h"
#include "interrupt.h"
#include "debug.h"

void Executeoneinstr(int);
void Operanderror(char*,int);
void run(int db_mode, int intDisable);
#endif
