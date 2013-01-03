#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "disk.h"
#include "data.h"

// void print(PAGE str, int pageNumber)
// {
// 	int i;
// 	char c;
// 	printf("$$$$$$$$$$$$$$$$$$$$$$$$ %d $$$$$$$$$$$$$$$$$$$$$$$" , pageNumber);
// 	scanf("%c", &c);
// 	for(i=0; i<256; i++)
// 		printf("--readFromDisk--- %s ---------\n", page[pageNumber].word[i]);
// 	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");
// // 	scanf("%c", &c);
// }




/*
  This function reads an entire page from address specified by blockNumber to pageNumber.
*/
int readFromDisk(int pageNumber, int blockNumber) {
	int fd;
	fd = open(DISK_NAME, O_RDONLY, 0666);
	if(fd < 0)
	{
	  printf("Unable to Open Disk File\n");
	  return -1;
	}
	lseek(fd,sizeof (PAGE)*blockNumber,SEEK_SET);
	read(fd,&page[pageNumber],sizeof (PAGE));
// 	print(page[pageNumber], pageNumber);
	close(fd);
}



/*
 This function writes an entire page to blocknumber from pageNumber.
*/
int writeToDisk(int pageNumber, int blockNumber) {
	int fd;
	fd = open(DISK_NAME, O_WRONLY, 0777);
	if(fd < 0)
	{
	  printf("Unable to Open Disk File\n");
	  return -1;
	}
	lseek(fd,sizeof (PAGE)*blockNumber,SEEK_SET);
	write(fd,&page[pageNumber],sizeof (PAGE));
	close(fd);	
}
