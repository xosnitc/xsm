#include "createDisk.h"


/*
  createDisk creates  the disk file if not present.
  if format is equal to zero then the function creates the disk but does not format it.
  if format is not equal to zero then the function will create and format the disk.
  Formatting is done as follows:
    1. A memory copy of the disk is maintained. This copy contains NO_BLOCKS_TO_COPY + EXTRA_BLOCKS (in this case 13 + 1) blocks.
      The extra block is a temporary block. This memory copy is called the virtual disk. This is first cleared.
    2. Then the memory freelist is initialised.
    3. The fat blocks are also initialised. The basic block entries are all set to -1. The memory copy is then committed to the 
      disk file.
    4. Finally the entry for init process is made.
*/
void createDisk(int format){
    int fd;
      if(format){
	 fd = open(DISK_NAME, O_CREAT | O_TRUNC | O_SYNC, 0666);
	 clearVirtDisk();
	 close(fd);
	// loadFileToVirtualDisk();		note: commented this line
	 int i=0,j=0;
	 for(j=0; j<NO_OF_FREE_LIST_BLOCKS; j++){
		if(j == 0)
		  for(i=0;i<DATA_START_BLOCK + INIT_SIZE ;i++)
		      storeInteger(disk[FREE_LIST_START_BLOCK].word[i], 1);
		else
			i=0;
		
		for( ;i<BLOCK_SIZE;i++)
			storeInteger(disk[FREE_LIST_START_BLOCK + j].word[i], 0);
		writeToDisk(FREE_LIST_START_BLOCK + j, FREE_LIST_START_BLOCK+j);
	    }
	    
	    
	for(j=0; j<NO_OF_FAT_BLOCKS; j++){
		for(i=FAT_BASICBLOCK; i<BLOCK_SIZE; i=i+FAT_ENTRY_SIZE){
			storeInteger(disk[FAT_START_BLOCK + j].word[i], -1);
		}
		writeToDisk(FAT_START_BLOCK+j, FAT_START_BLOCK+j);
	}
	
	initializeINIT();
      }
      else
      {
	fd = open(DISK_NAME, O_CREAT, 0666);
	close(fd);
      }
	
}


