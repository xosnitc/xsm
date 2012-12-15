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
		int pid, page_entry;
		pid = getInteger(reg[PID_REG]);
		page_entry = pid * NUM_PAGES_PERPROCESS;
		resultant_addr.page_no = getInteger(page[PAGETBL_PAGENO].word[PAGETBL_START_WORD + page_entry + virtual_addr / PAGE_SIZE]);
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
	return *((int*)str);
}

void storeInteger(char *str, int num) {
	char *c = (char*)&num;
	str[0] = *c;
	str[1] = *(c + 1);
	str[2] = *(c + 2);
	str[3] = *(c + 3);
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
			case PID_REG: 
				printf("PID: %d\n",getInteger(reg[PID_REG]));
				break;	
			default: 
				if(i<=7)
					printf("R%d: %d\n",i,getInteger(reg[i]));
				else if(i<20)
					printf("S%d: %d\n",i-12,getInteger(reg[i]));
				else
					printf("T%d: %d\n",i-20,getInteger(reg[i]));
				break;
		}
	}
}

void exception(char str[50]) {
	if(getInteger(reg[IP_REG])<0 || getInteger(reg[IP_REG]) > SIZE_OF_MEM)
	      printf("<ERROR:%s\nTried to access IP:%d\n", str, getInteger(reg[IP_REG]));
	else
	      printf("<ERROR:%d:%s> %s\n",getInteger(reg[IP_REG]),instruction, str);
	exit(0); 
	//storeInteger(reg[IP_REG], EXCEP_HANDLER * PAGE_SIZE);
}
