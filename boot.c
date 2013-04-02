#include "boot.h"


/*
  This code loads the OS Startup code from the disk to proper location in memory.
*/

void loadStartupCode()
{
  mode = KERNEL_MODE;
  emptyPage(OS_STARTUP_CODE_PAGENO);
  readFromDisk(OS_STARTUP_CODE_PAGENO, BOOT_BLOCK);
  storeInteger(reg[IP_REG], OS_STARTUP_CODE_PAGENO * PAGE_SIZE);
}


/*
  This function initialises all the registers to zero
*/
void initializeRegs() {
	int i;
	for(i=0; i<NUM_REGS; i++)
		storeInteger(reg[i],0);
}
