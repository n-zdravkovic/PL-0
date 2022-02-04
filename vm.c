#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#define MAX_REG_LENGTH 10
#define MAX_DATA_LENGTH 50
#define MAX_PROGRAM_LENGTH 150

void print_execution(int line, char *opname, instruction IR, int PC, int BP, int SP, int RP, int *data_stack, int *register_stack)
{
	int i;
	// print out instruction and registers
	printf("%2d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t\t", line, opname, IR.l, IR.m, PC, BP, SP, RP);
	
	// print register stack
	for (i = MAX_REG_LENGTH - 1; i >= RP; i--)
		printf("%d ", register_stack[i]);
	printf("\n");
	
	// print data stack
	printf("\tdata stack : ");
	for (i = 0; i <= SP; i++)
		printf("%d ", data_stack[i]);
	printf("\n");
}

int base(int L, int BP, int *data_stack)
{
	int ctr = L;
	int rtn = BP;
	while (ctr > 0)
	{
		rtn = data_stack[rtn];
		ctr--;
	}
	return rtn;
}

void execute_program(instruction *code, int printFlag)
{
	if (printFlag)
	{
		printf("\t\t\t\tPC\tBP\tSP\tRP\n");
		printf("Initial values:\t\t\t%d\t%d\t%d\t%d\n", PC, BP, SP, RP);
	}
	
}