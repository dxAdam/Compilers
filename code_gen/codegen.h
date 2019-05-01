#include <stdio.h>

#include "tree.h"


extern Node * parse(int argc, char *argv[]);
extern void printTree(FILE *ofp, Node *Tree);
extern void print_symboltable();
extern void print_scopeVars(FILE *opf, char *scope);
extern int get_pos(char *id);
extern int set_pos(char *id, int pos);

int codegen(Node *AST);

void push_ret_addr();
void pop_ret_addr(int num_args);
void push_acc();

void standard_final();

void cgen(Node *AST);

void post_order_traverse(Node *op);