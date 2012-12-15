/*
  Contains basic declarations for register, pages,etc.
*/



#ifndef DATA_H
#define DATA_H


#define KERNEL_MODE	0
#define USER_MODE	1

#define R0		0
#define R1		1
#define R2		2
#define R3		3
#define R4		4
#define R5		5
#define R6		6
#define R7		7

#define BP_REG		8
#define SP_REG		9
#define IP_REG		10
#define PID_REG		11



#define S0		12
#define S1		13
#define S2		14
#define S3		15
#define S4		16
#define S5		17
#define S6		18
#define S7		19

#define T0		20
#define T1		21
#define T2		22
#define T3		23


#define NO_USER_REG	8
#define NO_SPECIAL_REG  4
#define NO_SYS_REG	8
#define NO_TEMP_REG  	4




#define NUM_REGS	(NO_USER_REG + NO_SPECIAL_REG + NO_SYS_REG + NO_TEMP_REG)
#define WORD_SIZE	16
#define PAGE_SIZE	256
#define NUM_PAGES	64
#define SIZE_OF_MEM	(PAGE_SIZE * NUM_PAGES)         //note Entire memory
#define PCB_SIZE	16


#define CODE_PAGE	0
#define NUM_CODE_PAGES	2
#define DATA_PAGE	2
#define STACK_PAGE	3
#define STACK_START_ADDRESS (STACK_PAGE * PAGE_SIZE)
#define NUM_PAGES_PERPROCESS	4

struct {
	char data[WORD_SIZE];
	int flag;
}yylval;

char reg[NUM_REGS][WORD_SIZE];
char instruction[WORD_SIZE];
int mode;

typedef struct {
	char word[PAGE_SIZE][WORD_SIZE];
}PAGE;				//WORD refers to one single page( like a page datatype. Note: If possible change WORD to something else)

PAGE page[NUM_PAGES];	//Whole memory

struct address {
	int page_no;
	int word_no;
};

#endif
