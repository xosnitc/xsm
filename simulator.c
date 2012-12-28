#include "simulator.h"

//unsigned long long int tempCount1=0;
/*
   note : -db for debug mode -id for disabling timer interrupt
*/
main(int argc,char **argv){
	int db_mode=0, intDisable=0;
	if(argc >= 2)
	{
	  if(strcmp(argv[1],"-db") == 0)
	    db_mode = 1;
	  else if(strcmp(argv[1], "-id") == 0)
	    intDisable = 1;
	  else
	  {
	    printf("Invalid arguement %s", argv[1]);
	    exit(0);
	  }
	}
	if(argc == 3)
	{
	  if(strcmp(argv[2],"-db") == 0)
	    db_mode = 1;
	  else if(strcmp(argv[2], "-id") == 0)
	    intDisable = 1;
	  else
	  {
	    printf("Invalid arguement %s", argv[2]);
	    exit(0);
	  }
	}
	initializeRegs();  
	run(db_mode, intDisable);	
}



/*
  This function does the following:
    1. Loads OS Startup Code.
    2. Copies the instruction to be parsed as per the address specified by the IP register.
    3. Checks whether interrupt is disabled. If not th clock ticks.
    4. Begins the lexical analysis by getting the first token and passing it as arguement to Executeoneinstr.
    5. Finally checks if time slice allocated is over or not. If yes and mode is user mode ,and if interrupt is enabled then
      INT 0 code is run.
*/
void run(int db_mode, int intDisable) {
	loadStartupCode();
	int instr;
	unsigned long long int tempCount=0;
	while(1) {
		struct address translatedAddr;
		if(getInteger(reg[IP_REG])<0 || getInteger(reg[IP_REG]) + 1 >SIZE_OF_MEM){			//checks if address is outside limits
		    exception("IP Register value out of bounds", EX_ILLMEM, 0);
		    continue;
		}
		if(mode == USER_MODE){						//checks if address is outside limits if mode is user mode
			if(getInteger(reg[IP_REG]) < 0 || getInteger(reg[IP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) {
				printf("%d", getInteger(reg[IP_REG]));
				exception("Illegal IP access1", EX_ILLOPERAND, 0);
				continue;
			}
		}
		translatedAddr = translate(getInteger(reg[IP_REG]));
		strcpy(instruction,page[translatedAddr.page_no].word[translatedAddr.word_no]);
		translatedAddr = translate(getInteger(reg[IP_REG])+1);
		strcat(instruction,page[translatedAddr.page_no].word[translatedAddr.word_no]);
//  		printf("%s\n", instruction); // note:debugging
		translatedAddr.word_no = -1;
		translatedAddr.page_no = -1;
		instr = yylex();
		if(mode == USER_MODE && !intDisable) 
			tick();
		//tempCount1++;
		Executeoneinstr(instr);
		if(db_mode) {
			printf("Values in registers after executing instruction :%s\n", instruction);
			printRegisters();
			printf("Press X to exit or any other key to continue.....\n");
			char ch;
			scanf("%c",&ch);
			if(ch=='X' || ch=='x')
				exit(0);
		}
		if(is_time_zero() && !intDisable && mode==USER_MODE) {
			reset_timer();
			runInt0Code();
		}
	}
}



/*
  This code is used to execute each ininstruction.
*/
void Executeoneinstr(int instr)
{
	int opnd1,opnd2,flag1,flag12,flag2,flag22,oper,result, result2;
	int opnd1Value;
	int opnd2Value;
	int i;
	char charRead;
	struct address translatedAddr;
	struct address translatedAddr1, translatedAddr2;;
	switch(instr)
	{
		case START: storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);	//increment IP
		break;
		case MOV:						//1st phase:get the value		2nd phase:store the value
		{
			opnd1 = yylex();
			flag1 = yylval.flag;
			flag12 = yylval.flag2;
			opnd2 = yylex();
			flag2 = yylval.flag;
			flag22 = yylval.flag2;
			switch(flag2)
			{
				case REG:
					if(mode == USER_MODE && opnd2 > R7)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd2 > T3)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[opnd2]);
					break;
				case SP: 
					result = getInteger(reg[SP_REG]);
					break;
				case BP: 
					result = getInteger(reg[BP_REG]);
					break;
				case IP:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[IP_REG]);
					break;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[PTBR_REG]);
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[PTLR_REG]);
					break;
				case EFR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[EFR_REG]);
					break;
				case NUM:
					result = opnd2;
					break;
				case MEM_REG:
				{
					if(mode == USER_MODE)
					{
						if(opnd2 > R7)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(getInteger(reg[opnd2]) < 0 || getInteger(reg[opnd2]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)
						{
							exception("Illegal address access1", EX_ILLMEM, 0);
							return;
						}							
					}
					else
					{
						if(opnd2 > T3)
						{
							exception("Illegal register access", EX_ILLOPERAND, 0);
							return;
						}
						else if(getInteger(reg[opnd2]) < 0 || getInteger(reg[opnd2]) >= SIZE_OF_MEM)
						{
							exception("Illegal address access1", EX_ILLMEM, 0);
							return;
						}
					}
					translatedAddr = translate(getInteger(reg[opnd2]));
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				}
				case MEM_SP:			
					if(getInteger(reg[SP_REG]) < 0 || (mode == USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && getInteger(reg[SP_REG]) >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nSP value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[SP_REG]));
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_BP:
					if(getInteger(reg[BP_REG]) < 0 || (mode == USER_MODE && getInteger(reg[BP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && getInteger(reg[BP_REG]) >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nBP value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[BP_REG]));
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_IP:							
					exception("Cannot use memory reference with IP in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getInteger(reg[PTBR_REG]) < 0 || getInteger(reg[PTBR_REG]) >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nRegister value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[PTBR_REG]));
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getInteger(reg[PTLR_REG]) < 0 || getInteger(reg[PTLR_REG]) >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nRegister value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[PTLR_REG]));
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_EFR:							
					exception("Cannot use memory reference with EFR in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR:
					if(opnd2 < 0 || (mode == USER_MODE && opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd2 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd2);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_REG:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && flag22 > R7)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;					
					}
					else if(mode == KERNEL_MODE && flag22 > T3)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}					 
					opnd2 += getInteger(reg[flag22]);
					if(opnd2 < 0 || (mode == USER_MODE && opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd2 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd2);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_SP:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}					 
					opnd2 += getInteger(reg[SP_REG]);
					if(opnd2 < 0 || (mode == USER_MODE && opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd2 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd2);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_BP:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}					 
					opnd2 += getInteger(reg[BP_REG]);
					if(opnd2 < 0 || (mode == USER_MODE && opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd2 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd2);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_IP:							
					exception("Cannot use memory reference with IP in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR_PTBR:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}					 
					opnd2 += getInteger(reg[PTBR_REG]);
					if(opnd2 < 0 || opnd2 >= SIZE_OF_MEM) 
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd2);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_PTLR:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}					 
					opnd2 += getInteger(reg[PTLR_REG]);
					if(opnd2 < 0 || opnd2 >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd2);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_EFR:							
					exception("Cannot use memory reference with EFR in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR_IN:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}
					opnd2 += flag22;
					if(opnd2 < 0 || (mode == USER_MODE && opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd2 >= SIZE_OF_MEM) )
					{
						 exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						 return;
					}
					else
						translatedAddr = translate(opnd2);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				default:
					exception("Illegal Operand", EX_ILLOPERAND, 0);
					return;
				break;
			}
			switch(flag1)
			{
				case REG:
					if(mode == USER_MODE && opnd1 > R7)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 > T3)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else 
						storeInteger(reg[opnd1], result);
					break;
				case SP:
					storeInteger(reg[SP_REG], result);
					break;
				case BP: 
					storeInteger(reg[BP_REG], result);
					break;
				case IP:
					exception("Illegal operand IP. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;					
					break;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						storeInteger(reg[PTBR_REG], result);
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						storeInteger(reg[PTLR_REG], result);
					break;
				case EFR:
					exception("Illegal operand EFR. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;					
					break;
				case MEM_REG:
					if(mode == USER_MODE)
					{
						if(opnd1 > R7)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(getInteger(reg[opnd1]) < 0 || getInteger(reg[opnd1]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)
						{
							exception("Illegal address access1", EX_ILLMEM, 0);
							return;
						}							
					}
					else
					{
						if(opnd1 > T3)
						{
							exception("Illegal register access", EX_ILLOPERAND, 0);
							return;
						}
						else if(getInteger(reg[opnd1]) < 0 || getInteger(reg[opnd1]) >= SIZE_OF_MEM)
						{
							exception("Illegal address access1", EX_ILLMEM, 0);
							return;
						}
					}
					translatedAddr = translate(getInteger(reg[opnd1]));
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_SP:
					if(getInteger(reg[SP_REG]) < 0 || (mode == USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && getInteger(reg[SP_REG]) >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nSP value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else					
						translatedAddr = translate(getInteger(reg[SP_REG]));
				    storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_BP:
				    if(getInteger(reg[BP_REG]) < 0 || (mode == USER_MODE && getInteger(reg[BP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && getInteger(reg[BP_REG]) >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nBP value is out of bounds.", EX_ILLMEM, 0);
						return;
					} 
				    else
				    	translatedAddr = translate(getInteger(reg[BP_REG]));
				    storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_IP:
				    exception("Cannot use memory reference with IP in any mode.", EX_ILLOPERAND, 0);
				    return;
					break;
				case MEM_PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getInteger(reg[PTBR_REG]) < 0 || getInteger(reg[PTBR_REG]) >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nRegister value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[PTBR_REG]));
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getInteger(reg[PTLR_REG]) < 0 || getInteger(reg[PTLR_REG]) >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nRegister value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[PTLR_REG]));
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_EFR:							
					exception("Cannot use memory reference with EFR in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR:
					if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access4", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_DIR_REG:
					if(mode == USER_MODE && flag12 > R7)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;					
					}
					else if(mode == KERNEL_MODE && flag12 > T3)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					opnd1 += getInteger(reg[flag12]);
					if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_DIR_SP:
					opnd1 += getInteger(reg[SP_REG]);
					if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_DIR_BP:
					opnd1 += getInteger(reg[BP_REG]);
					if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_DIR_IP:							
					exception("Cannot use memory reference with IP in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR_PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					opnd1 += getInteger(reg[PTBR_REG]);
					if(opnd1 < 0 || opnd1 >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_DIR_PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					opnd1 += getInteger(reg[PTLR_REG]);
					if(opnd1 < 0 || opnd1 >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				case MEM_DIR_EFR:							
					exception("Cannot use memory reference with EFR in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR_IN:
					opnd1 += flag12;
					if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						 exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						 return;
					}
					else
						translatedAddr = translate(opnd1);
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;
				break;
			}
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
		}
		break;
		case ARITH:
		{
			oper = yylval.flag;
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand1", EX_ILLOPERAND, 0);
				return;
			}
			else
			{
				if(oper!= INR && oper!=DCR)
				{
					opnd2 = yylex();
					if(yylval.flag != REG) {
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
				}
				switch(oper)
				{
					case ADD:
						storeInteger(reg[opnd1],getInteger(reg[opnd1]) + getInteger(reg[opnd2]));
						break;			
					case SUB:
						storeInteger(reg[opnd1],getInteger(reg[opnd1]) - getInteger(reg[opnd2]));
						break;
					case MUL:
						storeInteger(reg[opnd1],getInteger(reg[opnd1]) * getInteger(reg[opnd2]));
						break;
					case DIV: 
						if(getInteger(reg[opnd2]) == 0) {
							exception("Divide by ZERO", EX_ILLOPERAND, 0);
							return;
						}
						storeInteger(reg[opnd1],getInteger(reg[opnd1]) / getInteger(reg[opnd2]));
						break;
					case MOD:
						storeInteger(reg[opnd1],getInteger(reg[opnd1]) % getInteger(reg[opnd2]));
						break;
					case INR:
						storeInteger(reg[opnd1],getInteger(reg[opnd1]) + 1);
						break;
					case DCR:
						storeInteger(reg[opnd1],getInteger(reg[opnd1]) - 1);
						break;
					default:
						exception("Illegal Instruction", EX_ILLINSTR, 0);
						return;
						break;
				}
				storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			}
			
		}
		break;
		case LOGIC:
		{
			oper = yylval.flag;
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			opnd2 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			switch(oper)
			{
				case LT:
					storeInteger(reg[opnd1],(getInteger(reg[opnd1]) < getInteger(reg[opnd2])?1:0));
					break;
				case GT:
					storeInteger(reg[opnd1],(getInteger(reg[opnd1]) > getInteger(reg[opnd2])?1:0));
					break;	
				case EQ:
					storeInteger(reg[opnd1],(getInteger(reg[opnd1]) == getInteger(reg[opnd2])?1:0));
					break;
				case NE:
					storeInteger(reg[opnd1],(getInteger(reg[opnd1]) != getInteger(reg[opnd2])?1:0));
					break;	
				case LE:
					storeInteger(reg[opnd1],(getInteger(reg[opnd1]) <= getInteger(reg[opnd2])?1:0));
					break;
				case GE:
					storeInteger(reg[opnd1],(getInteger(reg[opnd1]) >= getInteger(reg[opnd2])?1:0));
					break;	
				default:
					exception("Illegal Instruction", EX_ILLINSTR, 0);
					return;
					break;
			}
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
		}
		break;
		case BRANCH:
		{
			oper = yylval.flag;
			switch(oper)
			{
				case JZ:
					opnd1 = yylex();
					if(yylval.flag != REG) {
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					opnd2 = yylex();
					if(yylval.flag != NUM){
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					if(getInteger(reg[opnd1]) == 0)
					{
						if(mode == USER_MODE) {
							if(opnd2 < 0 || opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) {
								exception("Illegal address access", EX_ILLMEM, 0);
								return;
							}
						}
						storeInteger(reg[IP_REG], opnd2);
						YY_FLUSH_BUFFER;
					}
					else
						storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
					break;
				
				case JNZ:
					opnd1 = yylex();
					if(yylval.flag != REG){
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					opnd2 = yylex();
					if(yylval.flag != NUM) {
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					if( getInteger(reg[opnd1]) != 0)
					{
						if(mode == USER_MODE) {
							if(opnd2 < 0 || opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) {
								exception("Illegal address access", EX_ILLMEM, 0);
								return;
							}
						}
						storeInteger(reg[IP_REG], opnd2);
						YY_FLUSH_BUFFER;
					}
					else
						storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
					break;
				
				case JMP:
					opnd1 = yylex();
					if(yylval.flag != NUM){
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					if(mode == USER_MODE) {
						if(opnd1 < 0 || opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) {
							exception("Illegal address access", EX_ILLMEM, 0);
						return;
						}
					}
					storeInteger(reg[IP_REG], opnd1);
					YY_FLUSH_BUFFER;
					break;
				
				default:
					exception("Illegal Branch Instruction", EX_ILLINSTR, 0);
					return;
					break;
			}
		}
		break;
								
		case PUSH:				//note:Modified here
			/*if(mode == KERNEL_MODE){
			  exception("PUSH command not available in KERNEL mode");
			}*/
			opnd1 = yylex();
			if(getInteger(reg[SP_REG]) + 1 < 0 ){
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}
			if(getInteger(reg[SP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE){
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}
			storeInteger(reg[SP_REG],getInteger(reg[SP_REG])+1);
			translatedAddr = translate(getInteger(reg[SP_REG]));
			switch(yylval.flag){				//error: need to replace KERNEL constants with actual constants in the machine
				case REG:
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], getInteger(reg[opnd1]));
					break;
				case SP:
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], getInteger(reg[SP_REG]));
					break;
				case BP:
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], getInteger(reg[BP_REG]));
					break;
				case IP:
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], getInteger(reg[IP_REG]));
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;
			}
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
		case POP:				//note:Modified here
			/*if(mode == KERNEL_MODE){
				exception("POP command not available in KERNEL mode");
				return;
			}*/
			opnd1 = yylex();
			if(getInteger(reg[SP_REG]) < 0){
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}
			if(getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE){
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}
			translatedAddr = translate(getInteger(reg[SP_REG]));
			switch(yylval.flag){
				case REG:
					storeInteger(reg[opnd1], getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]));
					break;
				case SP:
					storeInteger(reg[SP_REG], getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]));
					break;
				case BP:
					storeInteger(reg[BP_REG], getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]));
					break;
				case IP:
					if(mode == USER_MODE){
					  	exception("Trying to modify IP in USER mode", EX_ILLOPERAND, 0);
						return;
					}
					storeInteger(reg[IP_REG], getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]));
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;
					break;
			}
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG])-1);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
		case CALL:				//note: Modified here.
			/*if(mode == KERNEL_MODE){
			  exception("Cannot call CALL in KERNEL mode");
			}*/
			opnd1 = yylex();
			if(yylval.flag != NUM) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			if(getInteger(reg[SP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE){
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}
			if(getInteger(reg[SP_REG]) + 1 < 0 )
			{
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + 1);
			translatedAddr = translate(getInteger(reg[SP_REG]));
			storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], getInteger(reg[IP_REG]));
			storeInteger(reg[IP_REG], opnd1);
			YY_FLUSH_BUFFER;
			break;
		case RET:		//note: Modified here
			/*if(mode == KERNEL_MODE){
			  exception("Cannot call RET in KERNEL mode");
			}*/
			if(getInteger(reg[SP_REG]) < 0){ 
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}
			if(getInteger(reg[SP_REG])  >= getInteger(reg[PTLR_REG]) * PAGE_SIZE){
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}
			translatedAddr = translate(getInteger(reg[SP_REG]));
			storeInteger(reg[IP_REG], getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]));
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) - 1);
			YY_FLUSH_BUFFER;
			break;
		case INT:				//error: pid is used to return back and not kernel page table
			if(mode == KERNEL_MODE){
			  	exception("Cannot call INT in KERNEL mode", EX_ILLINSTR, 0);
				return;
			}
			opnd1 = yylex();
			if(yylval.flag != NUM) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			if(opnd1 < 1 || opnd1 > 7) {				//error: might need to modify this
				exception("Illegal INT instruction\n", EX_ILLOPERAND, 0);
				return;
			}
			if(getInteger(reg[SP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE){
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}
			if(getInteger(reg[SP_REG]) + 1 < 0 ){
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + 1);
			translatedAddr = translate(getInteger(reg[SP_REG]));
// 			printf("Pushing %d into %d\n",getInteger(reg[IP_REG]),getInteger(reg[SP_REG]));
// 			printf("Calling INT %d\n", opnd1);
// 			char sh;
// 			scanf("%c", &sh);
			storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no],getInteger(reg[IP_REG]));
			storeInteger(reg[IP_REG], (opnd1 + INT_START_PAGE) * PAGE_SIZE);
			mode = KERNEL_MODE;
			break;
		
		case IRET:			//note: Modified here
			if(mode == USER_MODE){
				exception("Illegal Instruction", EX_ILLINSTR, 0);
				return;
			}
			if(getInteger(reg[SP_REG]) < 0) {			//note:for sfety check for overflow
				exception("Stack Underflow\n", EX_ILLMEM, 0);
				return;
			}
			if(getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) {			//note:for sfety check for overflow
				exception("Stack Overflow\n", EX_ILLMEM, 0);
				return;
			}
			mode = USER_MODE;
			translatedAddr = translate(getInteger(reg[SP_REG]));
// 			printf("tempCount1 = %llu\n", tempCount1);
// 			printf("Popping %d from %d\n", getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]), getInteger(reg[SP_REG]));
// 			printf("This is page %d and word %d\n",translatedAddr.page_no, translatedAddr.word_no );
			storeInteger(reg[IP_REG], getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]));
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) - 1);
			break;
		case IN:
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			int input;
			scanf("%d",&input);
			storeInteger(reg[opnd1], input);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
		
		case OUT:		//note: is there any need to print '\n' since string is present
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			printf("%d\n",getInteger(reg[opnd1]));
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
		
/*		case SIN:			//note:modified here
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand");
			}
			char str[WORD_SIZE];
			i=0;
			charRead = getchar();
			while(i<WORD_SIZE-1 && charRead != '\n'){
			    str[i++] = charRead;
			    charRead = getchar();
			}
			str[i++] = charRead;	
			str[WORD_SIZE-1]='\0';
// 			printf("---%s----\n", str);
			opnd1Value = getInteger(reg[opnd1]);
			translatedAddr = translate(opnd1Value);
			strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str);
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG])+WORDS_PERINSTR);
		break;
		case SOUT:				//note: modified and do we need '\n'
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand");
			}
			opnd1Value = getInteger(reg[opnd1]);
			translatedAddr = translate(opnd1Value);
			printf("%s\n",page[translatedAddr.page_no].word[translatedAddr.word_no]);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
		break;*/
		case STRCMP:				//note:modified here
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			opnd2 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			opnd1Value = getInteger(reg[opnd1]);
			opnd2Value = getInteger(reg[opnd2]);
			translatedAddr1 = translate(opnd1Value);
			translatedAddr2 = translate(opnd2Value);
// 			printf("---%s---\n---%s---\n",page[translatedAddr1.page_no].word[translatedAddr1.word_no],page[translatedAddr2.page_no].word[translatedAddr2.word_no]);
			storeInteger(reg[opnd1],strcmp(page[translatedAddr1.page_no].word[translatedAddr1.word_no],page[translatedAddr2.page_no].word[translatedAddr2.word_no]));
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
		break;
		
		case STRCPY:				//note:modified here
			opnd1 = yylex();
			if(yylval.flag != REG && yylval.flag != SP) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			opnd2 = yylex();
			if(yylval.flag != REG && yylval.flag != SP) {
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			opnd1Value = (yylval.flag==REG ? getInteger(reg[opnd1]) : getInteger(reg[SP_REG]));
			opnd2Value = (yylval.flag==REG ? getInteger(reg[opnd2]) : getInteger(reg[SP_REG]));
//  			printf("opndVal1 = %d \n opndVal2 = %d\n", opnd1Value, opnd2Value);
			translatedAddr1 = translate(opnd1Value);
			translatedAddr2 = translate(opnd2Value);
// 			printf("Page1 = %d\n offset1 = %d\n", translatedAddr1.page_no, translatedAddr1.word_no);
// 			printf("Page2 = %d\n offset2 = %d\n", translatedAddr2.page_no, translatedAddr2.word_no);
			strcpy(page[translatedAddr1.page_no].word[translatedAddr1.word_no], page[translatedAddr2.page_no].word[translatedAddr2.word_no]);
// 			printf("---- %s -----\n", page[translatedAddr1.page_no].word[translatedAddr1.word_no]);
// 			printf("---- %d -----\n", getInteger(page[translatedAddr1.page_no].word[translatedAddr1.word_no]));
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;	
		case LOAD:
			opnd1 = yylex();
			flag1 = yylval.flag;
			opnd2 = yylex();
			flag2 = yylval.flag;
			
			switch(flag1){
				case REG: 
					result = getInteger(reg[opnd1]);
					break;
				case MEM_REG:
					translatedAddr = translate(getInteger(reg[opnd1]));
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR:
					translatedAddr = translate(opnd1);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;
					break;
			}
			
			switch(flag2)
			{
				case REG:
					result2 = getInteger(reg[opnd2]);
					break;
				case NUM:
					result2 = opnd2;
					break;
				case MEM_REG:
					translatedAddr = translate(getInteger(reg[opnd2]));
					result2 = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					//mem[reg[opnd2]];
				break;
				case MEM_DIR:
					translatedAddr = translate(opnd2);
					result2 = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				default:
					exception("Illegal Operand", EX_ILLOPERAND, 0);
					return;
					break;
			}
			
			
			
			
// 			printf("ReadFromDisk: page = %d\n diskBlock = %d\n", result, result2);
			readFromDisk(result, result2);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;	
		
		case STORE:					//note:modified
			opnd1 = yylex();
			flag1 = yylval.flag;
			opnd2 = yylex();
			flag2 = yylval.flag;
			
			switch(flag1){
				case REG: 
					result = getInteger(reg[opnd1]);
					break;
				case NUM:
					result = opnd1;
					break;
				case MEM_REG:
					translatedAddr = translate(getInteger(reg[opnd1]));
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR:
					translatedAddr = translate(opnd1);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				default:
					exception("Illegal operand1", EX_ILLOPERAND, 0);
					return;
					break;
			}
			
			switch(flag2){
				case REG:
					result2 = getInteger(reg[opnd2]);
					break;
				case NUM:
					result2 = opnd2;
					break;
				case MEM_REG:
					translatedAddr = translate(getInteger(reg[opnd2]));
					result2 = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					//mem[reg[opnd2]];
					break;
				case MEM_DIR:
					translatedAddr = translate(opnd2);
					result2 = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				default:
					exception("Illegal Operand2", EX_ILLOPERAND, 0);
					return;
					break;
			}
			
			
			
			writeToDisk(result2, result);
			//printf("%d\n", result);
			//printf("%d\n", result2);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
			
		case HALT:
			printf("Machine is halting\n");
			exit(0);
			break;
			
		case END:
			printf("Machine is exiting\n");
			break;
		
		case BRKP:
			printf("Values in registers after executing instruction :%s\n", instruction);
			printRegisters();
			printf("Press X to exit or any other key to continue.....\n");
			char ch;
			scanf("%c",&ch);
			break;
		default:
			exception("Illegal instruction\n", EX_ILLINSTR, 0);
			return;
	}
}
