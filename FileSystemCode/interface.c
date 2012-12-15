#include "interface.h"

void menu()
{
  printf("\n\nChoose your option:\n");
  printf("\t1. List All Files\n");
  printf("\t2. Load Executable File\n");
  printf("\t3. Remove Executable File\n");
  printf("\t4. Load OS Start up Code\n");
  printf("\t5. Load Interrupts\n");
  printf("\t6. Format/Create disk\n");
  printf("\t7. Load INIT Code\n");
  printf("\t8. Exit\n");
  printf("\t9. Load everything :)\n");
/* This option will work if the directory containing the executable has the interrupts with the 
    same name as in the switch case in the main function.*/
  printf("Option No:\n");
}




int main(){
  int  intNo;
  char fileName[WORD_SIZE], option;
  FILE* diskFp;
  createDisk(DO_NOT_FORMAT); //in case the disk file is not present
  loadFileToVirtualDisk();
  while(1){
    menu();
    scanf("%c", &option);
    switch(option){
      case '1':
	listAllFiles();
	break;
      case '2':
	printf("\nEnter Filename\n");
	scanf("%s", fileName);
	fileName[WORD_SIZE - 1] = '\0';
	loadExecutableToDisk(fileName);
	break;
      case '3':
	printf("\nEnter Filename\n");
	scanf("%s", fileName);
	fileName[WORD_SIZE - 1] = '\0';
	deleteExecutableFromDisk(fileName);
	break;
      case '4':
	printf("\nEnter Filename\n");
	scanf("%s", fileName);
	fileName[WORD_SIZE - 1] = '\0';
	loadOSCode(fileName);
	break;
      case '5':
	printf("\nEnter interrupt no:\n");
	scanf("%d", &intNo);
	printf("Enter filename\n");
	scanf("%s", fileName);
	loadIntCode(fileName, intNo);
	break;
      case '6':
	printf("Formatting...............\n");
	createDisk(FORMAT);
	break;
      case '7':
	printf("\nEnter Filename\n");
	scanf("%s", fileName);
	fileName[WORD_SIZE - 1] = '\0';
	loadINITCode(fileName);
	break;
      case '8':
	printf("Exiting...........\n");
	exit(0);
	break;
      case '9':
	loadOSCode("startup.esim");
	loadIntCode("int0.esim", 0);
	loadIntCode("int1.esim", 1);
	loadIntCode("int2.esim", 2);
	loadIntCode("int3.esim", 3);
	loadIntCode("int4.esim", 4);
	loadIntCode("int5.esim", 5);
	loadIntCode("int6.esim", 6);
	loadIntCode("int7.esim", 7);
	loadINITCode("init.esim");
	break;
      default:
	printf("Invalid option\n");
	break;
    }
    scanf("%c", &option);
    while(option != '\n')
      scanf("%c", &option);
  }
}