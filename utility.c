#include "utility.h"

/*
 * Gets the instruction pointed by IP, to the argument
 * Return 0 on success
 * Returns -1 on error after setting IP to exception handler
 */
int getInstruction(char *instruction)
{
	struct address translatedAddr;
	int len;
	bzero(instruction,WORD_SIZE * WORDS_PERINSTR);
	if(getType(reg[IP_REG]) == TYPE_STR)
	{	
		exception("Illegal IP value. Not an address", EX_ILLMEM, 0);
		return -1;
	}
	if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
	{	
		exception("Illegal PTLR value", EX_ILLMEM, 0);
		return -1;
	}
	if(getInteger(reg[IP_REG])<0 || getInteger(reg[IP_REG]) + 1 >=SIZE_OF_MEM){			//checks if address is outside limits
		exception("IP Register value out of bounds", EX_ILLMEM, 0);
		return -1;
	}
	if(mode == USER_MODE){						//checks if address is outside limits if mode is user mode
		if(getInteger(reg[IP_REG]) < 0 || getInteger(reg[IP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) {
			printf("%d", getInteger(reg[IP_REG]));
			exception("Illegal IP access1", EX_ILLOPERAND, 0);
			return -1;
		}
	}
	translatedAddr = translate(getInteger(reg[IP_REG]));
	if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
		return -1;
	strcpy(instruction,page[translatedAddr.page_no].word[translatedAddr.word_no]);
	translatedAddr = translate(getInteger(reg[IP_REG])+1);
	if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
		return -1;
	len = strlen(instruction);
	instruction[len]=' ';
	instruction[len+1]='\0';
	strcat(instruction,page[translatedAddr.page_no].word[translatedAddr.word_no]);
	return 0;
}

void emptyPage(int page_no) {
	int i;
	for(i = 0 ; i < PAGE_SIZE ; i++)
	{
		strcpy(page[page_no].word[i],"") ;
	}
}

struct address translate (int virtual_addr) {
	if(mode == USER_MODE)
	{
		struct address resultant_addr;
		int page_entry;
		resultant_addr.page_no = -1;
		resultant_addr.word_no = -1;
		if(getType(reg[PTBR_REG]) == TYPE_STR)
		{	
			exception("Illegal Register value", EX_ILLMEM, 0);
			return resultant_addr;
		}
		page_entry = getInteger(reg[PTBR_REG]) + (virtual_addr / PAGE_SIZE) * 2;
		if(page[(page_entry+1) / PAGE_SIZE].word[(page_entry+1) % PAGE_SIZE][1] == VALID )
		{ 
			resultant_addr.page_no = getInteger(page[page_entry / PAGE_SIZE].word[page_entry % PAGE_SIZE] );
			resultant_addr.word_no = virtual_addr % PAGE_SIZE;
			page[(page_entry+1) / PAGE_SIZE].word[(page_entry+1) % PAGE_SIZE][0] = REFERENCED;
		}
		else
			exception("Page Fault", EX_PAGEFAULT, virtual_addr / PAGE_SIZE);
		return resultant_addr;
	}
	else
	{
		struct address resultant_addr;
		resultant_addr.page_no = virtual_addr / PAGE_SIZE;
		resultant_addr.word_no = virtual_addr % PAGE_SIZE;
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

void exception(char str[50], int ex_status, int fault_pageno) {
	if(mode == KERNEL_MODE)
	{
		printf("<ERROR:%d:%s> %s\n",getInteger(reg[IP_REG]),instruction, str);
		if(db_mode == ENABLE)
			debug_interface();
		exit(0);
	}
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
