#include <stdlib.h>

#define MAX_CHILDREN 128

typedef struct node {
    struct node * children[MAX_CHILDREN];
    
    // opNodes will use this struct as binary tree
    struct node * left; 
    struct node * right;
    
    // flags
    unsigned int root              : 1;
    unsigned int funcDecl          : 1;
    unsigned int varDecl           : 1;
    unsigned int func              : 1;
    unsigned int SYM               : 1;
    unsigned int num               : 1;
    unsigned int callAssign        : 1;
    unsigned int returnStmt        : 1;
    unsigned int controlStmt       : 1; 
    unsigned int compStmt          : 1;  
    unsigned int eqlStmt           : 1; 
    unsigned int callStmt          : 1;
    unsigned int semicolon         : 1;
    unsigned int op                : 1;
    unsigned int opRoot            : 1;
    unsigned int expr              : 1;
    unsigned int term              : 1;
    unsigned int factor            : 1;
    unsigned int param             : 1;
    unsigned int params            : 1;
    
    // opNodes calculate when created in get_op_node()
    int base10val;

    // token character or string
    char *ID;

} Node;



/* 
    functions in tree.c
*/

Node * get_new_tree();
Node * get_zeroed_node();
Node * new_op_node(Node *left, Node *right, char *op);

int new_child_node(Node *Tree);
int get_free_index(Node *parent);


