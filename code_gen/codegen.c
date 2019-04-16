#include <stdio.h>

#include "codegen.h"

FILE *ofp;

int main(int argc, char *argv[]){

    if(argc < 3) {
        printf("file names not detected!\n");
        printf("usage: ./codegen <input file> <output file>\n");
        return 1;
    }

    // input and output file descriptors
    FILE *ifp;
    
    if((ifp = fopen(argv[1], "r")) == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    if((ofp = fopen(argv[2], "w+")) == NULL){
        printf("Error preparing output file\n");
        return 1;
    }


    // get abstract syntax tree from parser
    Node * AST = parse(argc, argv);

    // start code gen
    codegen(AST);

    printf("-- exiting codegen normally\n\n");
    return 1;
}

int codegen(Node *AST){

    if(AST->root == 1){
        fprintf(ofp, ".data\n");
        fprintf(ofp, "\n.text\n\n");
    }

    if(AST->funcDecl == 1){
        fprintf(ofp, "%s:\n", AST->children[1]->ID); // func name:
        push_ret_addr();
    }

    if(AST->param == 1){
        //push_param(AST->children[1]);
        return 1;
    }

    if(AST->op == 1){
        printf("-- op\n");
        cgen(AST);
    }
        
        

    if(AST->num == 1)
        printf("num: %s\n", AST->ID);

    if(AST->compStmt == 1)
        printf("-- compstmt\n");
    

    int i = 0;
    while(AST->children[i] != NULL){
        //printf("AST->children[%d]->ID == %s\n", i, AST->children[i]->ID);
        //printf("AST->children[%d]->funcDecl == %d\n", i, AST->children[i]->funcDecl);
        codegen(AST->children[i]);
        i++;
    }

    return 1;
}

void push_ret_addr(){
    fprintf(ofp, "  addiu $sp, $sp, -8\n");
    fprintf(ofp, "  sw $ra, 4($sp)\n");
}

void push_acc(){
    fprintf(ofp, "  sw $a0, 0($sp)\n");
    fprintf(ofp, "  addiu $sp, $sp, -4\n");
}

void cgen(Node *AST){
    ;
}

