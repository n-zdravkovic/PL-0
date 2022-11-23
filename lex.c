/*
	Nick Zdravkovic
	COP3402 - Spring 2022
	HW2 - Lexical Analyzer
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#define MAX_NUMBER_TOKENS 1000
#define MAX_IDENT_LEN 11
#define MAX_NUMBER_LEN 5
#define RESERVED_LENGTH 13
#define SYMBOL_LENGTH 21

const char *reserved_words[] = {
	"const", "var", "procedure", "call", "begin",
	"end", "if", "then", "else", "while", "do",
	"read", "write"
};

const token_type reserved_tokens[] = {
	constsym, varsym, procsym, callsym, beginsym,
	endsym, ifsym, thensym, elsesym, whilesym, dosym,
	readsym, writesym
};

const char *special_symbols[] = {
	"==", "!=", "<", "<=", ">", ">=", "*", "/", "+",
	"-", "(", ")", ",", ".", ";", ":=", "&&", "||", "!", "/*", "*/"
};

const token_type symbol_tokens[] = {
	eqlsym, neqsym, lsssym, leqsym, gtrsym, geqsym,
	multsym, divsym, plussym, minussym, lparensym, rparensym,
	commasym, periodsym, semicolonsym, assignsym, andsym, orsym, notsym
};


lexeme *list;
lexeme temp;
int lex_index;

char helper[3];
char number_buffer[MAX_IDENT_LEN + 1];
char word_buffer[MAX_NUMBER_LEN + 1];
char symbol_buffer[2048];

int comment_flag = 0;
int error = 0;

void printlexerror(int type);
void printtokens();
void processSymbol(char *symbol);

// processes any words found in the program
void processWord(char *word)
{
	int i;
	int len = strlen(word);

	// takes the word and checks if it is a reserved word
	// sets it to the correct type and adds it to the list
	for (i = 0; i < RESERVED_LENGTH; i++)
	{
		if (strcmp(word, reserved_words[i]) == 0)
		{
			temp.type = reserved_tokens[i];
			list[lex_index++] = temp;
			return;
		}
	}

	// otherwise, the word is an identifier
	// sets the lexem to the correct type and adds it to the list
	temp.type = identsym;
	strcpy(temp.name, word);
	list[lex_index++] = temp;
}

// processes any number found in the program
void processNumber(char *number)
{
	int i;
	int len = strlen(number);

	// sets the type to a number token and adds it to the list
	temp.type = numbersym;
	temp.value = atoi(number);
	list[lex_index++] = temp;
}

void actualProcessor( char *symbol)
{
	int i = 0;

	if (symbol[0] == '\0')
	{
		return;
	}

	// checks if the input symbol is an open or closing comment
	// and sets the comment_flag accordingly
	if (strcmp(special_symbols[SYMBOL_LENGTH - 1], symbol) == 0)
	{
		comment_flag = 0;
		return;	
	}
	if (strcmp(special_symbols[SYMBOL_LENGTH - 2], symbol) == 0)
	{
		comment_flag = 1;
		return;	
	}

	// otherwise, goes through the special_symbols array
	// and stores the new lexeme into the list
	for (i = 0; i < SYMBOL_LENGTH - 2; i++)
	{
		if (strcmp(special_symbols[i], symbol) == 0 && !comment_flag)
		{
			temp.type = symbol_tokens[i];
			list[lex_index++] = temp;
			return;
		}
	}

	// if the symbol is not valid (ex '^'), then it sets the error flag
	if (!comment_flag && !error)
	{
		error = 1;
	}
}	

// breaks the incoming symbol array into valid chunks (as we can have
// a long list of valid symbols without whitespace)
void helperPunct(char *symbol)
{
	helper[0] = symbol[0];
	helper[1] = '\0';
	helper[2] = '\0';

	// checks if the incoming symbol has a valid two-character
	// symbol (ex: ":=" is the valid assign symbol) and breaks out of the loop
	while (1)
	{
		if (symbol[0] == ':')
		{
			if (symbol[1] == '=')
			{
				break;
			}
		}
		if (symbol[0] == '/')
		{
			if (symbol[1] == '*')
			{
				break;
			}
		}
		if (symbol[0] == '*')
		{
			if (symbol[1] == '/')
			{
				break;
			}
		}
		if (symbol[0] == '>')
		{
			if (symbol[1] == '=')
			{
				break;
			}
		}
		if (symbol[0] == '<')
		{
			if (symbol[1] == '=')
			{
				break;
			}
		}
		if (symbol[0] == '=')
		{
			if (symbol[1] == '=')
			{
				break;
			}
		}
		if (symbol[0] == '!')
		{
			if (symbol[1] == '=')
			{
				break;
			}
		}
		if (symbol[0] == '&')
		{
			if (symbol[1] == '&')
			{
				break;
			}
		}
		if (symbol[0] == '|')
		{
			if (symbol[1] == '|')
			{
				break;
			}
		}

		// otherwise, the symbol is only one char long
		// and we need to send it to process
		// also increment the symbol array by 1 to eliminate the char we just processed
		actualProcessor(helper);
		processSymbol(symbol + 1);
		return;
	}

	// the first symbol was two char long and we process it
	// also increments the symbol array by 2 to eliminate the two chars
	// we just processed
	helper[1] = symbol[1];
	actualProcessor(helper);
	processSymbol(symbol + 2);
}

// takes the symbol string and sends it into the correct helper function
void processSymbol(char *symbol)
{
	if (strlen(symbol) > 1)
	{
		// sends the symbol array to be broken down in smaller chunks
		helperPunct(symbol);
	}
	else
	{
		// otherwise, the length is 1 and we can process it immediately
		actualProcessor(symbol);
	}
}

lexeme *lexanalyzer(char *input, int printFlag)
{
	list = malloc(sizeof(lexeme) * MAX_NUMBER_TOKENS);
	lex_index = 0;
	int i = 0, place = 0, len = 0;

	// loops through the program file character by character
	while (input[i] != '\0')
	{
		// flag to indicate an invalid symbol was detected
		if (error)
		{
			printf("%d\n", i);
			printlexerror(4);
			return NULL;
		}

		// skips over whitespace
		if (isspace(input[i]) || iscntrl(input[i]))
		{
			i++;
			continue;
		}
		
		// the executes if we have detected a number
		if (isdigit(input[i])&& !comment_flag)
		{
			// don't do anything if we are in a comment
			if (comment_flag)
			{
				i++;
				continue;
			}

			// add the digit to the number_buffer
			number_buffer[place++] = input[i];
			
			// and continues the next character if they are also numbers
			while (isdigit(input[++i]))
			{
				if (isdigit(input[i]))
				{
					number_buffer[place++] = input[i];
				}
			}
			number_buffer[place] = '\0';

			// stores the length of the number and prints out the correct error
			// either the number is too long (greater than 5 digits) or we encountered 
			// a letter (which means it's an invalid indentifier)
			len = strlen(number_buffer);
			if (len > MAX_NUMBER_LEN && !comment_flag)
			{
				printlexerror(2);
				return NULL;
			}
			if (!isspace(input[i]) && !ispunct(input[i]) && isalpha(input[i]) && !comment_flag)
			{
				printlexerror(1);
				return NULL;
			}

			// if it passes the checks, send the number to be processed
			processNumber(number_buffer);
			place = 0;
		}

		// executes if we encounter a word
		if (isalpha(input[i]))
		{
			// again, don't process the word if we are in a comment
			if (comment_flag)
			{
				i++;
				continue;
			}

			// add the initial letter to the word_buffer
			word_buffer[place++] = input[i];

			// and continue adding characters to the word_buffer
			// if it's a letter or number
			while (isalnum(input[++i]))
			{
				if (isalnum(input[i]))
				{
					word_buffer[place++] = input[i];
				}
			}
			word_buffer[place] = '\0';

			// gets the length of the word and prints the correct error if it's
			// too long
			len = strlen(word_buffer);
			if (len > MAX_IDENT_LEN && !comment_flag)
			{
				printlexerror(3);
				return NULL;
			}

			// otherwise, the word passes the check and is sent to be processed
			processWord(word_buffer);
			place = 0;
		}

		// executes if we encounter a punctuation mark (which forms the foundation of our symbols)
		if (ispunct(input[i]))
		{
			// adds the initial symbol to our symbol_buffer
			symbol_buffer[place++] = input[i];
			
			// and continues adding characters to the symbol_buffer
			// as long as the next character is a punctuation mark
			while (ispunct(input[++i]))
			{
				if (ispunct(input[i]))
				{
					symbol_buffer[place++] = input[i];
				}
			}
			symbol_buffer[place] = '\0';

			// sends the symbol over to be processed
			// no error checking on length because whitespace is not guaranteed
			processSymbol(symbol_buffer);
			place = 0;
		}
		place = 0;

	}
	
	// a closing comment symbol was never encountered
	// so we print out the corresponding error
	if (input[i] == '\0' && comment_flag)
	{
		printlexerror(5);
		return NULL;
	}

	// otherwise, there were no errors and the program was read successfully
	// calls the printtoken() function to print out the list of lexemes
	if (!error && printFlag)
	{
		printtokens();
	}
	
	// these last two lines are really important for the rest of the package to run
	list[lex_index].type = -1;
	return list;
}

void printtokens()
{
	int i;
	printf("Lexeme Table:\n");
	printf("lexeme\t\ttoken type\n");
	for (i = 0; i < lex_index; i++)
	{
		switch (list[i].type)
		{
			case eqlsym:
				printf("%11s\t%d", "==", eqlsym);
				break;
			case neqsym:
				printf("%11s\t%d", "!=", neqsym);
				break;
			case lsssym:
				printf("%11s\t%d", "<", lsssym);
				break;
			case leqsym:
				printf("%11s\t%d", "<=", leqsym);
				break;
			case gtrsym:
				printf("%11s\t%d", ">", gtrsym);
				break;
			case geqsym:
				printf("%11s\t%d", ">=", geqsym);
				break;
			case multsym:
				printf("%11s\t%d", "*", multsym);
				break;
			case divsym:
				printf("%11s\t%d", "/", divsym);
				break;
			case plussym:
				printf("%11s\t%d", "+", plussym);
				break;
			case minussym:
				printf("%11s\t%d", "-", minussym);
				break;
			case lparensym:
				printf("%11s\t%d", "(", lparensym);
				break;
			case rparensym:
				printf("%11s\t%d", ")", rparensym);
				break;
			case commasym:
				printf("%11s\t%d", ",", commasym);
				break;
			case periodsym:
				printf("%11s\t%d", ".", periodsym);
				break;
			case semicolonsym:
				printf("%11s\t%d", ";", semicolonsym);
				break;
			case assignsym:
				printf("%11s\t%d", ":=", assignsym);
				break;
			case beginsym:
				printf("%11s\t%d", "begin", beginsym);
				break;
			case endsym:
				printf("%11s\t%d", "end", endsym);
				break;
			case ifsym:
				printf("%11s\t%d", "if", ifsym);
				break;
			case thensym:
				printf("%11s\t%d", "then", thensym);
				break;
			case elsesym:
				printf("%11s\t%d", "else", elsesym);
				break;
			case whilesym:
				printf("%11s\t%d", "while", whilesym);
				break;
			case dosym:
				printf("%11s\t%d", "do", dosym);
				break;
			case callsym:
				printf("%11s\t%d", "call", callsym);
				break;
			case writesym:
				printf("%11s\t%d", "write", writesym);
				break;
			case readsym:
				printf("%11s\t%d", "read", readsym);
				break;
			case constsym:
				printf("%11s\t%d", "const", constsym);
				break;
			case varsym:
				printf("%11s\t%d", "var", varsym);
				break;
			case procsym:
				printf("%11s\t%d", "procedure", procsym);
				break;
			case identsym:
				printf("%11s\t%d", list[i].name, identsym);
				break;
			case numbersym:
				printf("%11d\t%d", list[i].value, numbersym);
				break;
			case andsym:
				printf("%11s\t%d", "&&", andsym);
				break;
			case orsym:
				printf("%11s\t%d", "||", orsym);
				break;
			case notsym:
				printf("%11s\t%d", "!", notsym);
				break;
		}
		printf("\n");
	}
	printf("\n");
	printf("Token List:\n");
	for (i = 0; i < lex_index; i++)
	{
		if (list[i].type == numbersym)
			printf("%d %d ", numbersym, list[i].value);
		else if (list[i].type == identsym)
			printf("%d %s ", identsym, list[i].name);
		else
			printf("%d ", list[i].type);
	}
	printf("\n");
}

void printlexerror(int type)
{
	if (type == 1)
		printf("Lexical Analyzer Error: Invalid Identifier\n");
	else if (type == 2)
		printf("Lexical Analyzer Error: Number Length\n");
	else if (type == 3)
		printf("Lexical Analyzer Error: Identifier Length\n");
	else if (type == 4)
		printf("Lexical Analyzer Error: Invalid Symbol\n");
	else if (type == 5)
		printf("Lexical Analyzer Error: Never-ending comment\n");
	else
		printf("Implementation Error: Unrecognized Error Type\n");
	
	free(list);
	return;
}