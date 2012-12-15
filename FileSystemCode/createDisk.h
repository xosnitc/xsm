#ifndef CREATEDISK_H
#define CREATEDISK_H

#include <stdio.h>
#include "fileSystem.h"
#include <string.h>
#include "fileUtility.h"
#include <fcntl.h>


/*
  This function creates the disk file if not present. It also has an option for formatting or not.
*/
void createDisk(int format);

#endif