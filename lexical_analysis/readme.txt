Program: lex
    performs a lexical analysis of an input file and generates a list of tokens     as an output file.


Usage: 
    make
    ./lex <input-file> <output-file>


Example:     
    
    if the input file contains:

       int a;
       a = 7;

    the output will be:              

       (1,KEY,"int")             
       (1,ID,"a")
       (1,SYM,";")
       (2,ID,"a")
       (2,SYM,"=")
       (2,NUM,"7")
       (2,SYM,";")


    where the format is (linenumber,type,id)   


Contents:

    testinputs       - a collection of test input files

    Makefile         - run 'make' to compile the project

    compare.sh       - used to compare lex's output to reference compiler's

    lex.h            - lex header file

    lex.c            - main lex c file

    lexico           - reference compiler (output is correct)

    output           - the output produced when
			       ./lex testinputs/main.c output
	               is run

    submitted.tar.gz - version that was submitted for grading
