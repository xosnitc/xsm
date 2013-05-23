#include "debug.h"

/*
 * This function initializes all debug flags and buffers
 */
void initialize_debug()
{
	db_mode = DISABLE;
	step_flag = DISABLE;
	bzero(command,COMMAND_LENGTH);
	bzero( prev_command, COMMAND_LENGTH);
	initialize_Watch();	
}

/* 
Function to invoke Command Line interface 
*/
void debug_interface()	
{
	char c;
	char next_instr[WORD_SIZE * WORDS_PERINSTR];
	int i,j,val;
	printf("Last Instruction Executed : %s\n", instruction);
	printf("Mode : %s \t Current IP Value: %s\n", (mode == USER_MODE)?"USER":"KERNEL" ,reg[IP_REG]);
	if(getInstruction(next_instr) == 0)		//gets the next instruction to be executed
		printf("Next Instruction to be Executed : %s\n", next_instr);
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
		if(command[0] == '\0')
			strcpy(command,prev_command);		
		if(command[0]!='\0')
		{
			strcpy(prev_command,command);	// backup this command
			val = runCommand(command);
			if(val == 1)
				return;	
		}
	}
}

/*
 * function processes each command the user enters
 * returns 1 if step or continue
 * returns 0 on success
 * returns -1 on error
 */
int runCommand(char command[])
{
	char *name = strtok(command, " ");
	char *arg1, *arg2, *arg3;
	int arg1value, arg2value;
	struct address translatedAddr;
	if(strcmp(name,"help")==0 || strcmp(name,"h")==0)		//"help" to display all commands
	{
		printf("\n step / s\n\t Single step the exection\n\n");	
		printf(" continue / c\n\t Continue to next breakpoint \n\n");
		printf(" reg / r \n\t Prints the value of all registers \n\n");
		printf(" reg / r <register_name>  \n\t Prints the value of a particular register \n\n");
		printf(" reg / r <register_name1> <register_name2>  \n\t Prints the value of all registers from <register_name1> to <register_name2> \n\n");
		printf(" mem / m <page_num>  \n\t Displays contents of a memory page \n\n");
		printf(" mem / m <page_num1> <page_num2>  \n\t Displays contents of memory pages from <page_num1> to <page_num2>\n\n");
		printf(" pcb / p \n \t Displays the PCB with state as running \n\n");
		printf(" pcb / p <pid> \n\t Displays the <pid> th PCB \n\n");
		printf(" pagetable / pt \n \t Displays the page table at location pointed by PTBR \n\n");
		printf(" pagetable / pt <pid> \n\t Displays the <pid> th page table \n\n");
		printf(" filetable / ft \n \t Displays the System Wide Open File Table\n\n");
		printf(" memfreelist / mf \n \t Displays the Memory Free List\n\n");
		printf(" diskfreelist / df \n \t Displays the Memory copy of Disk Free List\n\n");
		printf(" fat \n \t Displays the Memory Copy of File Allocation Table\n\n");
		printf(" location / l <address> \n \t Displays the content at memory address (Translation takes place in USER mode)\n\n");
		printf(" watch / w <physical address> \n \t Sets a watch point at this address\n\n");
		printf(" watchclear / wc \n \t Clears all the watch points\n\n");
		printf(" exit / e \n\t Exit the interface and Halt the machine\n");
		printf(" help / h\n");
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
			printRegisters(R0, NUM_REGS-1);
		else if(arg2 == NULL)
		{
			arg1value = getRegArg(arg1);
			if(arg1value == -1)
			{
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
				return -1;
			}
			else
				printRegisters(arg1value,arg1value);
		}
		else
		{
			arg1value = getRegArg(arg1);
			arg2value = getRegArg(arg2);
			if(arg1value == -1 || arg2value == -1)
			{
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
				return -1;
			}
			else
			{
				if(arg1value > arg2value) 	//swap them
				{
					arg1value = arg1value + arg2value;
					arg2value = arg1value - arg2value;
					arg1value = arg1value - arg2value;
				}
				printRegisters(arg1value,arg2value);
			}
		}
	}	
	else if (strcmp(name,"mem")==0 || strcmp(name,"m")==0)	//displays pages in memory
	{
		arg1 = strtok(NULL, " ");
		arg2 = strtok(NULL, " ");
		if(arg1 == NULL)
		{
			printf("Insufficient argument for \"%s\". See \"help\" for more information",name);
			return -1;
		}
		else if(arg2 == NULL)
		{
			arg1value = atoi(arg1);
			if(arg1value >0 && arg1value < NUM_PAGES)
				printMemory(arg1value);
			else
			{
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
				return -1;
			}
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
			{
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
				return -1;
			}
		}	
	}						
	else if (strcmp(name,"pcb")==0 || strcmp(name,"p")==0)	//displays PCB of a process
	{
		arg1 = strtok(NULL, " ");
		if(arg1 == NULL)  //finds the PCB with state as running
		{
			int page_no, word_no;
			arg1value = 0;
			while(arg1value < NUM_PCB)
			{
				page_no = (READY_LIST + arg1value * PCB_ENTRY + 1) / PAGE_SIZE;
				word_no = (READY_LIST + arg1value * PCB_ENTRY + 1) % PAGE_SIZE;
				if(getInteger(page[page_no].word[word_no]) == STATE_RUNNING)
					break;
				arg1value++;
			}
			if(arg1value == NUM_PCB)
			{
				printf("No PCB found with state as running");
				return -1;
			}
		}
		else
		{
			arg1value = atoi(arg1);
			if(arg1value<0 || arg1value >=NUM_PCB)
			{
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
				return -1;
			}
		}
		printPCB(arg1value);
	}
	else if (strcmp(name,"pagetable")==0 || strcmp(name,"pt")==0)	//displays Page Table of a process
	{
		arg1 = strtok(NULL, " ");
		if(arg1 == NULL)  //finds the page table using PTBR
		{
			int page_no, word_no;
			arg1value = getInteger(reg[PTBR_REG]);
			if(arg1value < PAGE_TABLE || arg1value > (PAGE_TABLE + ((NUM_PCB-1)*NUM_PAGE_TABLE*PAGE_TABLE_ENTRY)) )
			{
				printf("Illegal PTBR value");
				return -1;
			}
		}
		else
		{			
			if(atoi(arg1) < 0 || atoi(arg1) >= NUM_PCB )
			{
				printf("Illegal argument for \"%s\". See \"help\" for more information",name);
				return -1;
			}
			arg1value = PAGE_TABLE + atoi(arg1) * (PAGE_TABLE_ENTRY * NUM_PAGE_TABLE);
		}
		printPageTable(arg1value);
	}
	else if (strcmp(name,"filetable")==0 || strcmp(name,"ft")==0)	//displays System Wide Open File Table
		printFileTable();
	else if (strcmp(name,"memfreelist")==0 || strcmp(name,"mf")==0)	//displays Memory Free Lisk
		printMemFreeList();
	else if (strcmp(name,"diskfreelist")==0 || strcmp(name,"df")==0)	//displays Disk Free List
		printDiskFreeList();
	else if (strcmp(name,"fat")==0)	//displays File Allocation Table
		printFAT();
	else if (strcmp(name,"location")==0 || strcmp(name,"l")==0 )	//displays a content of a memory location
	{
		arg1 = strtok(NULL, " ");
		if(arg1 == NULL) 
		{
			printf("Insufficient argument for \"%s\". See \"help\" for more information",name);
			return -1;
		}
		translatedAddr = translate_debug(atoi(arg1));
		if(getType(arg1) == TYPE_STR || (translatedAddr.page_no == -1 && translatedAddr.word_no == -1) )
		{
			printf("Illegal argument for \"%s\". See \"help\" for more information",name);
			return -1;
		}
		printLocation(translatedAddr);
	}
	else if (strcmp(name,"watch")==0 || strcmp(name,"w")==0 )	//Sets watch point to a memory location
	{
		arg1 = strtok(NULL, " ");
		if(arg1 == NULL) 
		{
			printf("Insufficient argument for \"%s\". See \"help\" for more information",name);
			return -1;
		}
		if( getType(arg1) == TYPE_STR || atoi(arg1) < 0 || atoi(arg1) >= SIZE_OF_MEM )
		{
			printf("Illegal argument for \"%s\". See \"help\" for more information",name);
			return -1;
		}
		if( watch_count >= NUM_WATCH)
		{
			printf("You have already used %d watch points. No more watch points can be set.\nUse \"watchclear\" to clear all watch points. \n", NUM_WATCH );
			return -1;
		}
		watch[watch_count].addr.page_no = atoi(arg1) / PAGE_SIZE;
		watch[watch_count].addr.word_no = atoi(arg1) % PAGE_SIZE;
		strcpy(watch[watch_count].value, page[watch[watch_count].addr.page_no].word[watch[watch_count].addr.word_no]);
		watch_count++;
		printf("Watch point %d set.\n",watch_count);
	}
	else if (strcmp(name,"watchclear")==0 || strcmp(name,"wc")==0 )	//Clears all watch points
	{
		initialize_Watch();
		printf("All watch points cleared.\n");
	}
	else if (strcmp(name,"exit")==0 || strcmp(name,"e")==0)		//Exits the interface
		exit(0);
	else
	{
		printf("Unknown command \"%s\". See \"help\" for more information",name);
		return -1;
	}
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
void printRegisters(int arg1, int arg2)
{
	int i=1;
	while(arg1 <= arg2)
	{
		switch(arg1) 
		{
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
				if(arg1<S0)
					printf("R%d: %s\t",arg1,reg[arg1]);
				else if(arg1<T0)
					printf("S%d: %s\t",arg1-S0,reg[arg1]);
				else
					printf("T%d: %s\t",arg1-T0,reg[arg1]);
				break;
		}
		if(i % 4 == 0)
			printf("\n");
		arg1++;
		i++;
	}
	printf("\n");
}

/*
 * This fuction prints the memory page passed as argument.
 */
void printMemory(int page_no)
{
	int word_no;
	printf("Page No : %d",page_no);
	for(word_no = 0; word_no < PAGE_SIZE; word_no++)
	{
		if(word_no % 3 == 0)
			printf("\n");
		printf("%d: %s \t\t", word_no, page[page_no].word[word_no]);
	}
	printf("\n\n");
}

/*
 * This fuction prints the PCB of process with given process ID.
 */
void printPCB(int pid)
{
	int page_no, word_no, counter;
	page_no = (READY_LIST + pid * PCB_ENTRY) / PAGE_SIZE;
	word_no = (READY_LIST + pid * PCB_ENTRY) % PAGE_SIZE;
	printf("PID\t: %s\nSTATE\t: %s\n", page[page_no].word[word_no], page[page_no].word[word_no+1]);
	printf("BP\t: %s\n", page[page_no].word[word_no+2]);
	printf("SP\t: %s\n", page[page_no].word[word_no+3]);
	printf("IP\t: %s\n", page[page_no].word[word_no+4]);
	printf("PTBR\t: %s\n", page[page_no].word[word_no+5]);
	printf("PTLR\t: %s\n", page[page_no].word[word_no+6]);
	counter=0;
	while(counter < NO_USER_REG)
	{
		printf("R%d\t: %s\n", counter, page[page_no].word[word_no+7+counter]);
		counter++;
	}
	printf("Per-Process Open File Table\n");
	counter = 0;
	while(counter < NUM_PERFILE_TABLE)
	{
		printf("%d: %s\t%s\n", counter, page[page_no].word[word_no+15+ counter*PERFILE_TABLE_ENTRY], page[page_no].word[word_no+16+ counter*PERFILE_TABLE_ENTRY]);
		counter++;
	}
}

/*
 * This fuction prints the page table of process with given process ID.
 */
void printPageTable(int ptbr)
{
	int page_no, word_no, counter;
	page_no = ptbr / PAGE_SIZE;
	word_no = ptbr % PAGE_SIZE;
	printf("Page Table\n");
	counter = 0;
	while(counter < NUM_PAGE_TABLE)
	{
		printf("%d: %s\t%s\n", counter, page[page_no].word[word_no+ counter*PAGE_TABLE_ENTRY], page[page_no].word[word_no + counter*PAGE_TABLE_ENTRY +1]);
		counter++;
	}
}

/* 
 * This function prints the system wide open file table
 */
void printFileTable()
{
	int page_no, word_no, counter;
	page_no = FILE_TABLE / PAGE_SIZE;
	word_no = FILE_TABLE % PAGE_SIZE;
	printf("System Wide Open File Table\n");
	counter = 0;
	while(counter < NUM_FILE_TABLE)
	{
		printf("%d: %s\t%s\n", counter, page[page_no].word[word_no+ counter*FILE_TABLE_ENTRY], page[page_no].word[word_no + counter*FILE_TABLE_ENTRY +1]);
		counter++;
	}
}
 
 /* 
 * This function prints the memory free list
 */
void printMemFreeList()
{
	int page_no, word_no, counter;
	page_no = MEM_LIST / PAGE_SIZE;
	word_no = MEM_LIST % PAGE_SIZE;
	printf("Memory Free List");
	counter = 0;
	while(counter < NUM_PAGES)
	{
		if(counter % 4 == 0)
			printf("\n");
		printf("%d: %s \t\t", counter, (getInteger(page[page_no].word[word_no + counter])==0)?"FREE":"USED" );
		counter++;
	}
	printf("\n\n");
}

/* 
 * This function prints the disk free list
 */
void printDiskFreeList()
{
	int page_no, word_no, counter;
	page_no = DISK_LIST / PAGE_SIZE;
	word_no = DISK_LIST % PAGE_SIZE;
	printf("Disk Free List");
	counter = 0;
	while(counter < NUM_BLOCKS)
	{
		if(counter % 4 == 0)
			printf("\n");
		printf("%d: %s \t\t", counter, (getInteger(page[page_no].word[word_no + counter])==0)?"FREE":"USED" );
		counter++;
	}
	printf("\n\n");
}

/* 
 * This function prints the File Allocation table
 */
void printFAT()
{
	int page_no, word_no, counter;
	page_no = FAT / PAGE_SIZE;
	word_no = FAT % PAGE_SIZE;
	printf("File Allocation Table (FAT)\n");
	counter = 0;
	while(counter < NUM_FAT)
	{
		printf("%d: %s\t%s\t%s\n", counter, page[page_no].word[word_no+ counter*FAT_ENTRY], page[page_no].word[word_no + counter*FAT_ENTRY +1], page[page_no].word[word_no + counter*FAT_ENTRY +2]);
		counter++;
	}
}
 
/* 
 * This function translates an address without
 * invoking execution on errors.
 * returns page_no and word_no as -1 on failure
 */ 
struct address translate_debug (int virtual_addr)
{
	struct address resultant_addr;
	resultant_addr.page_no = -1;
	resultant_addr.word_no = -1;
	if(mode == USER_MODE)
	{		
		int page_entry;
		if(getType(reg[PTBR_REG]) == TYPE_STR || getType(reg[PTLR_REG]) == TYPE_STR || virtual_addr < 0
		 || virtual_addr >= getInteger(reg[PTLR_REG]) * PAGE_SIZE)
			return resultant_addr;							
		page_entry = getInteger(reg[PTBR_REG]) + (virtual_addr / PAGE_SIZE) * 2;
		if(page[(page_entry+1) / PAGE_SIZE].word[(page_entry+1) % PAGE_SIZE][1] == VALID )
		{ 
			resultant_addr.page_no = getInteger(page[page_entry / PAGE_SIZE].word[page_entry % PAGE_SIZE] );
			resultant_addr.word_no = virtual_addr % PAGE_SIZE;
			page[(page_entry+1) / PAGE_SIZE].word[(page_entry+1) % PAGE_SIZE][0] = REFERENCED;
		}
		return resultant_addr;
	}
	else
	{		
		if( virtual_addr < 0 || virtual_addr >= SIZE_OF_MEM )
			return resultant_addr;
		resultant_addr.page_no = virtual_addr / PAGE_SIZE;
		resultant_addr.word_no = virtual_addr % PAGE_SIZE;
		return resultant_addr;
	}
}
 
/* 
 * This function prints the memory location
 */
void printLocation(struct address translatedAddr)
{
	printf("%s\n", page[translatedAddr.page_no].word[translatedAddr.word_no]);
}

/*
 * This function check whether the value of watch location has changed
 * If changed returns ENABLE after printing old and new value of location
 * If unchanged returns DISABLE
 */
int checkWatch()
{
	int i;
	for(i = 0; i < watch_count; i++)
	{
		if(strcmp( page[watch[i].addr.page_no].word[watch[i].addr.word_no], watch[i].value) != 0)
		{
			printf("\nXSM Debug Environment\n");
			printf("The Value at Location %d (watch point %d) Changed\n", watch[i].addr.page_no * PAGE_SIZE + watch[i].addr.word_no, i + 1);
			printf("Previous value : %s\t\t" , watch[i].value);
			printf("New value : %s\n" , page[watch[i].addr.page_no].word[watch[i].addr.word_no]);
			strcpy(watch[i].value, page[watch[i].addr.page_no].word[watch[i].addr.word_no]);
			return ENABLE;
		}
	}
	return DISABLE;
}

/*
 * This function initialize a data structures for watch
 */
void initialize_Watch()
{
	int i;
	for(i=0 ; i < NUM_WATCH; i++)
	{
		bzero(watch[i].value, WORD_SIZE);
		watch[i].addr.page_no = 0;
		watch[i].addr.word_no = 0;
	}
	watch_count = 0;
}
