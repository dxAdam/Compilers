Program: lex
    performs a lexical analysis of an input file generates a list of tokens as
    an output file.

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

  
Format:

       (line number, type, id)





Contents:

	
