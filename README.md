# Compilers

## lexical_analysis
Lexical analysis of a C-minus program. C-minus is a minimal C-like
language (see lexical_analysis/CminusSpec.pdf). 

The program accepts a C-minus file as input and outputs a list of tokens to a file.

See lexical_analysis/readme.txt
  
## syntax_analysis
Parsing of lexical analysis token list for semantic and syntactic errors.

The program accepts a C-minus file as input and writes a parse tree to an output
file upon successful parsing.

See syntax_analysis/readme.txt
  
## code_gen
Code generation from syntax analysis parse tree.

The program accepts a C-minus file as input and writes MIPS assembly code to
an output file.

See code_gen/readme.txt
