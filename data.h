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

#define R8		8
#define R9		9
#define R10		10
#define R11		11
#define R12		12
#define R13		13
#define R14		14
#define R15		15

#define T0		16
#define T1		17
#define T2		18
#define T3		19

#define BP_REG		20
#define IP_REG		21
#define SP_REG		22
#define PTBR_REG	23
#define PTLR_REG	24
#define EFR_REG		25

#define NO_USER_REG		8
#define NO_SYS_REG		8
#define NO_TEMP_REG  	4
#define NO_SPECIAL_REG  6

#define NUM_REGS		(NO_USER_REG + NO_SPECIAL_REG + NO_SYS_REG + NO_TEMP_REG)
#define WORD_SIZE		16
#define PAGE_SIZE		512
#define NUM_PAGES		64
#define SIZE_OF_MEM		(PAGE_SIZE * NUM_PAGES)         //note Entire memory
#define WORDS_PERINSTR	2

#define VALID			'1'
#define INVALID			'0'
#define REFERENCED		'1'
#define UNREFERENCED		'0'

#define TYPE_INT		1
#define TYPE_STR		0

#define EX_PAGEFAULT	0
#define EX_ILLINSTR		1
#define EX_ILLMEM		2
#define EX_ARITH		3
#define EX_ILLOPERAND	4

struct {
	char data[WORD_SIZE];
	int flag;
	int flag2;
}yylval;

char reg[NUM_REGS][WORD_SIZE];
char instruction[WORD_SIZE * WORDS_PERINSTR];
int mode;
int db_mode;

typedef struct {
	char word[PAGE_SIZE][WORD_SIZE];
}PAGE;				//WORD refers to one single page( like a page datatype. Note: If possible change WORD to something else)

PAGE page[NUM_PAGES];	//Whole memory

struct address {
	int page_no;
	int word_no;
};

#endif
