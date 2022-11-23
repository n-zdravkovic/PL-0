/*
	Nick Zdravkovic
	COP3402 - Spring 2022
	HW1 - VM
*/

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
	// declaration of registers
	int BP = 0;
	int SP = BP - 1;
	int PC = 0;
	int RP = MAX_REG_LENGTH;
	int halt = 1;
	int line = 0;

	// declaration of stacks
	int register_stack[MAX_REG_LENGTH] = {0};
	int data_stack[MAX_DATA_LENGTH] = {0};

	// holds the current instruction
	instruction IR;

	// prints out the initial values if the printFlag is not 0
	if (printFlag)
	{
		printf("\t\t\t\tPC\tBP\tSP\tRP\n");
		printf("Initial values:\t\t\t%d\t%d\t%d\t%d\n", PC, BP, SP, RP);
	}

	// loop through the program until the HAL (halt, 9 0 3) opcode is given
	while (halt)
	{	
		// gets the new instruction and increments PC (program counter) for the upcoming
		// instruction
		IR = code[PC];
		PC++;

		// switch statement based on the opcode
		switch (IR.opcode)
		{
			// literal; pushes the value of m to the register stack
			case 1:
				register_stack[RP - 1] = IR.m;
				RP--;
				if (printFlag)
				{
					print_execution(line, "LIT", IR, PC, BP, SP, RP, data_stack, register_stack);
				}
				line++;
			break;

			// switch statement based on the M value of opcode 2
			case 2:
				switch (IR.m)
				{
					// return; returns from a subprocedure
					case 0:
					SP = BP - 1;
						PC = data_stack[SP + 3];
						BP = data_stack[SP + 2];

						if (printFlag)
						{
							print_execution(line, "RET", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line = PC;
					break;

					// negate; negates the bottom register stack value
					case 1:
						register_stack[RP] = -register_stack[RP];
						if (printFlag)
						{
							print_execution(line, "NEG", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					/*
						arithmetic operations; higher position (operation) lower position
						the resultin value will be stored in the higher position 
						RP will increment
					*/
					case 2:
						register_stack[RP + 1] = register_stack[RP + 1] + register_stack[RP];
						RP++;
						if (printFlag)
						{
							print_execution(line, "ADD", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;
					case 3:
						register_stack[RP + 1] = register_stack[RP + 1] - register_stack[RP];
						RP++;
						if (printFlag)
						{
							print_execution(line, "SUB", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;
					case 4:
						register_stack[RP + 1] = register_stack[RP + 1] * register_stack[RP];
						RP++;
						if (printFlag)
						{
							print_execution(line, "MUL", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;
					case 5:
						register_stack[RP+1 ] = register_stack[RP + 1] / register_stack[RP];
						RP++;
						if (printFlag)
						{
							print_execution(line, "DIV", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					/* 
						comparisons; compares the value of the higher position to the value
						in the lower position; stores 1 if the comparion is true, 0 if
						the comparison is false; 
					*/

					// equal to
					case 6:
						if (register_stack[RP + 1] == register_stack[RP])
						{
							register_stack[RP + 1] = 1;							
						}
						else 
						{
							register_stack[RP + 1] = 0;	
						}
						RP++;
						if (printFlag)
						{
							print_execution(line, "EQL", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// not equal to
					case 7:
						if (register_stack[RP + 1] != register_stack[RP])
						{
							register_stack[RP + 1] = 1;							
						}
						else 
						{
							register_stack[RP + 1] = 0;	
						}
						RP++;
						if (printFlag)
						{
							print_execution(line, "NEQ", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// less than
					case 8:
						if (register_stack[RP + 1] < register_stack[RP])
						{
							register_stack[RP + 1] = 1;							
						}
						else 
						{
							register_stack[RP + 1] = 0;	
						}
						RP++;
						if (printFlag)
						{
							print_execution(line, "LSS", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// less than or equal to
					case 9:
						if (register_stack[RP + 1] <= register_stack[RP])
						{
							register_stack[RP + 1] = 1;							
						}
						else 
						{
							register_stack[RP + 1] = 0;	
						}
						RP++;
						if (printFlag)
						{
							print_execution(line, "LEQ", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// greater than
					case 10:
						if (register_stack[RP + 1] > register_stack[RP])
						{
							register_stack[RP + 1] = 1;							
						}
						else
						{
							register_stack[RP + 1] = 0;							
						}
						RP++;
						if (printFlag)
						{
							print_execution(line, "GTR", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// greater than or equal to
					case 11:
						if (register_stack[RP + 1] >= register_stack[RP])
						{
							register_stack[RP + 1] = 1;							
						}
						else 
						{
							register_stack[RP + 1] = 0;	
						}
						RP++;
						if (printFlag)
						{
							print_execution(line, "GEQ", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// and operator
					case 12:
						if (register_stack[RP + 1] == 1 && register_stack[RP] == 1)
						{
							register_stack[RP + 1] = 1;							
						}
						else 
						{
							register_stack[RP + 1] = 0;	
						}
						RP++;
						if (printFlag)
						{
							print_execution(line, "AND", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// or operator
					case 13:
						if (register_stack[RP + 1] == 1 || register_stack[RP] == 1)
						{
							register_stack[RP + 1] = 1;							
						}
						else 
						{
							register_stack[RP + 1] = 0;	
						}
						RP++;
						if (printFlag)
						{
							print_execution(line, "ORR", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// not operator
					case 14:
						if (register_stack[RP] == 0)
						{
							register_stack[RP] = 1;
						}
						else
						{
							register_stack[RP] = 0;
						}
						if (printFlag)
						{
							print_execution(line, "NOT", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;
				}
			break;

			// loads value to the bottom of the register stack from the data stack
			// value is found in the data stack using the base() function
			case 3:
				register_stack[RP - 1] = data_stack[base(IR.l, BP, data_stack) + IR.m];
				RP--;
				if (printFlag)
				{
					print_execution(line, "LOD", IR, PC, BP, SP, RP, data_stack, register_stack);
				}
				line++;
			break;

			// stores the value at the bottom of the register stack to the data stack
			// value is stored in the data stack using the base() function
			case 4:
				data_stack[base(IR.l, BP, data_stack) + IR.m] = register_stack[RP];
				RP++;
				if (printFlag)
				{
					print_execution(line, "STO", IR, PC, BP, SP, RP, data_stack, register_stack);
				}
				line++;
			break;

			// calls procedure at index M, and generates a new Activation Record
			case 5:
				data_stack[SP + 1] = base(IR.l, BP, data_stack);
				data_stack[SP + 2] = BP;
				data_stack[SP + 3] = PC;
				BP = SP + 1;
				PC = IR.m;
				
				if (printFlag)
				{
					print_execution(line, "CAL", IR, PC, BP, SP, RP, data_stack, register_stack);
				}
				line = PC;
			break;

			// allocates M amount of memory words 
			case 6:
				SP += IR.m;
				if (printFlag)
				{
					print_execution(line, "INC", IR, PC, BP, SP, RP, data_stack, register_stack);
				}
				line++;
			break;

			// jumps to instruction M
			case 7:
				PC = IR.m;
				if (printFlag)
				{
					print_execution(line, "JMP", IR, PC, BP, SP, RP, data_stack, register_stack);
				}
				line = PC;
			break;

			// jumps to instruction M is the bottom of the register stack is 0
			case 8:
				if (register_stack[RP] == 0)
				{
					PC = IR.m;
				}
				RP++;
				if (printFlag)
				{
					print_execution(line, "JPC", IR, PC, BP, SP, RP, data_stack, register_stack);
				}
				line = PC;
			break;

			// system operations; write, read, and halt
			case 9:
				switch (IR.m)
				{
					// writes the value at the bottom of the register stack 
					case 1:
						printf("Top of Stack Value: %d\n", register_stack[RP]);
						RP++;
						if (printFlag)
						{
							print_execution(line, "WRT", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// reads in input from the user and stores it at the bottom of the register stack
					case 2:
						printf("Please Enter an Integer: \n");
						scanf("%d", &register_stack[RP - 1]);
						RP--;
						if (printFlag)
						{
							print_execution(line, "RED", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;

					// sets the halt flag to 0 to end the program
					case 3:	
						halt = 0;
						if (printFlag)
						{
							print_execution(line, "HAL", IR, PC, BP, SP, RP, data_stack, register_stack);
						}
						line++;
					break;
				}
			break;
		}

	}
	
}