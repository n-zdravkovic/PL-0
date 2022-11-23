## About
This HW assignment finalizes the PL/0 compiler assignment for this course. parser.c, lex.c, and vm.c were written by Nick Zdravkovic in C. 

The compiler takes an input file with PL/0 code. This input will be scanned and converted into lexemes. The lexemes will be read, placed in the symbol table when appropriate, and analyzed for syntactic correctness. An error message will be printed if the input is not syntactically correct. If the program is well-made, assembly code will be generated from the lexeme toxens to run the virtual machine. Once the assembly code has been generated, the execution continues by executing the assembly code on the virtual machine.


## Errors
lex.c supports five different errors:
1) Invalid identifiers: a variable name begins with a non-alphabetic character
2) Number length: the length of a number is too long
3) Identifier length: the length of a variable name is too long
4) Invalid symbol: the encountered symbol is not supported
5) Neverending comment: the comment has never been closed

Encountering any of these errors, lex.c will print out the error message and terminate.

parser.c will generate four (4) types of errors, with 20 error messages:
1) The first type of errors are based on the absence of a particular symbol. Twelve (12) error messages are of this type. For example, the absence of a semicolon.
2) The second type of errors are those based on the presence of an unexpected symbol. The next five (5) errors are of this type. For example, recieving an identifier when it expected a comma.
3) The third type of errors arise when a conflict is detected in the symbol table. The next two (2) errors are of this type. For example, using an identifier that has not been used prior.
4) The final error type is the "Register Overflow Error", which occurs when more register space is used than is available.

Encountering any of these errors, parser.c will print out the error message and terminate. 


## Flags
Note that the compiler driver supports the following flags:
1) -s which will print out the symbol table to the command prompt window
2) -a which will print out the generated assembly code to the command prompt window
3) -v which will print out the virtual machine execution to the command prompt window
4) -l which will print out the lexeme table and lexeme list.


## Running
As before, ensure that all files (lex.c, vm.c, parser.c, driver.c, compiler.h, Makefile, PL/0 input files, and tester4.sh) are in the same directory with.

tester4.sh will require execution privileges and the default test cases (logicexample.txt and errorexample.txt). The corresponding output files will also be needed. Ensure these files are also in the same directory.

Navigate to the directory in the terminal using the command "cd". Use "make" to compile the code. Then use "./a.out <input file> <compiler flags>" to run it in the command prompt window.

If you want to use tester4.sh, navigate to the directory using the command terminal as before. Then type "./tester4.sh" and hit enter. 

