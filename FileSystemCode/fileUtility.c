#include "fileUtility.h"


/*
 This function empties a block as specified by the first arguement in the memory copy of the disk file.
*/
void emptyBlock(int blockNo) {
	int i;
	for(i = 0 ; i < BLOCK_SIZE ; i++)
	{
		strcpy(disk[blockNo].word[i],"") ;
	}
}

/*
  char* to int conversion
*/
int getInteger(char* str ) {
	return *((int*)str);
}

/*
  int to char* conversion
*/
void storeInteger(char *str, int num) {
	char *c = (char*)&num;
	str[0] = *c;
	str[1] = *(c + 1);
	str[2] = *(c + 2);
	str[3] = *(c + 3);
}


/*
 This function reads an entire BLOCK from the address specified from fileBlockNumber on the disk file to virtBlockNumber
 on the memory copy of the disk.
*/
int readFromDisk(int virtBlockNumber, int fileBlockNumber) {
	int fd;
	fd = open(DISK_NAME, O_RDONLY, 0666);
	if(fd < 0){
	  printf("Unable to Open Disk File\n");
	  return -1;
	}
	lseek(fd,sizeof (BLOCK)*fileBlockNumber,SEEK_SET);
	read(fd,&disk[virtBlockNumber],sizeof (BLOCK));
	close(fd);
	return 0;
}


/*
  This function writes an entire block to fileBlocknumber on the disk file from virtBlockNumber on the memory copy
  of the disk.
*/
int writeToDisk(int virtBlockNumber, int fileBlockNumber) {
	int fd;
	fd = open(DISK_NAME, O_WRONLY, 0666);
	if(fd < 0){
	  printf("Unable to Open Disk File\n");
	  return -1;
	}
	lseek(fd,0,SEEK_SET);
	lseek(fd,sizeof (BLOCK)*fileBlockNumber,SEEK_CUR);
	write(fd,&disk[virtBlockNumber],sizeof (BLOCK));
	close(fd);	
	return 0;
}


/*
  This function initialises the memory copy of the disk with the contents from the actual disk file.
*/
int loadFileToVirtualDisk(){
  int i;
  for(i=FREE_LIST_START_BLOCK; i<FREE_LIST_START_BLOCK + NO_OF_FREE_LIST_BLOCKS; i++)
      readFromDisk(i,i);
  for(i=FAT_START_BLOCK; i<FAT_START_BLOCK + NO_OF_FAT_BLOCKS; i++)
      readFromDisk(i,i);
}

/*
  This function wipes out the entire contents of the memory copy of the disk.
*/
void clearVirtDisk(){
  bzero(disk, sizeof(disk));
}