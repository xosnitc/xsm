#ifndef DISK_H
#define DISK_H
#include "FileSystemCode/disk.h"


/*
  This function reads an entire page from address specified by blockNumber to pageNumber.
*/
int readFromDisk(int pageNumber, int blockNumber);


/*
 This function writes an entire page to blocknumber from pageNumber.
*/
int writeToDisk(int pageNumber, int blockNumber);


#endif