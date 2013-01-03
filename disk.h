#ifndef DISK_H
#define DISK_H
//#include "../xfs-interface/disk.h"
#define DISK_NAME 	"../xfs-interface/disk.xfs"
#define BOOT_BLOCK	0

/*
  This function reads an entire page from address specified by blockNumber to pageNumber.
*/
int readFromDisk(int pageNumber, int blockNumber);


/*
 This function writes an entire page to blocknumber from pageNumber.
*/
int writeToDisk(int pageNumber, int blockNumber);


#endif
