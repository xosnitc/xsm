#include "fileSystem.h"
#include<stdio.h>
#include<stdlib.h>



/*
  This function lists all the files present on the disk.
  This is done as follows:
    1. The basic block entry in the memory copy of the disk is searched. If the value is not -1 then the filename is 
      shown as output.
*/
void listAllFiles(){
	int i,j;
	for(j = FAT_START_BLOCK ; j < FAT_START_BLOCK + NO_OF_FAT_BLOCKS ; j++){
		for(i = FAT_BASICBLOCK ; i < BLOCK_SIZE ; i = i + FAT_ENTRY_SIZE){
			if( getInteger(disk[j].word[i]) != -1 )
				printf("%s\n",disk[j].word[i-FAT_BASICBLOCK]);
		}
	}
}


/*
  This function checks if a file having name as the first arguement is present on the disk file.
  This is done as follows:
    1. It checks the entry in the fat block. If a file with same name exists then the function returns the relative word
      address of the entry having the same name.
*/
int CheckRepeatedName(char *name){
	int i,j;
	//name = strcat(name, "\n");
	for(j = FAT_START_BLOCK ; j < FAT_START_BLOCK + NO_OF_FAT_BLOCKS ; j++){
		for(i = FAT_FILENAME ; i < BLOCK_SIZE ; i = i + FAT_ENTRY_SIZE)	{
			if(strcmp(disk[j].word[i],name) == 0 && getInteger(disk[j].word[i]) != -1)		//note: modified here
				return (((j - FAT_START_BLOCK) * BLOCK_SIZE) + i);
		}
	}
	return (((j - FAT_START_BLOCK)* BLOCK_SIZE) + i);
}



/*
  This function returns the basic block entry(pass by pointer) corresponding to the address specified by the second arguement.
  NOTE: locationOfFat - relative word address of the name field in the fat.
	This function works only for EXE files.
*/
int getDataBlocks(int *basicBlockAddr, int locationOfFat){
	int i;
	basicBlockAddr[0] = getInteger(disk[FAT_START_BLOCK + locationOfFat / BLOCK_SIZE].word[locationOfFat % BLOCK_SIZE + FAT_BASICBLOCK]);
	readFromDisk(TEMP_BLOCK,basicBlockAddr[0]);                 //note:need to modify this
	for( i = 0 ; i < SIZE_EXEFILE ; i++){
		basicBlockAddr[i+1] = getInteger(disk[TEMP_BLOCK].word[i]);
	}
	return 0;
}




// int dispBasicBlock(int locationOfFat){
// 	int i;
// 	readFromDisk(TEMP_BLOCK,getInteger(disk[FAT_START_BLOCK + locationOfFat / BLOCK_SIZE].word[locationOfFat % BLOCK_SIZE + FAT_BASICBLOCK]));                 //note:need to modify this
// 	for( i = 0 ; i < SIZE_EXEFILE ; i++){
// 		printf("%d\n", getInteger(disk[TEMP_BLOCK].word[i]));
// 	}
// 	return 0;
// }



/*
  This function frees the blocks specified by the block number present in the first arguement. The second arguement is the size
  of the first argument.
  The memory copy is not committed.
*/
void FreeUnusedBlock(int *freeBlock, int size){
	int i=0;
	for( i = 0 ; i < size && freeBlock[i] != -1 ; i++){
		storeInteger( disk[FREE_LIST_START_BLOCK + freeBlock[i] / BLOCK_SIZE].word[freeBlock[i] % BLOCK_SIZE] , 0 );
	}
	
// 	for( i = 0 ; i < size ; i++){
// 		printf("Block Num = %d\n %d\n", freeBlock[i], sizeof(freeBlock)/sizeof(int));
// 		printf("%d\n", getInteger(disk[FREE_LIST_START_BLOCK + freeBlock[i] / BLOCK_SIZE].word[freeBlock[i] % BLOCK_SIZE]));
// 	}

}


/*
  This function removes the fat entry corresponding to the first arguement.
  NOTE: locationOfFat - relative word address of the name field in the fat.
  This is done as follows:
    1. The name field is set to empty string.
    2. The basic block entry is set to -1.
  The memory copy is not committed.
*/
int removeFatEntry(int locationOfFat){
	int i;
	int blockNumber = FAT_START_BLOCK + locationOfFat / BLOCK_SIZE;
	int startWordNumber = locationOfFat % BLOCK_SIZE;
	for( i = startWordNumber ; i < startWordNumber + FAT_ENTRY_SIZE ; i++ ){
		strcpy(disk[blockNumber].word[i],"");
	}
	storeInteger(disk[blockNumber].word[startWordNumber + FAT_BASICBLOCK], -1);
	return 0;
}



/*
  This function deletes an executable file from the disk.
  NOTE: 1. Memory copy is committed to disk.
	2. Due to a technical glitch any string which is already stored on the disk will have to be searched in the
	  memory copy after appending a newline.
  
*/
int deleteExecutableFromDisk(char *name){
	int locationOfFat,i,blockAddresses[SIZE_EXEFILE_BASIC];   //0-basic block , 1,2,3-code+data blocks
	name = strcat(name, "\n");
	if(strcmp(name, INIT_NAME) == 0){
	  printf("Init cannot be removed\n");
	  return 0;
	}
	locationOfFat = CheckRepeatedName(name);
	if(locationOfFat >= FAT_SIZE){
		printf("File not found\n");
		return -1;
	}
	getDataBlocks(blockAddresses,locationOfFat);		
	FreeUnusedBlock(blockAddresses, SIZE_EXEFILE_BASIC);
// 	writeToDisk(FREE_LIST_START_BLOCK, FREE_LIST_START_BLOCK);
// 	writeToDisk(FREE_LIST_START_BLOCK+1, FREE_LIST_START_BLOCK+1);
	removeFatEntry(locationOfFat);
	for(i = FAT_START_BLOCK ; i < FAT_START_BLOCK + NO_OF_FAT_BLOCKS ; i++){
		writeToDisk(i,i);
	}
	for( i=FREE_LIST_START_BLOCK ; i<FREE_LIST_START_BLOCK + NO_OF_FREE_LIST_BLOCKS; i++)
		writeToDisk(i,i);
		return 0;	
}



/*
  This function returns the address of a free block on the disk.
  The value returned will be the relative word address of the corresponding entry in the free list.
*/
int FindFreeBlock(){
	int i,j;
	for(i = FREE_LIST_START_BLOCK ; i < FREE_LIST_START_BLOCK + NO_OF_FREE_LIST_BLOCKS ;i++){
		for(j = 0 ; j < BLOCK_SIZE; j++){
			if( getInteger(disk[i].word[j]) == 0 ){
				storeInteger( disk[i].word[j] , 1 );	
				return ((i-FREE_LIST_START_BLOCK)*BLOCK_SIZE + j);
			}
		}
	}
	return -1;	
}




/*
  This function returns an  empty fat entry if present.
  NOTE: The return address will be the relative word address corresponding to the filename entry in the basic block.
*/
int FindEmptyFatEntry(){
	int i,j,entryFound = 0,entryNumber = 0;
	for(j = FAT_START_BLOCK ; j < FAT_START_BLOCK + NO_OF_FAT_BLOCKS ; j++){
		for(i = FAT_BASICBLOCK; i < BLOCK_SIZE ; i = i + FAT_ENTRY_SIZE){
			if( getInteger(disk[j].word[i]) == -1  ){
				entryNumber = (((j - FAT_START_BLOCK) * BLOCK_SIZE) + i);
				entryFound = 1;
				break;
			}
		}
		if(entryFound == 1)
			break;
	}
	if( entryNumber > FAT_SIZE ){
		printf("FAT  is full.\n");
		// note:FreeUnusedBlock(freeBlock);
		return -1;
	}
	return (entryNumber-FAT_BASICBLOCK);
}



/*
  This function adds the name, size and basic block address of the file to corresponding entry in the fat.
  The first arguement is a relative address
*/
void AddEntryToMemFat(int startIndexInFat, char *nameOfFile, int size_of_file, int addrOfBasicBlock){
	//char* str = strcat(nameOfFile,"\n");		//NOTE: Changed here
	strcpy(disk[FAT_START_BLOCK + (startIndexInFat / BLOCK_SIZE)].word[startIndexInFat % BLOCK_SIZE],nameOfFile);
	storeInteger( disk[FAT_START_BLOCK + (startIndexInFat / BLOCK_SIZE)].word[startIndexInFat % BLOCK_SIZE + FAT_FILESIZE] , size_of_file );
	storeInteger( disk[FAT_START_BLOCK + (startIndexInFat / BLOCK_SIZE)].word[startIndexInFat % BLOCK_SIZE + FAT_BASICBLOCK] , addrOfBasicBlock );
}



/*
  This file copies the necessary contents of a file to the corresponding location specified by the second arguemnt on the disk.
  The file is first copied to the memory copy of the disk. This is then committed to the actual disk file.
  NOTE: 1. EOF is set only after reading beyond the end of the file. This is the reason why the if condition is needed is needed.
	2. Also the function must read till EOF or BLOCK_SIZE line so that successive read proceeds accordingly
*/
int writeFileToDisk(FILE *f, int blockNum){
	int i, line=0;
	emptyBlock(TEMP_BLOCK);
	char c;
	for(i = 0; i < BLOCK_SIZE; i++){
		fgets(disk[TEMP_BLOCK].word[i],16,f);
		 /*if(!feof(f)){
//  		    printf("%d---%s--%d\n", blockNum, disk[TEMP_BLOCK].word[i], line++);   //note:For debugging
//  		    scanf("%c", &c);
		    if((strcmp(disk[TEMP_BLOCK].word[i],"OVER") == 0 || strcmp(disk[TEMP_BLOCK].word[i],"OVER\n") == 0)){
		      writeToDisk(TEMP_BLOCK,blockNum);
		      }
		 }
		 else*/				//note: modified here
		 if(feof(f)){
			 strcpy(disk[TEMP_BLOCK].word[i], "");
			writeToDisk(TEMP_BLOCK,blockNum);
			return -1;
		 }
	}
	writeToDisk(TEMP_BLOCK,blockNum);
	return 1;
}




/*
  This function loads the executable file corresponding to the first arguement to an appropriate location on the disk.
  This function systematically uses the above functions to do this action.
*/
int loadExecutableToDisk(char *name){
	FILE *fileToBeLoaded;
	int freeBlock[SIZE_EXEFILE_BASIC];
	int i,j;
	fileToBeLoaded = fopen(name, "r");
	if(fileToBeLoaded == NULL){
	    printf("File %s not found.\n", name);
	    return -1;
	  }
	name = strcat(name, "\n");  //NOTE:   modified here
	if(fileToBeLoaded == NULL){
		printf("The file could not be opened");
		return -1;
	}
	for(i = 0; i < SIZE_EXEFILE_BASIC ; i++){
		if((freeBlock[i] = FindFreeBlock()) == -1){
				printf("not sufficient space in disk to hold a new file.\n");
				FreeUnusedBlock(freeBlock, SIZE_EXEFILE_BASIC);
				return -1;
			}
	}
	i = CheckRepeatedName(name);
	if( i < FAT_SIZE ){
		printf("Disk already contains the file with this name. Try again with a different name.\n");
		FreeUnusedBlock(freeBlock, SIZE_EXEFILE_BASIC);
		return -1;
	}
	
	i = FindEmptyFatEntry();		
	if( i == -1 ){
		FreeUnusedBlock(freeBlock, SIZE_EXEFILE_BASIC);
		printf("No free FAT entry found.\n");
		return -1;			
	}
	AddEntryToMemFat(i, name, SIZE_EXEFILE * BLOCK_SIZE, freeBlock[0]);		
// 	printf("FAT %d\n", i);
// 	printf("basic %d\n", freeBlock[0]);
	for(i = FAT_START_BLOCK; i < FAT_START_BLOCK + NO_OF_FAT_BLOCKS ; i++){
		writeToDisk(i,i);				//updating disk fat entry note:check for correctness
	}
	for(i = FREE_LIST_START_BLOCK ;i < FREE_LIST_START_BLOCK + NO_OF_FREE_LIST_BLOCKS; i++)		//updating disk free list in disk
		writeToDisk(i, i);
	emptyBlock(TEMP_BLOCK);				//note:need to modify this
	
	for( i = 1 ; i < SIZE_EXEFILE_BASIC ; i++ ){
		storeInteger(disk[TEMP_BLOCK].word[i-1],freeBlock[i]); 
	}
	storeInteger(disk[TEMP_BLOCK].word[i-1],0);
	writeToDisk(TEMP_BLOCK,freeBlock[0]);
	j = writeFileToDisk(fileToBeLoaded, freeBlock[1]);		//writing executable file to disk
	if(j == 1)
	  j = writeFileToDisk(fileToBeLoaded, freeBlock[2]);		//if the file is longer than one page.  
	if(j == 1)
	  writeFileToDisk(fileToBeLoaded, freeBlock[3]);
      close(fileToBeLoaded);
      return 0;
}



/*
  This function loads the OS startup code specified by the first arguement to its appropriate location on disk.
  The code is first copied to memory copy. If this copying proceeds properly then the memory copy is committed to the disk.
*/
int loadOSCode(char* fileName){
  FILE* fp = fopen(fileName, "r");
  int instrCount;
  if(fp == NULL){
     printf("File %s not found.\n", fileName);
    return -1;
  }
  instrCount = 0;
  while(fgets(disk[OS_STARTUP_CODE].word[instrCount++], 16, fp)){
    if(instrCount > BLOCK_SIZE){
      printf("OS Code size exceeds one block\n");
      return -1;
    }
  }
  writeToDisk(OS_STARTUP_CODE,OS_STARTUP_CODE);
  close(fp);
  return 0;
}


/*
  This function copies the interrupts to the proper location on the disk.
*/
int loadIntCode(char* fileName, int intNo){
  FILE* fp = fopen(fileName, "r");
  int instrCount;
  if(fp == NULL){
     printf("File %s not found.\n", fileName);
    return -1;
  }
  instrCount = 0;
  while(fgets(disk[intNo + INT0].word[instrCount++], 16, fp)){
    if(instrCount > BLOCK_SIZE){
      printf("Interrupt Code size exceeds one block\n");
      return -1;
    }
  }
  writeToDisk(intNo+INT0 ,intNo+INT0);
  close(fp);
  return 0;
}


/*
  This function initialises the fat and basic block entries for the init process.
  The location of init process on the disk is fixed.
*/
int initializeINIT(){
  int locationOfFat, i;
  locationOfFat = CheckRepeatedName(INIT_NAME);
  if( locationOfFat >= FAT_SIZE ) {
    i = FindEmptyFatEntry();
    if(i == -1){
      printf("Disk error: IT would be proper to format the disk before proceeding\n");
      return -1;
    }
    AddEntryToMemFat(i, INIT_NAME, SIZE_EXEFILE * BLOCK_SIZE, INIT_BASIC_BLOCK);
    for(i = FAT_START_BLOCK; i < FAT_START_BLOCK + NO_OF_FAT_BLOCKS ; i++)
		writeToDisk(i,i);				//updating disk fat entry note:check for correctness
    emptyBlock(TEMP_BLOCK);				//note:need to modify this
	
    for( i = 1 ; i < SIZE_EXEFILE_BASIC ; i++ )						//updating basic block for file on disk
	storeInteger(disk[TEMP_BLOCK].word[i-1],INIT_BASIC_BLOCK + i); 
    writeToDisk(TEMP_BLOCK, INIT_BASIC_BLOCK);
  }
}


/*
  This function copies the init program to its proper location on the disk.
*/
int loadINITCode(char* fileName ){
  FILE * fp;
  int locationOfFat, i, j;
  fp = fopen(fileName, "r");
  if(fp == NULL){
    printf("File %s not found.\n", fileName);
    return -1;
  }
  
  j = writeFileToDisk(fp, INIT_BASIC_BLOCK + 1);		//writing executable file to disk
  if(j == 1)
    j = writeFileToDisk(fp, INIT_BASIC_BLOCK + 2);		//if the file is longer than one page.  
  if(j == 1)
    writeFileToDisk(fp, INIT_BASIC_BLOCK + 3);
  close(fp);
  return 0;
  
}