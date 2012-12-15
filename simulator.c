#include "simulator.h"

unsigned long long int tempCount1=0;
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
		if(getInteger(reg[IP_REG])<0 || getInteger(reg[IP_REG])>SIZE_OF_MEM){			//checks if address is outside limits
		    exception("IP Register value out of bounds");
		}
		if(mode == USER_MODE){						//checks if address is outside limits if mode is user mode
			if(getInteger(reg[IP_REG]) < 0 || getInteger(reg[IP_REG]) >= NUM_CODE_PAGES * PAGE_SIZE) {
				printf("%d", getInteger(reg[IP_REG]));
				exception("Illegal IP access1");
				continue;
			}
		}
		translatedAddr = translate(getInteger(reg[IP_REG]));
		strcpy(instruction,page[translatedAddr.page_no].word[translatedAddr.word_no]);
//  		printf("%s\n", instruction); // note:debugging
		translatedAddr.word_no = -1;
		translatedAddr.page_no = -1;
		instr = yylex();
		if(mode == USER_MODE && !intDisable) 
			tick();
		tempCount1++;
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
	int opnd1,opnd2,flag1,flag2,oper,result, result2;
	int opnd1Value;
	int opnd2Value;
	int i;
	char charRead;
	struct address translatedAddr;
	struct address translatedAddr1, translatedAddr2;;
	switch(instr)
	{
		case START: storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);	//increment IP
		break;
		case MOV:						//1st phase:get the value		2nd phase:store the value
		{
			opnd1 = yylex();
			flag1 = yylval.flag;
			opnd2 = yylex();
			flag2 = yylval.flag;
			switch(flag2)
			{
				case REG:
				  result = getInteger(reg[opnd2]);
				  break;
				case SP: 
				  result = getInteger(reg[SP_REG]);
				  break;
				case BP: 
				  result = getInteger(reg[BP_REG]);
				  break;
				case IP:
				  result = getInteger(reg[IP_REG]);
				  break;
// 				case PID:
// 					result = getInteger(reg[PID_REG]);;
				case NUM:
				  result = opnd2;
				  break;
				case MEM_REG:
				{
					char c;
					if(mode == USER_MODE)
					{
						if(getInteger(reg[opnd2]) < STACK_START_ADDRESS || getInteger(reg[opnd2]) >= STACK_START_ADDRESS + PAGE_SIZE) {
							exception("Illegal address access1");
						}
						translatedAddr = translate(getInteger(reg[opnd2]));
					}
					else
						translatedAddr = translate(getInteger(reg[opnd2]));
// 					printf("$$%d$$\n", getInteger(reg[opnd2]));
// 					scanf("%c", &c);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
					//mem[reg[opnd2]];
				}
				break;
				case MEM_SP:							//note: modified here
				  if(mode == KERNEL_MODE){
						exception("Cannot use memory reference with SP when in Kernel Mode");
					}
				  else if(getInteger(reg[SP_REG]) < STACK_START_ADDRESS || getInteger(reg[SP_REG]) >= STACK_START_ADDRESS + PAGE_SIZE) {
						exception("Illegal address access.\nSP value is out of bounds.");
					}
				    
				  translatedAddr = translate(getInteger(reg[SP_REG]));
				  result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
				  break;
				case MEM_BP:							//note:modified here
				  if(mode == KERNEL_MODE){
						exception("Cannot use memory reference with BP when in Kernel Mode");
					}
				  else if(getInteger(reg[BP_REG]) < STACK_START_ADDRESS || getInteger(reg[BP_REG]) >= STACK_START_ADDRESS + PAGE_SIZE) {
						exception("Illegal address access.\nBP value is out of bounds.");
					}
				    
				  translatedAddr = translate(getInteger(reg[BP_REG]));
				  result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
				break;
				case MEM_IP:							//note:modified here. Also note that this will never happen
				  exception("Cannot use memory reference with IP in any mode");
				break;
				case MEM_DIR:
					if(mode == USER_MODE) {
						if(opnd2 < STACK_START_ADDRESS || opnd2 >= STACK_START_ADDRESS + PAGE_SIZE) {
							exception("Illegal address access.\nDirect Address value out of bounds.");
						}
						translatedAddr = translate(opnd2);
					}
					else
						translatedAddr = translate(opnd2);
					result = getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]);
				break;
				default:
					exception("Illegal Operand");
				break;
			}
			switch(flag1)
			{
				case REG: 
				  storeInteger(reg[opnd1], result);
				  break;
				case SP: //can modify this to show error if result exceeds value that can be stored in SP.
				  storeInteger(reg[SP_REG], result);
				  break;
				case BP: //same as above. 
				  storeInteger(reg[BP_REG], result);
				  break;
// 				case PID: //same as above. 
// 					storeInteger(reg[PID_REG], result);
// 					break;
				case IP:
					if(mode == USER_MODE) {
						exception("Illegal operand IP");
					}
					else {	//note: modified here
						//commented the lines 1. struct address translatedAddr = translate(result);and 2. mode = USER_MODE.
// 						struct address translatedAddr = translate(result);
						storeInteger(reg[IP_REG], result);
// 						mode = USER_MODE;//error: why is this needed
					}	
				break;
				case MEM_REG:
					if(mode == USER_MODE) {
						if(getInteger(reg[opnd1]) < STACK_START_ADDRESS || getInteger(reg[opnd1]) >= STACK_START_ADDRESS + PAGE_SIZE) {
							exception("Illegal address access.\n Value in register out of bounds.");
						}
						translatedAddr = translate(getInteger(reg[opnd1]));
					}
					else
						translatedAddr = translate(getInteger(reg[opnd1]));
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
					//storeInteger(page[getInteger(page[KERNEL_PAGETBL_PAGENO].word[KERNEL_PAGETBL_START_WORD + STACK_PAGE])].word[getInteger(reg[opnd1])], result);
					//mem[reg[opnd1]] = result;
				break;
				case MEM_SP:	//note:Modified here
				    if(mode == KERNEL_MODE){
						exception("Cannot use memory reference with SP when in Kernel Mode");
					}
				    else if(getInteger(reg[SP_REG]) < STACK_START_ADDRESS || getInteger(reg[SP_REG]) >= STACK_START_ADDRESS + PAGE_SIZE) {
						exception("Illegal address access.\nSP value is out of bounds.");
					}  
				    translatedAddr = translate(getInteger(reg[SP_REG]));
				    storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
				break;
				case MEM_BP:	//note:Modified here
				    if(mode == KERNEL_MODE){
						exception("Cannot use memory reference with BP when in Kernel Mode");
					}
				    else if(getInteger(reg[BP_REG]) < STACK_START_ADDRESS || getInteger(reg[BP_REG]) >= STACK_START_ADDRESS + PAGE_SIZE) {
						exception("Illegal address access.\nBP value is out of bounds.");
					}  
				    translatedAddr = translate(getInteger(reg[BP_REG]));
				    storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
				break;
				case MEM_IP:	//note:Modified here
// 					storeInteger(page[getInteger(page[KERNEL_PAGETBL_PAGENO].word[KERNEL_PAGETBL_START_WORD + STACK_PAGE])].word[getInteger(reg[IP_REG])  % PAGE_SIZE], result);
				    error("Cannot use memory reference with IP in any mode.");
				break;
				case MEM_DIR:
					if(mode == USER_MODE) {
						if(opnd1 < STACK_START_ADDRESS || opnd1 >= STACK_START_ADDRESS + PAGE_SIZE) {
							exception("Illegal address access4");
						}
						translatedAddr = translate(opnd1);
					}
					else
						translatedAddr = translate(opnd1);
					storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], result);
				break;
				default:
					exception("Illegal operand");
				break;
			}
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
		}
		break;
		case ARITH:
		{
			oper = yylval.flag;
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand1");
			}
			else
			{
				if(oper!= INR && oper!=DCR)
				{
					opnd2 = yylex();
					if(yylval.flag != REG) {
						exception("Illegal operand");
					}
				}
				switch(oper)
				{
					case ADD: storeInteger(reg[opnd1],getInteger(reg[opnd1]) + getInteger(reg[opnd2]));
					break;			
					case SUB: storeInteger(reg[opnd1],getInteger(reg[opnd1]) - getInteger(reg[opnd2]));
					break;
					case MUL: storeInteger(reg[opnd1],getInteger(reg[opnd1]) * getInteger(reg[opnd2]));
					break;
					case DIV: 
						if(getInteger(reg[opnd2]) == 0) {
							exception("Divide by ZERO");
						}
						storeInteger(reg[opnd1],getInteger(reg[opnd1]) / getInteger(reg[opnd2]));
					break;
					case MOD: storeInteger(reg[opnd1],getInteger(reg[opnd1]) % getInteger(reg[opnd2]));
					break;
					case INR: storeInteger(reg[opnd1],getInteger(reg[opnd1]) + 1);
					break;
					case DCR: storeInteger(reg[opnd1],getInteger(reg[opnd1]) - 1);
					break;
					default:
						exception("Illegal Instruction");
					break;
				}
				storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
			}
			
		}
		break;
		case LOGIC:
		{
			oper = yylval.flag;
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand");
			}
			opnd2 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand");
			}
			switch(oper)
			{
				case LT:storeInteger(reg[opnd1],(getInteger(reg[opnd1]) < getInteger(reg[opnd2])?1:0));
				break;
				case GT:storeInteger(reg[opnd1],(getInteger(reg[opnd1]) > getInteger(reg[opnd2])?1:0));
				break;	
				case EQ:storeInteger(reg[opnd1],(getInteger(reg[opnd1]) == getInteger(reg[opnd2])?1:0));
				break;
				case NE:storeInteger(reg[opnd1],(getInteger(reg[opnd1]) != getInteger(reg[opnd2])?1:0));
				break;	
				case LE:storeInteger(reg[opnd1],(getInteger(reg[opnd1]) <= getInteger(reg[opnd2])?1:0));
				break;
				case GE:storeInteger(reg[opnd1],(getInteger(reg[opnd1]) >= getInteger(reg[opnd2])?1:0));
				break;	
				default:exception("Illegal Instruction");
				break;
			}
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
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
						exception("Illegal operand");
					}
					opnd2 = yylex();
					if(yylval.flag != NUM){
						exception("Illegal operand");
					}
					if(getInteger(reg[opnd1]) == 0)
					{
						if(mode == USER_MODE) {
							if(opnd2 < 0 || opnd2 >= NUM_CODE_PAGES * PAGE_SIZE) {
								exception("Illegal address access");
							}
						}
						storeInteger(reg[IP_REG], opnd2);
						YY_FLUSH_BUFFER;
					}
					else
						storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
					break;
				
				case JNZ:
					opnd1 = yylex();
					if(yylval.flag != REG){
						exception("Illegal operand");
					}
					opnd2 = yylex();
					if(yylval.flag != NUM) {
						exception("Illegal operand");
					}
					if( getInteger(reg[opnd1]) != 0)
					{
						if(mode == USER_MODE) {
							if(opnd2 < 0 || opnd2 >= NUM_CODE_PAGES * PAGE_SIZE) {
								exception("Illegal address access");
							}
						}
						storeInteger(reg[IP_REG], opnd2);
						YY_FLUSH_BUFFER;
					}
					else
						storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
					break;
				
				case JMP:
					opnd1 = yylex();
					if(yylval.flag != NUM){
						exception("Illegal operand");
					}
					if(mode == USER_MODE) {
						if(opnd1 < 0 || opnd1 >= NUM_CODE_PAGES * PAGE_SIZE) {
							exception("Illegal address access");
						}
					}
					storeInteger(reg[IP_REG], opnd1);
					YY_FLUSH_BUFFER;
					break;
				
				default:
					exception("Illegal Branch Instruction");
				break;
			}
		}
		break;
								
		case PUSH:				//note:Modified here
			if(mode == KERNEL_MODE){
			  exception("PUSH command not available in KERNEL mode");
			}
			opnd1 = yylex();
			if(getInteger(reg[SP_REG]) + 1 < STACK_START_ADDRESS ){
				exception("Stack Underflow");
			}
			if(getInteger(reg[SP_REG]) + 1 >= STACK_START_ADDRESS + PAGE_SIZE){
				exception("Stack Overflow");
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
					exception("Illegal operand");
			}
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
		break;
		case POP:				//note:Modified here
			if(mode == KERNEL_MODE){
			  exception("POP command not available in KERNEL mode");
			}
			opnd1 = yylex();
			if(getInteger(reg[SP_REG]) < STACK_START_ADDRESS){
				exception("Stack Underflow");
			}
			if(getInteger(reg[SP_REG]) >= STACK_START_ADDRESS + PAGE_SIZE){
				exception("Stack Overflow");
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
					  exception("Trying to modify IP in USER mode");
					}
					storeInteger(reg[IP_REG], getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]));
				break;
				default:
					exception("Illegal operand");
				break;
			}
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG])-1);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
		break;
		case CALL:				//note: Modified here.
			if(mode == KERNEL_MODE){
			  exception("Cannot call CALL in KERNEL mode");
			}
			opnd1 = yylex();
			if(yylval.flag != NUM) {
				exception("Illegal operand");
			}
			if(getInteger(reg[SP_REG]) + 1 >= STACK_START_ADDRESS + PAGE_SIZE){
				exception("Stack Overflow");
			}
			if(getInteger(reg[SP_REG]) + 1 < STACK_START_ADDRESS )
			{
				exception("Stack Underflow");
			}
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) + 1);
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG]) + 1);
			translatedAddr = translate(getInteger(reg[SP_REG]));
			storeInteger(page[translatedAddr.page_no].word[translatedAddr.word_no], getInteger(reg[IP_REG]));
			storeInteger(reg[IP_REG], opnd1);
			YY_FLUSH_BUFFER;
		break;
		case RET:		//note: Modified here
			if(mode == KERNEL_MODE){
			  exception("Cannot call RET in KERNEL mode");
			}
			if(getInteger(reg[SP_REG]) < STACK_START_ADDRESS){ 
				exception("Stack Underflow");
			}
			if(getInteger(reg[SP_REG])  >= STACK_START_ADDRESS + PAGE_SIZE){
				exception("Stack Overflow");
			}
			translatedAddr = translate(getInteger(reg[SP_REG]));
			storeInteger(reg[IP_REG], getInteger(page[translatedAddr.page_no].word[translatedAddr.word_no]));
			storeInteger(reg[SP_REG], getInteger(reg[SP_REG]) - 1);
			YY_FLUSH_BUFFER;
		break;
		case INT:				//error: pid is used to return back and not kernel page table
			if(mode == KERNEL_MODE){
			  exception("Cannot call INT in KERNEL mode");
			}
			opnd1 = yylex();
			if(yylval.flag != NUM) {
				exception("Illegal operand");
			}
			if(opnd1 < 1 || opnd1 > 7) {				//error: might need to modify this
				exception("Illegal INT instruction\n");
			}
			if(getInteger(reg[SP_REG]) + 1 >= STACK_START_ADDRESS + PAGE_SIZE){
				exception("Stack Overflow");
			}
			if(getInteger(reg[SP_REG]) + 1 < STACK_START_ADDRESS ){
				exception("Stack Underflow");
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
				exception("Illegal Instruction");
			}
			if(getInteger(reg[SP_REG]) < STACK_START_ADDRESS) {			//note:for sfety check for overflow
				exception("Stack Underflow\n");
			}
			if(getInteger(reg[SP_REG]) >= STACK_START_ADDRESS + PAGE_SIZE) {			//note:for sfety check for overflow
				exception("Stack Overflow\n");
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
				exception("Illegal operand");
			}
			int input;
			scanf("%d",&input);
			storeInteger(reg[opnd1], input);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
		break;
		
		case OUT:		//note: is there any need to print '\n' since string is present
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand");
			}
			printf("%d\n",getInteger(reg[opnd1]));
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
		break;
		
		case SIN:			//note:modified here
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
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG])+1);
		break;
		case SOUT:				//note: modified and do we need '\n'
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand");
			}
			opnd1Value = getInteger(reg[opnd1]);
			translatedAddr = translate(opnd1Value);
			printf("%s\n",page[translatedAddr.page_no].word[translatedAddr.word_no]);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
		break;
		case STRCMP:				//note:modified here
			opnd1 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand");
			}
			opnd2 = yylex();
			if(yylval.flag != REG) {
				exception("Illegal operand");
			}
			opnd1Value = getInteger(reg[opnd1]);
			opnd2Value = getInteger(reg[opnd2]);
			translatedAddr1 = translate(opnd1Value);
			translatedAddr2 = translate(opnd2Value);
// 			printf("---%s---\n---%s---\n",page[translatedAddr1.page_no].word[translatedAddr1.word_no],page[translatedAddr2.page_no].word[translatedAddr2.word_no]);
			storeInteger(reg[opnd1],strcmp(page[translatedAddr1.page_no].word[translatedAddr1.word_no],page[translatedAddr2.page_no].word[translatedAddr2.word_no]));
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
		break;
		
		case STRCPY:				//note:modified here
			opnd1 = yylex();
			if(yylval.flag != REG && yylval.flag != SP) {
				exception("Illegal operand");
			}
			opnd2 = yylex();
			if(yylval.flag != REG && yylval.flag != SP) {
				exception("Illegal operand");
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
			storeInteger(reg[IP_REG], getInteger(reg[IP_REG])+1);
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
					exception("Illegal operand");
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
					exception("Illegal Operand");
					break;
			}
			
			
			
			
// 			printf("ReadFromDisk: page = %d\n diskBlock = %d\n", result, result2);
			readFromDisk(result, result2);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
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
					exception("Illegal operand1");
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
					exception("Illegal Operand2");
					break;
			}
			
			
			
			writeToDisk(result2, result);
			//printf("%d\n", result);
			//printf("%d\n", result2);
			storeInteger(reg[IP_REG],getInteger(reg[IP_REG])+1);
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
			exception("Illegal instruction\n");
	}
}
