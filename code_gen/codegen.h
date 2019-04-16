#include <stdio.h>

#include "tree.h"


extern Node * parse(int argc, char *argv[]);
extern void printTree(FILE *ofp, Node *Tree);

int codegen(Node *AST);

void push_ret_addr();
void push_acc();

void cgen(Node *AST);