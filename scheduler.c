#include "scheduler.h"
#include <stdio.h>


/*
  This function runs the scheduler which is the INT 0 code.
*/
void runInt0Code(){
	struct address translated_addr;
	if(getInteger(reg[SP_REG])  + 1 < STACK_START_ADDRESS){
		exception("Stack underflow\n");
		exit(0);
		}
	if(getInteger(reg[SP_REG])  + 1 >= STACK_START_ADDRESS + PAGE_SIZE){
			exception("Stack overflow\n");
			exit(0);
		}	
	storeInteger(reg[SP_REG], getInteger(reg[SP_REG])+1);
	translated_addr = translate(getInteger(reg[SP_REG]));
	storeInteger(page[translated_addr.page_no].word[translated_addr.word_no], getInteger(reg[IP_REG]));
// 	printf("Pushing %d into %d\n",getInteger(reg[IP_REG]),getInteger(reg[SP_REG]));
	mode = KERNEL_MODE;
	storeInteger(reg[IP_REG], (INT0 + INT_START_PAGE) * PAGE_SIZE);
}