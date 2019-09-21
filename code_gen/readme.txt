Program: codegen
    Generates x86 assembly code from the parse tree generated in syntax
    analysis. Codegen runs our parser from syntax analysis modified to be a
    function that returns a parse tree. 

    Codegen outputs MIPS assembly to the output file.


Usage: 
    make
    ./codegen <input-file> <output-file>


Example:     
    
    if the input file contains:

       void main(void){
           int a;
           a = 7;
       }

    the output will be:              

      .data

      .text
      
      main:
      move $fp $sp
      sw $ra, 0($sp)
      addiu $sp, $sp, -4
      addiu $sp, $sp, -4 #a
      li $a0, 7
      sw $a0, -4($fp)
      end_main:
      lw $ra, 8($sp)
      addiu $sp, $sp, 12
      jr $ra
      
      
      .text
      .globl __crt_out_of_bounds
      .globl output
      .globl input
      
      __crt_out_of_bounds:
      li $v0, 10 # exit
      syscall
      
      output:
      lw $a0, 4($sp)
      li $v0, 1
      syscall
      li $v0, 11
      li $a0, 0x0a
      syscall
      addiu $sp, $sp, 4
      li $a0, 0
      j $ra
      
      input:
      li $v0, 5
      syscall
      jr $ra     
             
      

Contents:

    testinputs       - test input files

    CminusSpec.pdf   - the specification for the C minus programming language

    Makefile         - run 'make" to build codegen

    codegen.c        - main C file for codegen program

    codegen.h        - codegen header file

    geracodigo       - reference code generator (outputs correct MIPS)

    lex.c            - code for lex

    lex.h            - lex header file

    output           - the MIPS generated when
                           ./codegen testinputs/test-program-simple.in output
		       is ran

    parser.c         - code for parser

    parser.h         - parser header file

    submitted.tar.gz - version that was submitted for grading

    symtable.c       - code for symbol table

    symtable.h       - symtable header file

    tree.c           - code for binary tree

    tree.h           - tree header file
