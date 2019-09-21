# Compilers

## lexical_analysis
Lexical analysis of a C-minus program. C-minus is a minimal C-like
language (see lexical_analysis/CminusSpec.pdf). 

The program accepts a C-minus file as input and outputs a list of tokens.

See lexical_analysis/readme.txt
  
## syntax_analysis
Parsing of lexical analysis token list for semantic and syntactic errors.

The program accepts a C-minus file as input and outputs a parse tree upon
successful parsing.

See syntax_analysis/readme.txt
  
## code_gen
Code generation from syntax analysis parse tree.

The program accepts a C-minus file as input and outputs x86 assembly code.

See code_gen/readme.txt
