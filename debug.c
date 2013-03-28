#include "debug.h"

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
	
	
	if(strcmp(name,"help")==0)		//"help" to display all commands
	{
		/*printf("\n fdisk \n\t Format the disk with XFS filesystem\n\n");	
		printf(" load --exec  <pathname>  \n\t Loads an executable file to XFS disk \n\n");
		printf(" load --init  <pathname> \n\t Loads INIT code to XFS disk \n\n");
		printf(" load --data <pathname> \n\t Loads a data file to XFS disk \n\n");
		printf(" load --os  <pathname> \n\t Loads OS startup code to XFS disk \n\n");
		printf(" load --int=timer <pathname>\n\t Loads Timer Interrupt routine to XFS disk \n\n");		
		printf(" load --int=[1-7] <pathname>\n\t Loads the specified Interrupt routine to XFS disk \n\n");
		printf(" load --exhandler <pathname>  \n\t Loads exception handler routine to XFS disk \n\n");
		printf(" rm --exec <xfs_filename>\n\t Removes an executable file from XFS disk \n\n");
		printf(" rm --init <xfs_filename> \n\t Removes INIT code from XFS disk \n\n");	
		printf(" rm --data <xfs_filename>\n\t Removes a data file from XFS disk \n\n");
		printf(" rm --os \n\t Removes OS startup code from XFS disk \n\n");
		printf(" rm --int=timer \n\t Removes the Timer Interrupt routine from XFS disk \n\n");	
		printf(" rm --int=[1-7] \n\t Removes the specified Interrupt routine from XFS disk \n\n");
		printf(" rm --exhandler\n\t Removes the exception handler routine from XFS disk \n\n");
		printf(" ls \n\t List all files\n\n");	
		printf(" df \n\t Display free list and free space\n\n");
		printf(" cat <xfs_filename> \n\t to display contents of a file\n\n");
		printf(" copy <start_blocks> <end_block> <unix_filename>\n\t Copies contents of specified range of blocks to a UNIX file.\n\n");	
		printf(" exit \n\t Exit the interface\n");*/
		printf("help\n");
	}
	
	
	else if (strcmp(name,"step") == 0 || strcmp(name,"s") == 0)	//Single Stepping
	{
		step_flag = ENABLE;
		return 1;		
	}
	else if (strcmp(name,"continue") == 0 || strcmp(name,"c") == 0)	//Coontinue till next breakpoint
	{
		step_flag = DISABLE;
		return 1;		
	}	
	else if (strcmp(name,"load")==0) 	//loads files to XFS disk.
	{
		arg1 = strtok(NULL, " ");
		arg2 = strtok(NULL, " ");	

		/*char *int_command = strtok(arg1, "=");	
		char *intType = strtok(NULL, "=");
	    	char *fileName = arg2;;
	    
		if(fileName!=NULL)
			fileName[50] = '\0';
		else
		{
			printf("Missing <pathname> for load. See \"help\" for more information");
			return;
		}				
		if (strcmp(arg1,"--exec")==0)		
			
		else if (strcmp(arg1,"--init")==0)	
			
		else if (strcmp(arg1,"--data")==0) 
		{
		}
		else if (strcmp(arg1,"--os")==0)
		
		else if (strcmp(arg1,"--int")==0)
		{
		}
		else if (strcmp(arg1,"--exhandler")==0) 
		
		else
			printf("Invalid argument \"%s\" for load. See \"help\" for more information",arg1);*/
	}	
	else if (strcmp(name,"copy")==0)		//Copies blocks from Disk to UNIX file.
	{
		arg1 = strtok(NULL, " ");
		arg2 = strtok(NULL, " ");
		arg3 = strtok(NULL, " ");
		if(arg1==NULL || arg2==NULL|| arg3==NULL)
		{
			printf("Insufficient arguments for \"copy\". See \"help\" for more information");
			return;
		}	
		else
		{
			int startBlock = atoi(arg1);
			int endBlock = atoi(arg2);	
			char *fileName = arg3;			
			fileName[50] = '\0';
			//copyBlocksToFile (startBlock,endBlock,fileName);
		}	
	}						
	else if (strcmp(name,"exit")==0)		//Exits the interface
		exit(0);
	else
		printf("Unknown command \"%s\". See \"help\" for more information",name);
	return 0;
}

void printRegisters() {
	int i=0;
	for(i=0;i<NUM_REGS;i++) {
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
	}
	printf("\n");
}
