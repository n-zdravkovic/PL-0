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

    int BP = 0;
    int SP = BP - 1;
    int PC = 0;
    int RP = MAX_REG_LENGTH;
    
    int register_stack[MAX_REG_LENGTH];
    int data_stack[MAX_DATA_LENGTH];
    instruction IR;

    char* opname = malloc(4*sizeof(char));
    int halt = 0;
    int line;

    for (int i = 0; i < MAX_DATA_LENGTH; i++) {
        data_stack[i] = 0;
    }
    for (int i = 0; i < MAX_REG_LENGTH; i++) {
        register_stack[i] = 0;
    }

	if (printFlag)
	{
		printf("\t\t\t\tPC\tBP\tSP\tRP\n");
		printf("Initial values:\t\t\t%d\t%d\t%d\t%d\n", PC, BP, SP, RP);
	}

    while (!halt) {

        line = PC;
        IR = code[PC++];

        switch (IR.opcode) {
            case 1:
                strcpy(opname, "LIT");
                register_stack[--RP] = IR.m;
            break; case 2:

                if (IR.m == 0) {
                    strcpy(opname, "RET");
                    SP = BP-1;
                    PC = data_stack[SP+3];
                    BP = data_stack[SP+2];
                    break;
                }

                // Mathematic operations
                int temp;
                switch (IR.m) {
                    case 1:
                        strcpy(opname, "NEG");
                        temp = register_stack[RP] * -1;
                    break; case 2:
                        strcpy(opname, "ADD");
                        temp = register_stack[RP+1] + register_stack[RP++];
                    break; case 3:
                        strcpy(opname, "SUB");
                        temp = register_stack[RP+1] - register_stack[RP++];
                    break; case 4:
                        strcpy(opname, "MUL");
                        temp = register_stack[RP+1] * register_stack[RP++];
                    break; case 5: 
                        strcpy(opname, "DIV");
                        temp = register_stack[RP+1] / register_stack[RP++];
                    break; case 6:
                        strcpy(opname, "EQL");
                        temp = register_stack[RP+1] == register_stack[RP++];
                    break; case 7:
                        strcpy(opname, "NEQ");
                        temp = register_stack[RP+1] != register_stack[RP++];
                    break; case 8:
                        strcpy(opname, "LSS");
                        temp = register_stack[RP+1] < register_stack[RP++];
                    break; case 9:
                        strcpy(opname, "LEQ");
                        temp = register_stack[RP+1] <= register_stack[RP++];
                    break; case 10:
                        strcpy(opname, "GTR");
                        temp = register_stack[RP+1] > register_stack[RP++];
                    break; case 11:
                        strcpy(opname, "GEQ");
                        temp = register_stack[RP+1] >= register_stack[RP++];
                    break;
                }
                register_stack[RP] = temp;

            break; case 3:
                strcpy(opname, "LOD");
                register_stack[--RP] = data_stack[base(IR.l, BP, data_stack) + IR.m];

            break; case 4:
                strcpy(opname, "STO");
                data_stack[base(IR.l, BP, data_stack) + IR.m] = register_stack[RP++];

            break; case 5:
                strcpy(opname, "CAL");
                int SL = base(IR.l, BP, data_stack);
                int DL = BP;
                int RA = PC;

                data_stack[SP+1] = SL;
                data_stack[SP+2] = DL;
                data_stack[SP+3] = RA;

                BP = SP+1;
                PC = IR.m;

            break; case 6:
                strcpy(opname, "INC");
                SP += IR.m;

            break; case 7:
                strcpy(opname, "JMP");
                PC = IR.m;

            break; case 8:
                strcpy(opname, "JPC");
                if (register_stack[RP++] == 0) {
                    PC = IR.m;
                }

            break; case 9: 

                switch(IR.m) {
                    case 1:
                        strcpy(opname, "WRT");
                        printf("\t\tTop of Stack Value: %d\n", register_stack[RP++]);

                    break; case 2:
                        strcpy(opname, "RED");
                        printf("Please Enter an Integer:");
                        scanf("%d", &register_stack[--RP]);
                        printf("\n");

                    break; case 3:
                        strcpy(opname, "HAL");
                        halt = 1;

                    break;
                }

            break;
        }
        
        print_execution(line, opname, IR, PC, BP, SP, RP, data_stack, register_stack);
    }
}