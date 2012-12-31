#include "scheduler.h"
#include <stdio.h>

/*
  This function runs the scheduler which is the INT 0 code.
*/
void runInt0Code()
{
	struct address translated_addr;
	if(getType(reg[SP_REG]) == TYPE_STR || getType(reg[PTLR_REG]) == TYPE_STR)
	{
		exception("Illegal Register value", EX_ILLMEM, 0);
		return;
	}
	if(getInteger(reg[SP_REG])  + 1 < 0)
	{
		exception("Stack underflow\n", EX_ILLMEM, 0);
		return;
	}
	if(getInteger(reg[SP_REG])  + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)
	{
		exception("Stack overflow\n", EX_ILLMEM, 0);
		return;
	}	
	translated_addr = translate(getInteger(reg[SP_REG])+1);
	if(translated_addr.page_no == -1 && translated_addr.word_no == -1)
		return;
	storeInteger(reg[SP_REG], getInteger(reg[SP_REG])+1);
	storeInteger(page[translated_addr.page_no].word[translated_addr.word_no], getInteger(reg[IP_REG]));
// 	printf("Pushing %d into %d\n",getInteger(reg[IP_REG]),getInteger(reg[SP_REG]));
	mode = KERNEL_MODE;
	storeInteger(reg[IP_REG], INT0  * PAGE_SIZE);
}
