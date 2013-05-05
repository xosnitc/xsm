#include "simulator.h"

//unsigned long long int tempCount1=0;
/*
   note : -g for debug mode -d for disabling timer interrupt
*/
main(int argc,char **argv){
	int intDisable=0,flag_intValue;
	initialize_debug();
	if(argc >= 2)
	{
		if(strcmp(argv[1],"--debug") == 0 || strcmp(argv[1],"-d") == 0)
			db_mode = ENABLE;
		else
		{
			char *flag_name = strtok(argv[1], "=");	
			char *flag_value = strtok(NULL, "=");
			if(strcmp(flag_name, "--timer") == 0 || strcmp(flag_name, "-t") == 0)
			{
				flag_intValue = -1;
				if(flag_value != NULL)
					flag_intValue=getInteger(flag_value);
				if(flag_intValue >= 1 && flag_intValue <= 1024)
					TIMESLICE = flag_intValue;
				else if(flag_intValue == 0)
					intDisable = 1;
				else
				{
					printf("Invalid arguement %d to timer flag. Timer value should be between 0 and 1024\n", flag_intValue);
					exit(0);
				}
			}
			else
			{
				printf("Invalid arguement %s", argv[1]);
				exit(0);
			}
		}
	}
	if(argc == 3)
	{
		if(strcmp(argv[2],"--debug") == 0 || strcmp(argv[2],"-d") == 0)
			db_mode = ENABLE;
		else
		{
			char *flag_name = strtok(argv[2], "=");	
			char *flag_value = strtok(NULL, "=");
			if(strcmp(flag_name, "--timer") == 0 || strcmp(flag_name, "-t") == 0)
			{
				flag_intValue = -1;
				if(flag_value != NULL)
					flag_intValue=getInteger(flag_value);
				if(flag_intValue >= 1 && flag_intValue <= 1024)
					TIMESLICE = flag_intValue;
				else if(flag_intValue == 0)
					intDisable = 1;
				else
				{
					printf("Invalid arguement %d to timer flag. Timer value should be between 0 and 1024\n", flag_intValue);
					exit(0);
				}
			}
			else
			{
				printf("Invalid arguement %s", argv[2]);
				exit(0);
			}
		}
	}
	initializeRegs();  
	time_counter = TIMESLICE;
	run(db_mode, intDisable);	
}



/*
  This function does the following:
    1. Loads OS Startup Code.
    2. Copies the instruction to be parsed as per the address specified by the IP register.
    3. Checks whether interrupt is disabled. If not th clock ticks.
    4. Begins the lexical analysis by getting the first token and passing it as arguement to Executeoneinstr.
    5. If step flag is enabled enters debug mode
    6. Finally checks if time slice allocated is over or not. If yes and mode is user mode ,and if interrupt is enabled then
      INT 0 code is run.
*/
void run(int db_mode, int intDisable) {
	loadStartupCode();
	int instr;
	while(1)
	{
		YY_FLUSH_BUFFER;
		if(getInstruction(instruction) == -1)		//gets the next instruction in variable instruction
			continue;		
		instr = yylex();
		if(mode == USER_MODE && !intDisable) 
			tick();
		Executeoneinstr(instr);
		if( (watch_count > 0 && checkWatch() == ENABLE) || step_flag == ENABLE)
			debug_interface();
		if(is_time_zero() && !intDisable && mode==USER_MODE)
		{
			reset_timer();
			runInt0Code();
			if(step_flag == ENABLE)
				printf("TIMER Interrupt\n");
		}
	}
}



/*
  This code is used to execute each ininstruction.
*/
void Executeoneinstr(int instr)
{
//	printf("\n%d:Enter:%s\n",getInteger(reg[IP_REG]),instruction); //Debugging
	int opnd1,opnd2,flag1,flag12,flag2,flag22,oper,result, result2;
	int opnd1Value;
	int opnd2Value;
	int i;
	int type,type2;
	char string[WORD_SIZE];
	char str_result[WORD_SIZE],str_result2[WORD_SIZE];
	char charRead;
	struct address translatedAddr;
	struct address translatedAddr1, translatedAddr2;
	switch(instr)
	{
		case START:			
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);	//increment IP
			break;
		case MOV:						//1st phase:get the value		2nd phase:store the value
		{
			opnd1 = yylex();
			flag1 = yylval.flag;
			flag12 = yylval.flag2;
			opnd2 = yylex();
			flag2 = yylval.flag;
			flag22 = yylval.flag2;
			bzero(string,16);
			strcpy(string,yylval.data);
			//printf("\n%d:MID:%s\nop1:%d\tfl1:%d\tfl2:%d\nop2:%d\tfl1:%d\tfl2:%d\n",getInteger(reg[IP_REG]),instruction,
			//opnd1,flag1,flag12,opnd2,flag2,flag22); //Debugging
			switch(flag2)
			{
				case REG:
					if(mode == USER_MODE && opnd2 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd2 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else
					{
						type = getType(reg[opnd2]);
						strcpy(str_result,reg[opnd2]);
					}
					break;
				case SP:
					type = getType(reg[SP_REG]);
					strcpy(str_result,reg[SP_REG]);
					break;
				case BP: 
					type = getType(reg[BP_REG]);
					strcpy(str_result,reg[BP_REG]);
					break;
				case IP:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						strcpy(str_result,reg[IP_REG]);
					break;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						strcpy(str_result,reg[PTBR_REG]);
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						strcpy(str_result,reg[PTLR_REG]);
					break;
				case EFR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						strcpy(str_result,reg[EFR_REG]);
					break;
				case NUM:
					sprintf(str_result,"%d",opnd2);
					break;
				case STRING:
					strcpy(str_result,string);
					break;
				case MEM_REG:
				{
					if(mode == USER_MODE)
					{
						if(opnd2 >= NO_USER_REG)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[opnd2]) == TYPE_STR || getType(reg[PTLR_REG]) == TYPE_STR || getInteger(reg[opnd2]) < 0 || getInteger(reg[opnd2]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)
						{
							exception("Illegal address access1", EX_ILLMEM, 0);
							return;
						}							
					}
					else
					{
						if(opnd2 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
						{
							exception("Illegal register access", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[opnd2]) == TYPE_STR || getInteger(reg[opnd2]) < 0 || getInteger(reg[opnd2]) >= SIZE_OF_MEM)
						{
							exception("Illegal address access1", EX_ILLMEM, 0);
							return;
						}
					}
					translatedAddr = translate(getInteger(reg[opnd2]));
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				}
				case MEM_SP:
					if(getType(reg[SP_REG]) == TYPE_STR)
					{
						exception("Illegal address access.\nSP value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal PTLR value", EX_ILLMEM, 0);
						return;
					}			
					if(getInteger(reg[SP_REG]) < 0 || (mode == USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && getInteger(reg[SP_REG]) >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nSP value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[SP_REG]));
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_BP:
					if(getType(reg[BP_REG]) == TYPE_STR)
					{
						exception("Illegal address access.\nBP value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal PTLR value", EX_ILLMEM, 0);
						return;
					}
					if(getInteger(reg[BP_REG]) < 0 || (mode == USER_MODE && getInteger(reg[BP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && getInteger(reg[BP_REG]) >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nBP value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[BP_REG]));
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_IP:							
					exception("Cannot use memory reference with IP in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_PTBR:
					if(getType(reg[PTBR_REG]) == TYPE_STR)
					{
						exception("Illegal PTBR value.", EX_ILLMEM, 0);
						return;
					}
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_PTLR:
					if(getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal PTLR value.", EX_ILLMEM, 0);
						return;
					}
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_EFR:							
					exception("Cannot use memory reference with EFR in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR:
					if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal PTLR value", EX_ILLMEM, 0);
						return;
					}
					if(opnd2 < 0 || (mode == USER_MODE && opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd2 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd2);
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_REG:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && flag22 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;					
					}
					else if(mode == KERNEL_MODE && flag22 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[flag22]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_SP:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[SP_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case MEM_DIR_BP:
					if(flag1 == MEM_DIR_REG || flag1 == MEM_DIR_SP || flag1 == MEM_DIR_BP || flag1 == MEM_DIR_PTBR || flag1 == MEM_DIR_PTLR || flag1 == MEM_DIR_IN)
					{
						exception("Illegal Operands", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[BP_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
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
					else if(getType(reg[PTBR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
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
					else if(getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
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
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(str_result, page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				default:
					exception("Illegal Operand,default2", EX_ILLOPERAND, 0);
					return;
				break;
			}
			switch(flag1)
			{
				case REG:
					if(mode == USER_MODE && opnd1 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else 
						strcpy(reg[opnd1], str_result);
					break;
				case SP:
					strcpy(reg[SP_REG], str_result);
					break;
				case BP: 
					strcpy(reg[BP_REG], str_result);
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
						strcpy(reg[PTBR_REG], str_result);
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						strcpy(reg[PTLR_REG], str_result);
					break;
				case EFR:
					exception("Illegal operand EFR. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;					
					break;
				case MEM_REG:
					if(mode == USER_MODE)
					{
						if(opnd1 >= NO_USER_REG)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[opnd1]) == TYPE_STR || getType(reg[PTLR_REG]) == TYPE_STR)
						{
							exception("Illegal register value", EX_ILLMEM, 0);
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
						if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
						{
							exception("Illegal register access", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[opnd1]) == TYPE_STR)
						{
							exception("Illegal register value", EX_ILLMEM, 0);
							return;
						}
						else if(getInteger(reg[opnd1]) < 0 || getInteger(reg[opnd1]) >= SIZE_OF_MEM)
						{
							exception("Illegal address access1", EX_ILLMEM, 0);
							return;
						}
					}
					translatedAddr = translate(getInteger(reg[opnd1]));
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_SP:
					if(getType(reg[SP_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
						return;
					}
					else if(getInteger(reg[SP_REG]) < 0 || (mode == USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && getInteger(reg[SP_REG]) >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nSP value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else					
						translatedAddr = translate(getInteger(reg[SP_REG]));
				    if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
				    strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_BP:
				    if(getType(reg[BP_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
						return;
					}
					else if(getInteger(reg[BP_REG]) < 0 || (mode == USER_MODE && getInteger(reg[BP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && getInteger(reg[BP_REG]) >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nBP value is out of bounds.", EX_ILLMEM, 0);
						return;
					} 
				    else
				    	translatedAddr = translate(getInteger(reg[BP_REG]));
				    if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
				    strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
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
					else if(getType(reg[PTBR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(getInteger(reg[PTBR_REG]) < 0 || getInteger(reg[PTBR_REG]) >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nRegister value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[PTBR_REG]));
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(getInteger(reg[PTLR_REG]) < 0 || getInteger(reg[PTLR_REG]) >= SIZE_OF_MEM)
					{
						exception("Illegal address access.\nRegister value is out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(getInteger(reg[PTLR_REG]));
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_EFR:							
					exception("Cannot use memory reference with EFR in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR:
					if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
						return;
					}
					else if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access4", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_DIR_REG:
					if(mode == USER_MODE && flag12 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;					
					}
					else if(mode == KERNEL_MODE && flag12 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[flag12]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_DIR_SP:
					if(getType(reg[SP_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
						return;
					}
					opnd1 += getInteger(reg[SP_REG]);
					if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_DIR_BP:
					if(getType(reg[BP_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
						return;
					}
					else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
						return;
					}		
					opnd1 += getInteger(reg[BP_REG]);
					if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						return;
					}
					else
						translatedAddr = translate(opnd1);
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
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
					else if(getType(reg[PTBR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_DIR_PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLOPERAND, 0);
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
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
					break;
				case MEM_DIR_EFR:							
					exception("Cannot use memory reference with EFR in any mode", EX_ILLOPERAND, 0);
					return;
					break;
				case MEM_DIR_IN:
					if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal register value", EX_ILLMEM, 0);
						return;
					}
					opnd1 += flag12;
					if(opnd1 < 0 || (mode == USER_MODE && opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) || (mode == KERNEL_MODE && opnd1 >= SIZE_OF_MEM) )
					{
						 exception("Illegal address access.\nDirect Address value out of bounds.", EX_ILLMEM, 0);
						 return;
					}					
					else
						translatedAddr = translate(opnd1);
					if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
						return;
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no], str_result);
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
			switch(yylval.flag)
			{
				case REG:
					if(mode == USER_MODE && opnd1 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[opnd1]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = opnd1;
					break;
				case SP:
					if(getType(reg[SP_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = SP_REG;
					break;
				case BP:
					if(getType(reg[BP_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = BP_REG;
					break;
				case IP:
					exception("Illegal operand IP. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[PTBR_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = PTBR_REG;
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = PTLR_REG;
					break;
				case EFR:
					exception("Illegal operand EFR. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;					
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;					
			}
			if(oper!= INR && oper!=DCR)
			{
				opnd2 = yylex();
				flag2 = yylval.flag;
				switch(flag2)
				{
					case REG:
						if(mode == USER_MODE && opnd2 >= NO_USER_REG)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(opnd2 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
						{
							exception("Illegal register access", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[opnd2]) == TYPE_STR)
						{
							exception("Illegal operand1", EX_ILLOPERAND, 0);
							return;
						}
						else
							result2 = opnd2;
						break;
					case SP:
						if(getType(reg[SP_REG]) == TYPE_STR)
						{
							exception("Illegal operand1", EX_ILLOPERAND, 0);
							return;
						}
						else
							result2 = SP_REG;
						break;
					case BP:
						if(getType(reg[BP_REG]) == TYPE_STR)
						{
							exception("Illegal operand1", EX_ILLOPERAND, 0);
							return;
						}
						else
							result2 = BP_REG;
						break;
					case IP:
						if(mode == USER_MODE)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[IP_REG]) == TYPE_STR)
						{
							exception("Illegal operand1", EX_ILLOPERAND, 0);
							return;
						}
						else
							result2 = IP_REG;
						break;
					case PTBR:
						if(mode == USER_MODE)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[PTBR_REG]) == TYPE_STR)
						{
							exception("Illegal operand1", EX_ILLOPERAND, 0);
							return;
						}
						else
							result2 = PTBR_REG;
						break;
					case PTLR:
						if(mode == USER_MODE)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[PTLR_REG]) == TYPE_STR)
						{
							exception("Illegal operand1", EX_ILLOPERAND, 0);
							return;
						}
						else
							result2 = PTLR_REG;
						break;
					case EFR:
						if(mode == USER_MODE)
						{
							exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
							return;
						}
						else if(getType(reg[EFR_REG]) == TYPE_STR)
						{
							exception("Illegal operand1", EX_ILLOPERAND, 0);
							return;
						}
						else
							result2 = EFR_REG;
						break;
					case NUM:
						result2 = opnd2;
						break;
					default:
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;					
				}				
			}
			switch(oper)
			{
				case ADD:
					if(flag2 == NUM)
						storeInteger(reg[result],getInteger(reg[result]) + result2);
					else
						storeInteger(reg[result],getInteger(reg[result]) + getInteger(reg[result2]));
					break;			
				case SUB:
					if(flag2 == NUM)
						storeInteger(reg[result],getInteger(reg[result]) - result2);
					else
						storeInteger(reg[result],getInteger(reg[result]) - getInteger(reg[result2]));
					break;
				case MUL:
					if(flag2 == NUM)
						storeInteger(reg[result],getInteger(reg[result]) * result2);
					else
						storeInteger(reg[result],getInteger(reg[result]) * getInteger(reg[result2]));
					break;
				case DIV: 
					if(flag2 == NUM)
					{
						if(result2 == 0)
						{
							exception("Divide by ZERO", EX_ILLOPERAND, 0);
							return;
						}
						storeInteger(reg[result],getInteger(reg[result]) / result2);
					}
					else
					{
						if(getInteger(reg[result2]) == 0)
						{
							exception("Divide by ZERO", EX_ILLOPERAND, 0);
							return;
						}
						storeInteger(reg[result],getInteger(reg[result]) / getInteger(reg[result2]));
					}
					break;
				case MOD:
					if(flag2 == NUM)
					{
						if(result2 == 0)
						{
							exception("Divide by ZERO", EX_ILLOPERAND, 0);
							return;
						}
						storeInteger(reg[result],getInteger(reg[result]) % result2);
					}
					else
					{
						if(getInteger(reg[result2]) == 0)
						{
							exception("Divide by ZERO", EX_ILLOPERAND, 0);
							return;
						}
						storeInteger(reg[result],getInteger(reg[result]) % getInteger(reg[result2]));
					}
					break;
				case INR:
					storeInteger(reg[result],getInteger(reg[result]) + 1);
					break;
				case DCR:
					storeInteger(reg[result],getInteger(reg[result]) - 1);
					break;
				default:
					exception("Illegal Instruction", EX_ILLINSTR, 0);
					return;
					break;
			}
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);			
		}
		break;
		case LOGIC:
		{
			oper = yylval.flag;
			opnd1 = yylex();
			switch(yylval.flag)
			{
				case REG:
					if(mode == USER_MODE && opnd1 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = opnd1;
					break;
				case SP:
					result = SP_REG;
					break;
				case BP:
					result = BP_REG;
					break;
				case IP:
					exception("Illegal operand IP. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = PTBR_REG;
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = PTLR_REG;
					break;
				case EFR:
					exception("Illegal operand EFR. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;					
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;					
			}			
			opnd2 = yylex();
			switch(yylval.flag)
			{
				case REG:
					if(mode == USER_MODE && opnd2 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd2 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = opnd2;
					break;
				case SP:
					result2 = SP_REG;
					break;
				case BP:
					result2 = BP_REG;
					break;
				case IP:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = IP_REG;
					break;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = PTBR_REG;
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = PTLR_REG;
					break;
				case EFR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = EFR_REG;
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;					
			}			
			switch(oper)
			{
				case LT:
					if(getType(reg[result]) == getType(reg[result2]) && getType(reg[result]) == TYPE_INT)					
						storeInteger(reg[result],(getInteger(reg[result]) < getInteger(reg[result2])?1:0));
					else
						storeInteger(reg[result],( strcmp(reg[result],reg[result2]) < 0?1:0 ));
					break;
				case GT:
					if(getType(reg[result]) == getType(reg[result2]) && getType(reg[result]) == TYPE_INT)
						storeInteger(reg[result],(getInteger(reg[result]) > getInteger(reg[result2])?1:0));
					else
						storeInteger(reg[result],( strcmp(reg[result],reg[result2]) > 0?1:0 ));
					break;	
				case EQ:
					if(getType(reg[result]) == getType(reg[result2]) && getType(reg[result]) == TYPE_INT)
						storeInteger(reg[result],(getInteger(reg[result]) == getInteger(reg[result2])?1:0));
					else
						storeInteger(reg[result],( strcmp(reg[result],reg[result2]) == 0?1:0 ));
					break;
				case NE:
					if(getType(reg[result]) == getType(reg[result2]) && getType(reg[result]) == TYPE_INT)
						storeInteger(reg[result],(getInteger(reg[result]) != getInteger(reg[result2])?1:0));
					else
						storeInteger(reg[result],( strcmp(reg[result],reg[result2]) != 0?1:0 ));
					break;	
				case LE:
					if(getType(reg[result]) == getType(reg[result2]) && getType(reg[result]) == TYPE_INT)
						storeInteger(reg[result],(getInteger(reg[result]) <= getInteger(reg[result2])?1:0));
					else
						storeInteger(reg[result],( strcmp(reg[result],reg[result2]) <= 0?1:0 ));
					break;
				case GE:
					if(getType(reg[result]) == getType(reg[result2]) && getType(reg[result]) == TYPE_INT)
						storeInteger(reg[result],(getInteger(reg[result]) >= getInteger(reg[result2])?1:0));
					else
						storeInteger(reg[result],( strcmp(reg[result],reg[result2]) >= 0?1:0 ));
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
				case JNZ:
					opnd1 = yylex();
					switch(yylval.flag)
					{
						case REG:
							if(mode == USER_MODE && opnd1 >= NO_USER_REG)
							{
								exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
								return;
							}
							else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
							{
								exception("Illegal register access", EX_ILLOPERAND, 0);
								return;
							}
							else
								result = opnd1;
							break;
						case SP:
							result = SP_REG;
							break;
						case BP:
							result = BP_REG;
							break;
						case IP:
							if(mode == USER_MODE)
							{
								exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
								return;
							}
							else
								result = IP_REG;
							break;
						case PTBR:
							if(mode == USER_MODE)
							{
								exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
								return;
							}
							else
								result = PTBR_REG;
							break;
						case PTLR:
							if(mode == USER_MODE)
							{
								exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
								return;
							}
							else
								result = PTLR_REG;
							break;
						case EFR:
							if(mode == USER_MODE)
							{
								exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
								return;
							}
							else
								result = EFR_REG;
							break;
						default:
							exception("Illegal operand", EX_ILLOPERAND, 0);
							return;					
					}
					opnd2 = yylex();
					if(yylval.flag != NUM)
					{
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd2 < 0 || opnd2 >= SIZE_OF_MEM)
					{
						exception("Illegal address access", EX_ILLMEM, 0);
						return;
					}
					else if(mode == USER_MODE && (getType(reg[PTLR_REG]) == TYPE_STR || opnd2 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) )
					{
						exception("Illegal address access", EX_ILLMEM, 0);
						return;
					}					
					if( (getType(reg[result]) == TYPE_INT && oper==JZ && getInteger(reg[result]) == 0) 
					|| (getType(reg[result]) == TYPE_STR || (oper==JNZ && getInteger(reg[result]) != 0)) )
					{						
						storeInteger(reg[IP_REG], opnd2);
						
					}
					else
						storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
					break;				
				case JMP:
					opnd1 = yylex();
					if(yylval.flag != NUM)
					{
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 < 0 || opnd1 >= SIZE_OF_MEM)
					{
						exception("Illegal address access", EX_ILLMEM, 0);
						return;
					}
					else if(mode == USER_MODE && (getType(reg[PTLR_REG]) == TYPE_STR || opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) )
					{
						exception("Illegal address access", EX_ILLMEM, 0);
						return;
					}					
					storeInteger(reg[IP_REG], opnd1);
					
					break;				
				default:
					exception("Illegal Branch Instruction", EX_ILLINSTR, 0);
					return;
					break;
			}
		}
		break;
								
		case PUSH:
			opnd1 = yylex();
			if(getType(reg[SP_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(getInteger(reg[SP_REG]) + 1 < 0)
			{
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}			
			else if(getInteger(reg[SP_REG]) + 1 >= SIZE_OF_MEM || (mode==USER_MODE && getInteger(reg[SP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE))
			{
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}			
			translatedAddr = translate(getInteger(reg[SP_REG])+1);
			if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
				return;
			switch(yylval.flag)
			{
				case REG:
					if(mode == USER_MODE && opnd1 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else
						strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no],reg[opnd1]);
					break;
				case SP:
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no],reg[SP_REG]);
					break;
				case BP:
					strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no],reg[BP_REG]);
					break;
				case IP:
					if(mode == USER_MODE)
					{
						exception("IP not accessible in USER Mode", EX_ILLOPERAND,0);
						return;						
					}
					else
						strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no],reg[IP_REG]);
					break;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("PTBR not accessible in USER Mode", EX_ILLOPERAND,0);
						return;						
					}
					else
						strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no],reg[PTBR_REG]);
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("PTLR not accessible in USER Mode", EX_ILLOPERAND,0);
						return;						
					}
					else
						strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no],reg[PTLR_REG]);
					break;
				case EFR:
					if(mode == USER_MODE)
					{
						exception("EFR not accessible in USER Mode", EX_ILLOPERAND,0);
						return;						
					}
					else
						strcpy(page[translatedAddr.page_no].word[translatedAddr.word_no],reg[EFR_REG]);
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;
			}
			storeInteger(reg[SP_REG],getInteger(reg[SP_REG])+1);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
			
		case POP:
			opnd1 = yylex();
			if(getType(reg[SP_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(getInteger(reg[SP_REG]) < 0)
			{
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}			
			else if(getInteger(reg[SP_REG]) >= SIZE_OF_MEM || (mode==USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE))
			{
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}
			translatedAddr = translate(getInteger(reg[SP_REG]));
			if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
				return;
			switch(yylval.flag)
			{
				case REG:
					if(mode == USER_MODE && opnd1 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else
						strcpy(reg[opnd1], page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case SP:
					strcpy(reg[SP_REG], page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case BP:
					strcpy(reg[BP_REG], page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;
				case IP:
					exception("IP cannot be modified.", EX_ILLOPERAND, 0);
					return;
					break;	
				case PTBR:
					if(mode == USER_MODE)
					{
					  	exception("Trying to modify PTBR in USER mode", EX_ILLOPERAND, 0);
						return;
					}
					strcpy(reg[PTBR_REG], page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;	
				case PTLR:
					if(mode == USER_MODE)
					{
					  	exception("Trying to modify PTLR in USER mode", EX_ILLOPERAND, 0);
						return;
					}
					strcpy(reg[PTLR_REG], page[translatedAddr.page_no].word[translatedAddr.word_no]);
					break;				
				case EFR:
					exception("EFR cannot be modified", EX_ILLOPERAND, 0);
					return;
					break;					
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;
					break;
			}
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG])-1);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
		case CALL:
			opnd1 = yylex();
			if(getType(reg[SP_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(getInteger(reg[SP_REG]) + 1 < 0)
			{
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}			
			else if(getInteger(reg[SP_REG]) + 1 >= SIZE_OF_MEM || (mode==USER_MODE && getInteger(reg[SP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE))
			{
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}			
			else if(yylval.flag != NUM) 
			{
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			else if(opnd1 < 0 || opnd1 >= SIZE_OF_MEM)
			{
				exception("Illegal address access", EX_ILLMEM, 0);
				return;
			}
			else if(mode == USER_MODE && (getType(reg[PTLR_REG]) == TYPE_STR || opnd1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) )
			{
				exception("Illegal address access", EX_ILLMEM, 0);
				return;
			}
			translatedAddr = translate(getInteger(reg[SP_REG])+1);
			if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
				return;
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
			storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], getInteger(reg[IP_REG]) + WORDS_PERINSTR);
			storeInteger(reg[IP_REG], opnd1);
			
			break;
		case RET:
			if(getType(reg[SP_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(getInteger(reg[SP_REG]) < 0)
			{
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}			
			else if(getInteger(reg[SP_REG]) >= SIZE_OF_MEM || (mode==USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE))
			{
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}
			translatedAddr = translate(getInteger(reg[SP_REG]));
			if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
				return;
			else if(getType(page[translatedAddr.page_no].word[translatedAddr.word_no]) == TYPE_STR)
			{
				exception("Illegal return address", EX_ILLMEM, 0);
				return;
			}
			result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
			if(result < 0 || result >= SIZE_OF_MEM)
			{
				exception("Illegal return address", EX_ILLMEM, 0);
				return;
			}
			else if(mode == USER_MODE && (getType(reg[PTLR_REG]) == TYPE_STR || result >= getInteger(reg[PTLR_REG]) * PAGE_SIZE) )
			{
				exception("Illegal return address", EX_ILLMEM, 0);
				return;
			}			
			storeInteger(reg[IP_REG], result);
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) - 1);
			
			break;
		case INT:
			if(mode == KERNEL_MODE)
			{
			  	exception("Cannot call INT in KERNEL mode", EX_ILLINSTR, 0);
				return;
			}
			opnd1 = yylex();
			if(yylval.flag != NUM)
			{
				exception("Illegal operand", EX_ILLOPERAND, 0);
				return;
			}
			else if(opnd1 < 1 || opnd1 > 7) 
			{
				exception("Illegal INT instruction\n", EX_ILLOPERAND, 0);
				return;
			}
			else if(getType(reg[SP_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(getInteger(reg[SP_REG]) + 1 < 0)
			{
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}			
			else if(getInteger(reg[SP_REG]) + 1 >= SIZE_OF_MEM || (mode==USER_MODE && getInteger(reg[SP_REG]) + 1 >= getInteger(reg[PTLR_REG]) * PAGE_SIZE))
			{
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}		
			translatedAddr = translate(getInteger(reg[SP_REG])+1);
			if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
				return;
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
			storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no],getInteger(reg[IP_REG]) + WORDS_PERINSTR);
			storeInteger(reg[IP_REG], ( (opnd1*PAGE_PER_INT) + INT_START_PAGE) * PAGE_SIZE);
			mode = KERNEL_MODE;
			break;
		
		case IRET:
			if(mode == USER_MODE)
			{
			  	exception("Call to Privileged Instruction IRET in USER mode", EX_ILLINSTR, 0);
				return;
			}
			else if(getType(reg[SP_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(mode == USER_MODE && getType(reg[PTLR_REG]) == TYPE_STR)
			{
				exception("Illegal Register Value", EX_ILLMEM, 0);
				return;
			}
			else if(getInteger(reg[SP_REG]) < 0)
			{
				exception("Stack Underflow", EX_ILLMEM, 0);
				return;
			}			
			else if(getInteger(reg[SP_REG]) >= SIZE_OF_MEM || (mode==USER_MODE && getInteger(reg[SP_REG]) >= getInteger(reg[PTLR_REG]) * PAGE_SIZE))
			{
				exception("Stack Overflow", EX_ILLMEM, 0);
				return;
			}
			mode = USER_MODE;			
			translatedAddr = translate(getInteger(reg[SP_REG]));
			if(translatedAddr.page_no == -1 && translatedAddr.word_no == -1)
			{
				mode = KERNEL_MODE;
				return;
			}
			else if(getType(page[translatedAddr.page_no].word[translatedAddr.word_no]) == TYPE_STR)
			{
				mode = KERNEL_MODE;
				exception("Illegal return address", EX_ILLMEM, 0);
				return;
			}
			result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
			if(result < 0 || getType(reg[PTLR_REG]) == TYPE_STR || result >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)
			{
				mode = KERNEL_MODE;
				exception("Illegal return address", EX_ILLMEM, 0);
				return;
			}
			storeInteger(reg[IP_REG], result);
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) - 1);			
			break;
		case IN:
			opnd1 = yylex();
			switch(yylval.flag)
			{
				case REG:
					if(mode == USER_MODE && opnd1 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = opnd1;
					break;
				case SP:
					result = SP_REG;
					break;
				case BP:
					result = BP_REG;
					break;
				case IP:
					exception("Illegal operand IP. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = PTBR_REG;
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = PTLR_REG;
					break;
				case EFR:
					exception("Illegal operand EFR. Cannot alter readonly register", EX_ILLOPERAND, 0);
					return;					
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;					
			}
			char input[WORD_SIZE];
			scanf("%s",input);
			FLUSH_STDIN(input);    // strip newline, flush extra chars
			input[WORD_SIZE-1] = '\0';
			strcpy(reg[result], input);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
		
		case OUT:
			opnd1 = yylex();
			switch(yylval.flag)
			{
				case REG:
					if(mode == USER_MODE && opnd1 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = opnd1;
					break;
				case SP:
					result = SP_REG;
					break;
				case BP:
					result = BP_REG;
					break;
				case IP:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = IP_REG;
					break;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = PTBR_REG;
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = PTLR_REG;
					break;
				case EFR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = EFR_REG;					
					break;
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;					
			}
			printf("%s\n",reg[result]);
			fflush(stdout);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
		case LOAD:
		case STORE:	
			if(mode == USER_MODE)
			{
			  	exception("Call to Privileged Instruction in USER mode", EX_ILLINSTR, 0);
				return;
			}
			opnd1 = yylex();
			flag1 = yylval.flag;
			opnd2 = yylex();
			flag2 = yylval.flag;			
			switch(flag1)
			{
				case REG:
					if(mode == USER_MODE && opnd1 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd1 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[opnd1]) == TYPE_STR)
					{
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[opnd1]);
					break;
				case SP:
					if(getType(reg[SP_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[SP_REG]);
					break;
				case BP:
					if(getType(reg[BP_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[BP_REG]);
					break;
				case IP:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[IP_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
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
					else if(getType(reg[PTBR_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
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
					else if(getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
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
					else if(getType(reg[EFR_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result = getInteger(reg[EFR_REG]);
					break;
				case NUM:
					result = opnd1;
					break;				
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;
					break;
			}
			switch(flag2)
			{
				case REG:
					if(mode == USER_MODE && opnd2 >= NO_USER_REG)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(opnd2 >= NO_USER_REG + NO_SYS_REG + NO_TEMP_REG)
					{
						exception("Illegal register access", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[opnd2]) == TYPE_STR)
					{
						exception("Illegal operand", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = getInteger(reg[opnd2]);
					break;
				case SP:
					if(getType(reg[SP_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = getInteger(reg[SP_REG]);
					break;
				case BP:
					if(getType(reg[BP_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = getInteger(reg[BP_REG]);
					break;
				case IP:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[IP_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = getInteger(reg[IP_REG]);
					break;
				case PTBR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[PTBR_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = getInteger(reg[PTBR_REG]);
					break;
				case PTLR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[PTLR_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = getInteger(reg[PTLR_REG]);
					break;
				case EFR:
					if(mode == USER_MODE)
					{
						exception("Illegal register access in user mode", EX_ILLOPERAND, 0);
						return;
					}
					else if(getType(reg[EFR_REG]) == TYPE_STR)
					{
						exception("Illegal operand1", EX_ILLOPERAND, 0);
						return;
					}
					else
						result2 = getInteger(reg[EFR_REG]);
					break;
				case NUM:
					result2 = opnd2;
					break;				
				default:
					exception("Illegal operand", EX_ILLOPERAND, 0);
					return;
					break;
			}
			if(instr == LOAD)
			{			
				emptyPage(result);
				readFromDisk(result, result2);
			}
			else if(instr == STORE)
				writeToDisk(result2, result);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;	
				
		case HALT:
			if(mode == USER_MODE)
			{
			  	exception("Call to Privileged Instruction HALT in USER mode", EX_ILLINSTR, 0);
				return;
			}
			printf("Machine is halting\n");
			exit(0);
			break;			
		case END:
			break;
		
		case BRKP:
			if(db_mode == ENABLE)
			{
				step_flag = ENABLE;
				printf("\nXSM Debug Environment\nType \"help\" for  getting a list of commands\n");
			}
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+WORDS_PERINSTR);
			break;
		default:
			exception("Illegal instruction\n", EX_ILLINSTR, 0);
			return;
	}
}
