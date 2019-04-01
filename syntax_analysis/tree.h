#include <stdlib.h>

#define MAX_CHILDREN 128

typedef struct node {
    struct node * children[MAX_CHILDREN];
    
    // flags
    unsigned int root              : 1;
    unsigned int funcDecl          : 1;
    unsigned int varDecl           : 1;
    unsigned int func              : 1;
    unsigned int var               : 1;
    unsigned int assStmt           : 1;
    unsigned int selectStmt        : 1;
    unsigned int returnStmt        : 1;
    unsigned int ifStmt            : 1;
    unsigned int varStmt           : 1;
    unsigned int compStmt          : 1;
    unsigned int eqlStmt           : 1;
    unsigned int simpleAddStmt     : 1;  //  "+" or "="
    unsigned int simpleMultStmt    : 1;  //  "*" or "\"
    unsigned int complexAddStmt    : 1;
    unsigned int complexMultStmt   : 1;

    // token vars
    char *val;
    char *ID;

} Node;


Node * get_new_tree();
Node * get_zeroed_node();

int new_child_node(Node *Tree);

