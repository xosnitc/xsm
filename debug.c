#include "debug.h"

void printRegisters() {
	int i=0;
	for(i=0;i<NUM_REGS;i++) {
		switch(i) {
			case BP_REG: 
				printf("BP: %s\t",reg[BP_REG]);
				break;
			case SP_REG: 
				printf("SP: %s\t",reg[SP_REG]);
				break;
			case IP_REG: 
				printf("IP: %s\t",reg[IP_REG]);
				break;
			case PTBR_REG: 
				printf("PTBR: %s\t",reg[PTBR_REG]);
				break;
			case PTLR_REG: 
				printf("PTLR: %s\t",reg[PTLR_REG]);
				break;
			case EFR_REG: 
				printf("EFR: %s\t",reg[EFR_REG]);
				break;		
			default: 
				if(i<S0)
					printf("R%d: %s\t",i,reg[i]);
				else if(i<T0)
					printf("S%d: %s\t",i-S0,reg[i]);
				else
					printf("T%d: %s\t",i-T0,reg[i]);
				break;
		}
	}
	printf("\n");
}
