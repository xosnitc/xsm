DISK = disk

all:	  xsm

lex.yy.c:   data.h instr.h decode.lex
	flex -i decode.lex

xsm:	simulator.c lex.yy.c  disk.h disk.c timer.h boot.h boot.c  scheduler.h scheduler.c utility.h utility.c 
	gcc -g -o xsm simulator.c disk.c boot.c  scheduler.c utility.c 
clean:
	find . -type f -name 'lex.yy.c' | xargs -r rm
	find . -type f -name 'xsm' | xargs -r rm
	find . -type f -name 'createdisk' | xargs -r rm
	rm -rf $(DISK) *.o createDisk fileSystem 2> /dev/null


createdisk:  createdisk.c disk.h disk.c utility.h utility.c 
	gcc createdisk.c disk.c utility.c  -o createdisk 

makedisk: createdisk
	./createdisk

formatdisk: createdisk
	find . -type f -name 'disk' | xargs -r rm
	./createdisk
