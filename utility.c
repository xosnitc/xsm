#include "utility.h"

void emptyPage(int page_no) {
	int i;
	for(i = 0 ; i < PAGE_SIZE ; i++)
	{
		strcpy(page[page_no].word[i],"") ;
	}
}

struct address translate (int virtual_addr) {
	if(mode == USER_MODE) {
		struct address resultant_addr;
		int page_entry;
		if(getType(reg[PTBR_REG]) == TYPE_STR)
		{	
		    exception("Illegal Register value", EX_ILLMEM, 0);
			return;
		}
		page_entry = getInteger(reg[PTBR_REG]) + (virtual_addr / PAGE_SIZE) * 2;
		if(page[(page_entry+1) / PAGE_SIZE].word[(page_entry+1) % PAGE_SIZE][1] == VALID )
		{ 
			resultant_addr.page_no = getInteger(page[page_entry / PAGE_SIZE].word[page_entry % PAGE_SIZE] );
			resultant_addr.word_no = virtual_addr % PAGE_SIZE;
		}
		else
		{
			resultant_addr.page_no = -1;
			resultant_addr.word_no = -1;
			exception("Page Fault", EX_PAGEFAULT, virtual_addr / PAGE_SIZE);			
		}
// 		printf("pg %d - wd %d \n", resultant_addr.page_no, resultant_addr.word_no); note: debugging
		return resultant_addr;
	}
	else
	{
		struct address resultant_addr;
		resultant_addr.page_no = virtual_addr / PAGE_SIZE;
		resultant_addr.word_no = virtual_addr % PAGE_SIZE;
// 		printf("pg %d - wd %d \n", resultant_addr.page_no, resultant_addr.word_no); note:debugging
		return resultant_addr;
	}
}

int getInteger(char* str ) {
	return atoi(str);
}

void storeInteger(char *str, int num) {
	sprintf(str,"%d",num);
}

int getType(char* str)
{
	int i=0;
	if(str[i] == '+' || str[i] == '-')
		i++;
	for(;str[i]!='\0';i++)
		if(!(str[i]>='0' && str[i]<='9'))
			return TYPE_STR;
	return TYPE_INT;
}

void printRegisters() {
	int i=0;
	for(i=0;i<NUM_REGS;i++) {
		switch(i) {
			case BP_REG: 
				printf("BP: %s\n",reg[BP_REG]);
				break;
			case SP_REG: 
				printf("SP: %s\n",reg[SP_REG]);
				break;
			case IP_REG: 
				printf("IP: %s\n",reg[IP_REG]);
				break;
			case PTBR_REG: 
				printf("PTBR: %s\n",reg[PTBR_REG]);
				break;
			case PTLR_REG: 
				printf("PTLR: %s\n",reg[PTLR_REG]);
				break;
			case EFR_REG: 
				printf("EFR: %s\n",reg[EFR_REG]);
				break;		
			default: 
				if(i<T0)
					printf("R%d: %s\n",i,reg[i]);
				else
					printf("T%d: %s\n",i-T0,reg[i]);
				break;
		}
	}
}

void exception(char str[50], int ex_status, int fault_pageno) {
	if(ex_status != EX_PAGEFAULT)
		printf("<ERROR:%d:%s> %s\n",getInteger(reg[IP_REG]),instruction, str);
	if(mode == KERNEL_MODE)
		exit(0);
	else
	{
		int ex_flag;
		ex_flag = getInteger(reg[IP_REG])*1000;
		ex_flag += fault_pageno*10;
		ex_flag += ex_status;
		mode = KERNEL_MODE;
		storeInteger(reg[EFR_REG], ex_flag);
		storeInteger(reg[IP_REG], EXCEP_HANDLER * PAGE_SIZE);
	}
}
