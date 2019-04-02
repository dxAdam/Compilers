#include <stdio.h>

#include "tree.h"

Node * get_new_tree(){
    Node *tree = (Node *)calloc(1, sizeof(Node));
    tree->root = 1;
    return tree;
}

Node * get_zeroed_node(){
    return (Node *)calloc(1, sizeof(Node));
}

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

