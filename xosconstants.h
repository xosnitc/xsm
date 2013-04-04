
// Starting addresses of OS components
#define SCRATCHPAD 	512
#define PAGE_TABLE	1024
#define MEM_LIST	1280
#define FILE_TABLE	1344
#define READY_LIST	1536
#define FAT		2560
#define DISK_LIST	3072
#define EX_HANDLER	3584
#define T_INTERRUPT	4608
#define INTERRUPT	5632
#define USER_PROG	12800

// ***************************


#define NUM_PAGE_TABLE		4		// Number of entries in a per-process page table
#define NUM_FILE_TABLE		64		// Number of entries in system-wide open file table
#define NUM_PERFILE_TABLE 	8		// Number of entries in per-process open file table
#define NUM_PCB		 	32			// Number of PCBs / process / page tables
#define NUM_FAT	 		64			// Number of entries in FAT

#define PAGE_TABLE_ENTRY 	2		// Words per page table entry
#define FILE_TABLE_ENTRY 	2		// Words per system-wide open file table entry
#define PERFILE_TABLE_ENTRY 	2	// Words per per-process open file table entry
#define PCB_ENTRY	 	32			// Words per PCB
#define FAT_ENTRY 		8			// Words per FAT entry

#define NUM_BLOCKS		512			// Number of Disk Blocks


//*****************************

#define STATE_TERMINATED	0
#define STATE_READY		1
#define STATE_RUNNING		2
