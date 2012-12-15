DISK = disk

all: fileSystem

fileSystem: fileSystem.h fileSystem.c fileUtility.h fileUtility.h interface.h interface.c createDisk.h createDisk.c
	gcc fileSystem.c fileUtility.c interface.c createDisk.c -o fileSystem
	cp -f fileSystem ../

clean:
	rm -rf $(DISK) *.o fileSystem 2> /dev/null
