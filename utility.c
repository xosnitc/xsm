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
		page_entry = getInteger(reg[PTBR_REG]);
		resultant_addr.page_no = getInteger(page[page_entry / PAGE_SIZE].word[ (page_entry % PAGE_SIZE) + (virtual_addr / PAGE_SIZE) ]);
		resultant_addr.word_no = virtual_addr % PAGE_SIZE;
// 		printf("pg %d - wd %d \n", resultant_addr.page_no, resultant_addr.word_no); note: debugging
		return resultant_addr;
	}
	else {
		struct address resultant_addr;
		resultant_addr.page_no = virtual_addr / PAGE_SIZE;
		resultant_addr.word_no = virtual_addr % PAGE_SIZE;
// 		printf("pg %d - wd %d \n", resultant_addr.page_no, resultant_addr.word_no); note:debugging
		return resultant_addr;
	}
}

int getInteger(char* str ) {
	//return *((int*)str);
	return atoi(str);
}

void storeInteger(char *str, int num) {
	/*char *c = (char*)&num;
	str[0] = *c;
	str[1] = *(c + 1);
	str[2] = *(c + 2);
	str[3] = *(c + 3);*/
	sprintf(str,"%d",num);
}

int isInteger(char* str)
{
	int i=0;
	if(str[i] == '+' || str[i] == '-')
		i++;
	for(;str[i]!='\0';i++)
		if(!(str[i]>='0' && str[i]<='9'))
			return 0;
	return 1;
}

void printRegisters() {
	int i=0;
	for(i=0;i<NUM_REGS;i++) {
		switch(i) {
			case BP_REG: 
				printf("BP: %d\n",getInteger(reg[BP_REG]));
				break;
			case SP_REG: 
				printf("SP: %d\n",getInteger(reg[SP_REG]));
				break;
			case IP_REG: 
				printf("IP: %d\n",getInteger(reg[IP_REG]));
				break;
			case PTBR_REG: 
				printf("PTBR: %d\n",getInteger(reg[PTBR_REG]));
				break;
			case PTLR_REG: 
				printf("PTLR: %d\n",getInteger(reg[PTLR_REG]));
				break;
			case EFR_REG: 
				printf("EFR: %d\n",getInteger(reg[EFR_REG]));
				break;		
			default: 
				if(i<T0)
					printf("R%d: %d\n",i,getInteger(reg[i]));
				else
					printf("T%d: %d\n",i-T0,getInteger(reg[i]));
				break;
		}
	}
}

void exception(char str[50], int ex_status, int fault_pageno) {
	if(getInteger(reg[IP_REG])<0 || getInteger(reg[IP_REG]) > SIZE_OF_MEM)
	      printf("<ERROR:%s\nTried to access IP:%d\n", str, getInteger(reg[IP_REG]));
	else
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
