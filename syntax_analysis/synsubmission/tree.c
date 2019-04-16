#include <stdio.h>

#include "tree.h"

Node * get_new_tree(){
    Node *tree = (Node *)calloc(1, sizeof(Node));
    tree->left = NULL;
    tree->right = NULL;
    tree->root = 1;      // tree->root = 1 is the only difference to get_zeroed_node()
    return tree;
}

Node * get_zeroed_node(){
    Node *node = (Node *)calloc(1, sizeof(Node));
    node->left = NULL;
    node->right = NULL;
}


Node * new_op_node(Node *left, Node *right, char *op){
    Node *opNode = get_zeroed_node();
    opNode->ID = op;
    opNode->op = 1;
    opNode->left = left;
    opNode->right = right;

    return opNode;
}

// allocate new child and return its index
int new_child_node(Node *parent){
    int i = 0;
    while(parent->children[i] != NULL) i++;

    if(i==MAX_CHILDREN){
        printf("ERROR: Max Children\n");
        return -1; //error
    }

    parent->children[i] = get_zeroed_node();
    
    return i; //return index of new child 
}

// return index value of next available child
int get_free_index(Node *parent){
    int i = 0;
    while(parent->children[i] != NULL) i++;

    if(i==MAX_CHILDREN){
        printf("ERROR: Max Children\n");
        return -1; //error
    }
    
    return i; //return index of new child 
}