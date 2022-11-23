/*
	Nick Zdravkovic
	COP3402 - Spring 2022
	HW3 - Parser
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

#define MAX_CODE_LENGTH 150
#define MAX_SYMBOL_COUNT 20
#define MAX_REG_HEIGHT 10

instruction *code;
int cIndex;
symbol *table;
int tIndex;
int tokenIndex = 0;
int level;
int symIdx;
int jpcIdx;
int jmpIdx;
int loopIdx;
int parseError = 0;
int regCount = 0;
int debug = 0;
lexeme token;

void emit(int opname, int level, int mvalue);
void addToSymbolTable(int k, char n[], int v, int l, int a, int m);
void mark();
int multipledeclarationcheck(char name[]);
int findsymbol(char name[], int kind);
void printparseerror(int err_code);
void printsymboltable();
void printassemblycode();
void getNextToken(lexeme *list);

void block(lexeme *list);
void constant(lexeme *list);
void procedure(lexeme *list);
void statement(lexeme *list);
void expression(lexeme *list);
void logic(lexeme *list);
void condition(lexeme *list);
void term(lexeme *list);
void factor(lexeme *list);
int variable(lexeme *list);


instruction *parse(lexeme *list, int printTable, int printCode)
{
	code = malloc(sizeof(instruction) * MAX_CODE_LENGTH);
	table = malloc(sizeof(symbol) * MAX_SYMBOL_COUNT);
	tIndex = 0;
	cIndex = 0;
	level = 0;
	
	// first instruction is always a jump
	emit(7,0,0);

	// gets the first token of the program
	token = list[tokenIndex];

	// add "main", to the symbol table
	addToSymbolTable(3, "main", 0, level, 0, 0);
	level = -1;

	// begins parsing
	block(list);

	// there was a parse error somewhere in the code, returns
	if (parseError)
	{
		return NULL;
	}
	
	// ensures last token of the program is a period
	// otherwise, prints appropriate error and returns NULL
	if (token.type != periodsym && parseError == 0)
	{
		printparseerror(1);
		return NULL;
	}

	// emit the HALT code
	emit(9, 0, 3);

	// gets the addresses of subprocedures
	for (int i = 0; i < cIndex; i++)
	{
		if (code[i].opcode == 5)
		{
			code[i].m = table[code[i].m].addr;
		}
	}
	code[0].m = table[0].addr;

	// prints the symbol table and assembly code if no errors
	// were encountered, table is not NULL, and the corresponding print flags
	// are nonzero
	if (parseError == 0 && table != NULL)
	{
		if (printTable)
			printsymboltable();
		if (printCode)
			printassemblycode();
	}
	
	code[cIndex].opcode = -1;

	// returns the code array
	return code;
}

void block(lexeme *list)
{
	int pIndex = tIndex - 1;
	int numVars = 0;

	if (parseError)
	{
		return;
	}

	level++;

	if (debug)
	{
		printf("Function: Block()");
	}

	// calls the three different functions to process the codes
	// constants, variables, and procedures
	constant(list);
	if (parseError)
	{
		return;
	}
	numVars = variable(list);
	if (parseError)
	{
		return;
	}
	procedure(list);
	if (parseError)
	{
		return;
	}

	// emits INC
	table[pIndex].addr = cIndex;
	emit(6, 0, numVars + 3);
	
	statement(list);
	if (parseError)
	{
		return;
	}
	mark();

	level--;
}

// the function to process constants
void constant(lexeme *list)
{
	char name[12];

	if (parseError)
	{
		return;
	}

	if (debug)
	{
		printf("Function: Constant()");
	}

	// the current token is a const
	if (token.type == constsym)
	{
		// executes at least once, and while a commasym separates multiple consts
		do {
			getNextToken(list);

			// a const has to have an identifier; returns an error if none is detected
			if (token.type != identsym)
			{
				parseError = 1;
				printparseerror(2);
				return;
			}

			// checks if there is another const with the same name
			// returns an error if there is
			symIdx = multipledeclarationcheck(token.name);
			if (symIdx != -1)
			{
				parseError = 1;
				printparseerror(18);
				return;
			}


			// now it checks to makes sure the const declaration
			// follows the grammar rules
			strcpy(name, token.name);
			getNextToken(list);

			if (token.type != assignsym)
			{
				parseError = 1;
				printparseerror(2);
				return;
			}

			getNextToken(list);
			if (token.type != numbersym)
			{
				parseError = 1;
				printparseerror(2);
				return;
			}

			addToSymbolTable(1, name, list[tokenIndex].value, level, 0, 0);
			getNextToken(list);

		} while (token.type == commasym);

		// the declaration should end with a semicolon
		if (token.type != semicolonsym)
		{
			if (token.type == identsym)
			{
				parseError = 1;
				printparseerror(13);
				return;
			}
			else
			{
				parseError = 1;
				printparseerror(14);
				return;
			}
		}
		getNextToken(list);
	}
}

// function to process procedures
void procedure(lexeme *list)
{
	char name[12];

	if (parseError)
	{
		return;
	}

	// debug print to track which function we are in
	if (debug)
	{
		printf("Function: Procedure()");
	}

	// executes only if the token is a procsym
	// as procedures are optional in PL/0
	while (token.type == procsym)
	{
		getNextToken(list);

		// a procedure has to have an identifier; returns an error if this
		// is not the case
		if (token.type != identsym)
		{
			parseError = 1;
			printparseerror(4);
			return;
		}

		// checks if there are multiple procedures with the same name
		// and on the same level
		symIdx = multipledeclarationcheck(list[tokenIndex].name);
		if (symIdx != -1)
		{
			parseError = 1;
			printparseerror(18);
			return;
		}
		
		// adds the procedure to the symbol table
		strcpy(name, list[tokenIndex].name);
		addToSymbolTable(3, name, 0, level, 0, 0);
		getNextToken(list);

		// ensures the procedure declaration ends with a semicolon
		if (token.type != semicolonsym)
		{
			parseError = 1;
			printparseerror(4);
			return;
		} 

		// calls block again since a procedure is a separate block of code
		getNextToken(list);	
		block(list);
		if (parseError)
		{
			return;
		}

		if (token.type != semicolonsym)
		{
			parseError = 1;
			printparseerror(14);
			return;
		} 
		
		getNextToken(list);

		// emits RET
		emit(2, 0, 0);
	} 
}

// function to process statements
void statement(lexeme *list)
{
	if (parseError)
	{
		return;
	}

	if (debug)
	{
		printf("Function: Statement()");
	}

	// we encounter an indentifier
	if (token.type == identsym)
	{
		// we can only assign to variables; const values cannot change
		// and procedures can only be called
		symIdx = findsymbol(list[tokenIndex].name, 2);
		if (symIdx == -1)
		{
			if (findsymbol(list[tokenIndex].name, 3) != -1 || findsymbol(list[tokenIndex].name, 3) != -1)
			{
				parseError = 1;
				printparseerror(6);
				return;
			}
			else
			{
				parseError = 1;
				printparseerror(19);
				return;
			}
		}
		getNextToken(list);
		
		// the assign symbol is necessary for assignment
		if (token.type != assignsym)
		{
			parseError = 1;
			printparseerror(5);
			return;
		}

		getNextToken(list);
		expression(list);
		if (parseError)
		{
			return;
		}
		
		// emits STO, decrements regCount
		emit(4, level - table[symIdx].level, table[symIdx].addr);
		regCount--;
		return;
	}

	// if the token is begin
	if (token.type == beginsym)
	{
		do {
			getNextToken(list);
			statement(list);
			if (parseError)
			{
				return;
			}
		} while (token.type == semicolonsym);

		// begin must be followed by an end
		// and statements in a begin block must be separated by a semicolon
		if (token.type != endsym)
		{
			if (token.type == identsym)
			{
				parseError = 1;
				printparseerror(15);
				return;
			}
			else if (token.type == ifsym)
			{
				parseError = 1;
				printparseerror(15);
				return;
			}
			else if (token.type == whilesym)
			{
				parseError = 1;
				printparseerror(15);
				return;
			}
			else if (token.type == readsym)
			{
				parseError = 1;
				printparseerror(15);
				return;
			}
			else if (token.type == writesym)
			{
				parseError = 1;
				printparseerror(15);
				return;
			}
			else if (token.type == callsym)
			{
				parseError = 1;
				printparseerror(15);
				return;
			}
			else
			{
				parseError = 1;
				printparseerror(16);
				return;
			}
		}
		
		getNextToken(list);
		return;
	}

	// if the token is an if
	else if (token.type == ifsym)
	{
		getNextToken(list);
		logic(list);
		if (parseError)
		{
			return;
		}
		jpcIdx = cIndex;

		// emit JPC
		emit(8, 0, 0);
		regCount--;
		
		// an if symbol must be followed by a then
		if (token.type != thensym)
		{
			parseError = 1;
			printparseerror(8);
			return;
		}

		getNextToken(list);
		statement(list);
		if (parseError)
		{
			return;
		}

		// an else is optional in PL/0
		if (token.type == elsesym)
		{
			jmpIdx = cIndex;

			// emit JMP
			emit(7, 0, 0);

			code[jpcIdx].m = cIndex;
			getNextToken(list);
			statement(list);
			code[jmpIdx].m = cIndex;
		}
		else
		{
			code[jpcIdx].m = cIndex;
		}
		return;
	}

	// if the token is a while
	if (token.type == whilesym)
	{
		getNextToken(list);
		loopIdx = cIndex;
		logic(list);
		jpcIdx = cIndex;

		// emits JPC
		emit(8, 0, 0);
		regCount--;

		// a while symbol must be followed by a do symbol
		if (token.type != dosym)
		{
			parseError = 1;
			printparseerror(9);
			return;
		}

		getNextToken(list);
		statement(list);
		if (parseError)
		{
			return;
		}

		// emits JMP
		emit(7, 0, loopIdx);
		code[jpcIdx].m = cIndex;
		
		return;
	}

	// if the token is read
	else if (token.type == readsym)
	{
		getNextToken(list);
		if (token.type != identsym)
		{
			parseError = 1;
			printparseerror(6);
			return;
		}

		// we can only read from variables
		symIdx = findsymbol(list[tokenIndex].name, 2);
		if (symIdx == -1)
		{
			if (findsymbol(list[tokenIndex].name, 3) != -1 || findsymbol(list[tokenIndex].name, 3) != -1)
			{
				parseError = 1;
				printparseerror(6);
				return;
			}
			else
			{
				parseError = 1;
				printparseerror(19);
				return;
			}
		}
		getNextToken(list);

		// emit RED
		emit(9, 0, 2);

		// since we are adding to the register, we also
		// have to be careful to not exceed it's max height
		regCount++;
		if (regCount > MAX_REG_HEIGHT)
		{
			parseError = 1;
			printparseerror(20);
			return;
		}

		// emits STO
		emit(4, level - table[symIdx].level, symIdx);
		regCount--;
		return;
	}

	// if the token is write
	else if (token.type == writesym)
	{
		getNextToken(list);
		expression(list);
		if (parseError)
		{
			return;
		}

		// emits WRT
		emit(9, 0, 1);
		return;
	}

	//if the token is call
	else if (token.type == callsym)
	{
		getNextToken(list);
		if (token.type != identsym)
		{
			parseError = 1;
			printparseerror(7);
			return;
		}

		// we can only call procedures; returns the corresponding error 
		// if we try to call a variable or const
		symIdx = findsymbol(list[tokenIndex].name, 3);
		if (symIdx == -1)
		{
			if (findsymbol(list[tokenIndex].name, 1) != -1 || findsymbol(list[tokenIndex].name, 2) != -1)
			{
				parseError = 1;
				printparseerror(7);
				return;
			}
			else
			{
				parseError = 1;
				printparseerror(19);
				return;
			}
		}
		getNextToken(list);

		// emits CAL
		emit(5, level - table[symIdx].level, symIdx);
	}
	return;
}

// builds an expressions
void expression(lexeme *list)
{
	int temp;

	if (parseError)
	{
		return;
	}

	// debug print
	if (debug)
	{
		printf("Function: Expression()");
	}

	if (token.type == minussym || token.type == plussym)
	{
		temp = token.type;
		getNextToken(list);
		term(list);
		if (parseError)
		{
			return;
		}

		// a minus symbol alone means "negate the number"
		if (temp == minussym)
		{
			// so we emit NEG
			emit(2, 0, 1);
		}
	}
	else
	{
		term(list);
		if (parseError)
		{
			return;
		}
	}	

	// emits the corresponding code to minus and plus symbols, whichever 
	// the current symbol is
	while (token.type == minussym || token.type == plussym)
	{
		temp = token.type;
		getNextToken(list);
		term(list);
		if (parseError)
		{
			return;
		}
		if (temp == plussym)
		{
			// emit ADD
			emit(2, 0, 2);
			regCount--;
		}
		else
		{
			// emit SUB
			emit(2, 0, 3);
			regCount--;
		}
	}

	// an expression cannot end on any of these
	// so we return error 17
	if (token.type == plussym)
	{
		parseError = 1;
		printparseerror(17);
		return;
	}
	if (token.type == minussym)
	{
		parseError = 1;
		printparseerror(17);
		return;
	}
	if (token.type == multsym)
	{
		parseError = 1;
		printparseerror(17);
		return;
	}
	if (token.type == divsym)
	{
		parseError = 1;
		printparseerror(17);
		return;
	}
	if (token.type == lparensym)
	{
		parseError = 1;
		printparseerror(17);
		return;
	}
	if (token.type == identsym)
	{
		parseError = 1;
		printparseerror(17);
		return;
	}
	if (token.type == numbersym)
	{
		parseError = 1;
		printparseerror(17);
		return;
	}
}

void logic(lexeme *list)
{
	if (parseError)
	{
		return;
	}

	if (token.type == notsym)
	{
		getNextToken(list);
		condition(list);
		emit(2, 0, 14);
		regCount--;
	}
	else
	{
		condition(list);
		while (token.type == andsym || token.type == orsym)
		{
			if (token.type == andsym)
			{
				getNextToken(list);
				condition(list);
				emit(2, 0, 12);
				regCount--;
			}
			else
			{
				getNextToken(list);
				condition(list);
				emit(2, 0, 13);
				regCount--;
			}
		}
	}
}

// builds a conditional statement
void condition(lexeme *list)
{
	if (parseError)
	{
		return;
	}
	// debug printer to track which function an error may occur in
	if (debug)
	{
		printf("Function: Condition()");
		printf("Relational operator: %d", list[tokenIndex].type);
	}

	if (token.type == lparensym)
	{
		getNextToken(list);
		logic(list);
		if (token.type == rparensym)
		{
			getNextToken(list);
		}
		else 
		{
			parseError = 1;
			printparseerror(12);
			return;
		}
	}
	else
	{
		expression(list);

		// if/else if branch to see which comparator the token may be
		if (token.type == eqlsym)
		{
			getNextToken(list);
			expression(list);
			if (parseError)
			{
				return;
			}

			// emit EQL
			emit(2, level, 6);
			regCount--;
		}
		else if (token.type == neqsym)
		{
			getNextToken(list);
			
			expression(list);
			if (parseError)
			{
				return;
			}

			// emit NEQ
			emit(2, level, 7);
			regCount--;
		}
		else if (token.type == lsssym)
		{
			getNextToken(list);
			
			expression(list);
			if (parseError)
			{
				return;
			}

			// emit LSS
			emit(2, level, 8);
			regCount--;
		}
		else if (token.type == leqsym)
		{
			getNextToken(list);
			
			expression(list);
			if (parseError)
			{
				return;
			}

			// emit LEQ
			emit(2, level, 9);
			regCount--;
		}
		else if (token.type == gtrsym)
		{
			getNextToken(list);
			
			expression(list);
			if (parseError)
			{
				return;
			}

			// emit GTR
			emit(2, level, 10);
			regCount--;
		}
		else if (token.type == geqsym)
		{
			getNextToken(list);
			expression(list);
			if (parseError)
			{
				return;
			}

			// emit GEQ
			emit(2, level, 11);
			regCount--;
		}
		else
		{
			parseError = 1;
			printparseerror(10);
			return;
		}
	}
	
}

// creates a term
void term(lexeme *list)
{
	if (parseError)
	{
		return;
	}

	// another debug print statement
	if (debug)
	{
		printf("Function: Term()");
	}

	factor(list);
	if (parseError)
	{
		return;
	}

	// while a div or mult symbol is the current token
	while (token.type == multsym || token.type == divsym)
	{	
		
		if (token.type == multsym)
		{
			getNextToken(list);
			
			factor(list);
			if (parseError)
			{
				return;
			}

			// emit MUL
			emit(2, 0, 4);
			regCount--;
		}
		else
		{	
			getNextToken(list);
			
			factor(list);
			if (parseError)
			{
				return;
			}

			// emit DIV
			emit(2, 0, 5);
			regCount--;
		}
	}
	
}

// finds integers that are being evaluated
void factor(lexeme *list) 
{
	int varsymIdx;
	int constsymIdx;

	if (parseError)
	{
		return;
	}

	if (debug)
	{
		printf("Function: Factor()");
	}

	// the token is an identifier
	if (token.type == identsym)
	{
		// stores any variable and constants by the same name
		varsymIdx = findsymbol(list[tokenIndex].name, 2);
		constsymIdx = findsymbol(list[tokenIndex].name, 1);

		// prints the corresponding error if none are found
		if (varsymIdx == -1 && constsymIdx == -1)
		{
			if (findsymbol(list[tokenIndex].name, 3) != -1)
			{
				parseError = 1;
				printparseerror(11);
				return;
			}
			else
			{
				parseError = 1;
				printparseerror(19);
				return;
			}
		}	
		
		// we have found the constant
		if (varsymIdx == -1)
		{
			// makes sure the register height is not exceeded
			regCount++;
			if (regCount > MAX_REG_HEIGHT)
			{
				parseError = 1;
				printparseerror(20);
				return;
			}

			// emits LIT
			emit(1, 0, table[constsymIdx].val);
		}
		else if (constsymIdx == -1 || table[varsymIdx].level > table[constsymIdx].level)
		{
			// ensures register height is not exceeded
			regCount++;
			if (regCount > MAX_REG_HEIGHT)
			{
				parseError = 1;
				printparseerror(20);
				return;
			}

			// emits LOD
			emit(3, level - table[varsymIdx].level, table[varsymIdx].addr);
		}
		else
		{
			// ensures register height is not exceeded
			regCount++;
			if (regCount > MAX_REG_HEIGHT)
			{
				parseError = 1;
				printparseerror(20);
				return;
			}

			// emits LIT
			emit(1, 0, table[constsymIdx].val);
		}	
		getNextToken(list);
	}
	else if (token.type == numbersym)
	{
		regCount++;
		if (regCount > MAX_REG_HEIGHT)
		{
			parseError = 1;
			printparseerror(20);
			return;
		}

		// emit LIT
		emit(1, 0, list[tokenIndex].value);
		
		getNextToken(list);
	}
	else if (token.type == lparensym)
	{
		getNextToken(list);
		expression(list);
		if (parseError)
		{
			return;
		}
		if (token.type != rparensym)
		{
			parseError = 1;
			printparseerror(12);
			return;
		}
		getNextToken(list);
	}
	else
	{
		parseError = 1;
		printparseerror(11);
		return;
	}

}

// takes in the lexeme list and returns the number of variables
// we have to store in the table
int variable(lexeme *list)
{
	char name[12];
	int num = 0;

	if (parseError)
	{
		return num;
	}

	// yet another debug statement
	if (debug)
	{
		printf("Function: Variable()");
	}

	if (token.type == varsym)
	{
		// executes at least once since we already detected a varsym
		// continues to loop as long as the next token is a comma
		do {
			num++;
			getNextToken(list);

			if (token.type != identsym)
			{
				parseError = 1;
				printparseerror(3);
				return num;
			}

			// ensures we dont have another variable with the same name
			// in the same level
			symIdx = multipledeclarationcheck(list[tokenIndex].name);

			if (symIdx != -1)
			{
				parseError = 1;
				printparseerror(18);
				return num;
			}

			// adds the variable to the symbol table
			strcpy(name, list[tokenIndex].name);
			addToSymbolTable(2, name, 0, level, num + 2, 0);
			getNextToken(list);

		} while (token.type == commasym);

		// we have to end the variable declarations with a semicolon
		// prints out the correct error and returns;
		if (token.type != semicolonsym)
		{
			if (token.type == identsym)
			{
				parseError = 1;
				printparseerror(13);
				return num;
			}
			else
			{
				parseError = 1;
				printparseerror(14);
				return num;
			}
		}
		getNextToken(list);
	}

	// return the number of variables we have found
	return num;
}

// helper function; takes in the lexeme list and retrieves the next token
void getNextToken(lexeme *list)
{
	token = list[++tokenIndex];
}

// adds a line of code to the program
void emit(int opname, int level, int mvalue)
{
	code[cIndex].opcode = opname;
	code[cIndex].l = level;
	code[cIndex].m = mvalue;
	cIndex++;

	if (debug)
	{
		printf("Function: Emit()");
		printf("%d , %d , %d\n", code[cIndex - 1].opcode, code[cIndex - 1].l, code[cIndex - 1].m);
	}
}

// add a symbol to the symbol table
void addToSymbolTable(int k, char n[], int v, int l, int a, int m)
{
	table[tIndex].kind = k;
	strcpy(table[tIndex].name, n);
	table[tIndex].val = v;
	table[tIndex].level = l;
	table[tIndex].addr = a;
	table[tIndex].mark = m;
	tIndex++;

	if (debug)
	{
		printf("Function: addToSymbolTable()");
		printf("%d , %s , %d, %d , %d , % d\n", 
				table[tIndex - 1].kind, table[tIndex - 1].name, table[tIndex - 1].val,
				table[tIndex - 1].level, table[tIndex - 1].addr, table[tIndex - 1].mark);
	}
}

// mark the symbols belonging to the current procedure, should be called at the end of block
void mark()
{
	int i;
	for (i = tIndex - 1; i >= 0; i--)
	{
		if (table[i].mark == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].mark = 1;
	}
}

// checks if a new symbol has a valid name, by checking if there's an existing symbol
// with the same name in the procedure
int multipledeclarationcheck(char name[])
{
	int i;
	for (i = 0; i < tIndex; i++)
	{
		if (table[i].mark == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	}
	return -1;
}

// returns the index of a symbol with a given name and kind in the symbol table
// returns -1 if not found
// prioritizes lower lex levels
int findsymbol(char name[], int kind)
{
	int i;
	int max_idx = -1;
	int max_lvl = -1;
	for (i = 0; i < tIndex; i++)
	{
		if (table[i].mark == 0 && table[i].kind == kind && strcmp(name, table[i].name) == 0)
		{
			if (max_idx == -1 || table[i].level > max_lvl)
			{
				max_idx = i;
				max_lvl = table[i].level;
			}
		}
	}
	return max_idx;
}

void printparseerror(int err_code)
{
	switch (err_code)
	{
		case 1:
			printf("Parser Error: Program must be closed by a period\n");
			break;
		case 2:
			printf("Parser Error: Constant declarations should follow the pattern 'ident := number {, ident := number}'\n");
			break;
		case 3:
			printf("Parser Error: Variable declarations should follow the pattern 'ident {, ident}'\n");
			break;
		case 4:
			printf("Parser Error: Procedure declarations should follow the pattern 'ident ;'\n");
			break;
		case 5:
			printf("Parser Error: Variables must be assigned using :=\n");
			break;
		case 6:
			printf("Parser Error: Only variables may be assigned to or read\n");
			break;
		case 7:
			printf("Parser Error: call must be followed by a procedure identifier\n");
			break;
		case 8:
			printf("Parser Error: if must be followed by then\n");
			break;
		case 9:
			printf("Parser Error: while must be followed by do\n");
			break;
		case 10:
			printf("Parser Error: Relational operator missing from condition\n");
			break;
		case 11:
			printf("Parser Error: Arithmetic expressions may only contain arithmetic operators, numbers, parentheses, constants, and variables\n");
			break;
		case 12:
			printf("Parser Error: ( must be followed by )\n");
			break;
		case 13:
			printf("Parser Error: Multiple symbols in variable and constant declarations must be separated by commas\n");
			break;
		case 14:
			printf("Parser Error: Symbol declarations should close with a semicolon\n");
			break;
		case 15:
			printf("Parser Error: Statements within begin-end must be separated by a semicolon\n");
			break;
		case 16:
			printf("Parser Error: begin must be followed by end\n");
			break;
		case 17:
			printf("Parser Error: Bad arithmetic\n");
			break;
		case 18:
			printf("Parser Error: Confliciting symbol declarations\n");
			break;
		case 19:
			printf("Parser Error: Undeclared identifier\n");
			break;
		case 20:
			printf("Parser Error: Register Overflow Error\n");
			break;
		default:
			printf("Implementation Error: unrecognized error code\n");
			break;
	}
	
	free(code);
	free(table);
}

void printsymboltable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < tIndex; i++)
		printf("%4d | %11s | %5d | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr, table[i].mark); 
	
	free(table);
	table = NULL;
}

void printassemblycode()
{
	int i;
	printf("Line\tOP Code\tOP Name\tL\tM\n");
	for (i = 0; i < cIndex; i++)
	{
		printf("%d\t", i);
		printf("%d\t", code[i].opcode);
		switch (code[i].opcode)
		{
			case 1:
				printf("LIT\t");
				break;
			case 2:
				switch (code[i].m)
				{
					case 0:
						printf("RET\t");
						break;
					case 1:
						printf("NEG\t");
						break;
					case 2:
						printf("ADD\t");
						break;
					case 3:
						printf("SUB\t");
						break;
					case 4:
						printf("MUL\t");
						break;
					case 5:
						printf("DIV\t");
						break;
					case 6:
						printf("EQL\t");
						break;
					case 7:
						printf("NEQ\t");
						break;
					case 8:
						printf("LSS\t");
						break;
					case 9:
						printf("LEQ\t");
						break;
					case 10:
						printf("GTR\t");
						break;
					case 11:
						printf("GEQ\t");
						break;
					case 12:
						printf("AND\t");
						break;
					case 13:
						printf("ORR\t");
						break;
					case 14:
						printf("NOT\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			case 3:
				printf("LOD\t");
				break;
			case 4:
				printf("STO\t");
				break;
			case 5:
				printf("CAL\t");
				break;
			case 6:
				printf("INC\t");
				break;
			case 7:
				printf("JMP\t");
				break;
			case 8:
				printf("JPC\t");
				break;
			case 9:
				switch (code[i].m)
				{
					case 1:
						printf("WRT\t");
						break;
					case 2:
						printf("RED\t");
						break;
					case 3:
						printf("HAL\t");
						break;
					default:
						printf("err\t");
						break;
				}
				break;
			default:
				printf("err\t");
				break;
		}
		printf("%d\t%d\n", code[i].l, code[i].m);
	}
	if (table != NULL)
		free(table);
}