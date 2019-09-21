Program: parser
    parses a list of C-minus tokens for semantic and syntax errors. The parser
    runs our lexer from lexical_analysis modified to be a function that returns
    a list of tokens.

    if there are no semantic or syntactic errors the parser outputs a
    parse tree in the output file. If errors are detected the output file
    will be empty.


Usage: 
    make
    ./parser <input-file> <output-file>


Example:     
    
    our example in lexical_analysis would now fail (and output nothing) since
    not having a main function is a semantic error.

    if instead the input file contains:

       void main(void){
           int a;
           a = 7;
       }

    the output will be:              

       [program
         [fun-declaration
           [void]
           [main]
           [params]
           [compound-stmt
         [var-declaration  [int] [a]]
               [= [var [a]] [7]]
       
           ]
         ]
       ]
       


Contents:

    testinputs_sem   - semantic test inputs

    testinputs_syn   - syntactic test inputs

    Makefile         - run 'make" to build parser

    compare.sh       - compares parse's output to the reference parser's

    lex.c            - code for lex

    lex.h            - lex header file

    output           - the output generated when
                           ./parser testinputs_syn/test-program-simple.in output
                       is ran

    parser.c         - main c file for parser program

    parser.h         - parser header file

    sintatico        - reference parser (outputs correct parse tree)

    submitted.tar.gz - version that was submitted for grading

    symtable.c       - code for symbol table

    symtable.h       - symtable header file

    tree.c           - code for binary tree

    tree.h           - tree header file
