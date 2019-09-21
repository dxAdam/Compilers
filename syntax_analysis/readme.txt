Program: parser
    parses a list of C-minus tokens for semantic and syntax errors. The parser
    runs our lexer from lexical_analysis modified to be a function that returns
    a list of tokens to the parser. If there are no semantic or syntactic errors
    the parser outputs a parse tree, if errors are detected there is no output.

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

