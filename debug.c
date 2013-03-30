#include "debug.h"
#include "data.h"
#include "memory_constants.h"
#include "interrupt.h"

/* 
Function to invoke Command Line interface 
*/
void debug_interface()	
{
  char command[100], c;
  int i,j;
		
  printf("Unix-XFS Interace Version 1.0. \nType \"help\" for  getting a list of commands.");
  while(1)
  {
  	i=0;
  	printf("\n# ");
  	scanf("%c",&c);
  	while(c!='\n')
  	{  	
		command[i++] = c;
		scanf("%c",&c);
  	}
  	command[i] = '\0';
	if(command[0]!='\0')
		if(runCommand(command) == 1)
			return;
  }
}

/* 
Function to process commands 
*/
int runCommand(char command[])
{
	char *name = strtok(command, " ");
	char *arg1, *arg2, *arg3;
	int arg1value, arg2value;	
	if(strcmp(name,"help")==0)		//"help" to display all commands
	{
		printf("\n step / s\n\t Single step the exection\n\n");	
		printf(" continue / c\n\t Continue to next breakpoint \n\n");
		printf(" reg / r \n\t Prints the value of all registers \n\n");
		printf(" reg / r <register_name>  \n\t Prints the value of a particular register \n\n");
		printf(" reg / r <register_name1> <register_name2>  \n\t Prints the value of all registers from <register_name1> to <register_name2> \n\n");
		printf(" mem / m <page_num>  \n\t Displays contents of a memory page \n\n");
		printf(" mem / m <page_num1> <page_num2>  \n\t Displays contents of memory pages from <page_num1> to <page_num2>\n\n");
		printf(" exit \n\t Exit the interface and Halt the machine\n");
		printf("help\n");
	}	
	else if (strcmp(name,"step") == 0 || strcmp(name,"s") == 0)	//Single Stepping
	{
		step_flag = ENABLE;
		return 1;		
	}
	else if (strcmp(name,"continue") == 0 || strcmp(name,"c") == 0)	//Continue till next breakpoint
	{
		step_flag = DISABLE;
		return 1;		
	}	
	else if (strcmp(name,"reg")==0 || strcmp(name,"r")==0) 	//Prints the registers.
	{
		arg1 = strtok(NULL, " ");
		arg2 = strtok(NULL, " ");	
		if(arg1 == NULL)
			printRegisters(-1);
		else if(arg2 == NULL)
		{
			arg1value = getRegArg(arg1);
			if(arg1value == -1)
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
			else
				printRegisters(arg1value);
		}
		else
		{
			arg1value = getRegArg(arg1);
			arg2value = getRegArg(arg2);
			if(arg1value == -1 || arg2value == -1)
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
			else
			{
				if(arg1value > arg2value) 	//swap them
				{
					arg1value = arg1value + arg2value;
					arg2value = arg1value - arg2value;
					arg1value = arg1value - arg2value;
				}
				while(arg1value <= arg2value)
				{
					printRegisters(arg1value);
					arg1value++;
				}
			}				
		}
	}	
	else if (strcmp(name,"mem")==0 || strcmp(name,"m")==0)	//displays pages in memory
	{
		arg1 = strtok(NULL, " ");
		arg2 = strtok(NULL, " ");
		if(arg1 == NULL)
			printf("Insufficient argument for \"%s\". See \"help\" for more information",name);
		else if(arg2 == NULL)
		{
			arg1value = atoi(arg1);
			if(arg1value >0 && arg1value < NUM_PAGES)
				printMemory(arg1value);
			else
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
		}
		else
		{
			arg1value = atoi(arg1);
			arg2value = atoi(arg2);
			if(arg1value > arg2value) 	//swap them
			{
				arg1value = arg1value + arg2value;
				arg2value = arg1value - arg2value;
				arg1value = arg1value - arg2value;
			}
			if(arg1value >0 && arg2value < NUM_PAGES)
			{
				while(arg1value <= arg2value)
				{
					printMemory(arg1value);
					arg1value++;
				}
			}
			else
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
		}	
	}						
	else if (strcmp(name,"exit")==0 || strcmp(name,"e")==0)		//Exits the interface
		exit(0);
	else
		printf("Unknown command \"%s\". See \"help\" for more information",name);
	return 0;
}

/*
 * Function to get register number from argument
 */
int getRegArg(char *arg)
{
	int argvalue;
	if(strcmp(arg,"BP") == 0 || strcmp(arg,"bp") == 0)
		return(BP_REG);
	else if(strcmp(arg,"SP") == 0 || strcmp(arg,"sp") == 0)
		return(SP_REG);
	else if(strcmp(arg,"IP") == 0 || strcmp(arg,"ip") == 0)
		return(IP_REG);
	else if(strcmp(arg,"PTBR") == 0 || strcmp(arg,"ptbr") == 0)
		return(PTBR_REG);
	else if(strcmp(arg,"PTLR") == 0 || strcmp(arg,"ptlr") == 0)
		return(PTLR_REG);
	else if(strcmp(arg,"EFR") == 0 || strcmp(arg,"efr") == 0)
		return(EFR_REG);
	else
		argvalue = atoi(arg + 1);
	switch(arg[0])
	{
		case 'R':
		case 'r':
			if(argvalue >= 0 && argvalue < NO_USER_REG);
				return(R0 + argvalue);
			break;
		case 'S':
		case 's':
			if(argvalue >= 0 && argvalue < NO_SYS_REG);
				return(S0 + argvalue);
			break;
		case 'T':
		case 't':
			if(argvalue >= 0 && argvalue < NO_TEMP_REG);
				return(T0 + argvalue);
			break;
	}
	return -1;
}

/* Prints all the registers if arg is -1, 
 * otherwise prints the register passed as argument
 */
void printRegisters(int flag)
{
	int i=0;
	if(flag != -1)
		i=flag;
	do
	{
		switch(i) {
			case BP_REG: 
				printf("BP: %s\t",reg[BP_REG]);
				break;
			case SP_REG: 
				printf("SP: %s\t",reg[SP_REG]);
				break;
			case IP_REG: 
				printf("IP: %s\t",reg[IP_REG]);
				break;
			case PTBR_REG: 
				printf("PTBR: %s\t",reg[PTBR_REG]);
				break;
			case PTLR_REG: 
				printf("PTLR: %s\t",reg[PTLR_REG]);
				break;
			case EFR_REG: 
				printf("EFR: %s\t",reg[EFR_REG]);
				break;		
			default: 
				if(i<S0)
					printf("R%d: %s\t",i,reg[i]);
				else if(i<T0)
					printf("S%d: %s\t",i-S0,reg[i]);
				else
					printf("T%d: %s\t",i-T0,reg[i]);
				break;
		}
		i++;
	}while(i<NUM_REGS && flag == -1);
	printf("\n");
}

/*
 * This fuction prints the memory page passed as argument.
 */
void printMemory(int page_no)
{
	int word_no;
	printf("Page No : %d\n",page_no);
	for(word_no = 0; word_no < PAGE_SIZE; word_no++)
		printf("%d: %s \t\t", word_no, page[page_no].word[word_no]);
	printf("\n\n");
}
