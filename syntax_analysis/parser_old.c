#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

typedef enum { false, true } bool;

token_t tokens[1000];

bool isVarDecl(int index){
    int i = index;
    if(!strcmp(tokens[i].ID, "int")){
        if(!strcmp(tokens[i+2].ID, ";") || !(strcmp(tokens[i+2].ID, "[")))
            return true;
    }

    return false;
}

bool isFuncDecl(int index){
    int i = index;
    if(!strcmp(tokens[i].ID, "void") && !strcmp(tokens[i+2].ID, "("))
        return true;
    if(!strcmp(tokens[i].ID, "int") && !strcmp(tokens[i+2].ID, "("))
        return true;

    return false;
}

int newVarDeclNode(int i, Node *Tree){
    int j = 0;
    j = new_child_node(Tree);

    Tree->children[j]->varDecl = 1;

    i = handleVarDecl(i, Tree->children[j]);

    return i;
}

int newFuncDeclNode(int i, Node *Tree){
    int j=0;
    j = new_child_node(Tree);
    
    Tree->children[j]->funcDecl = 1;

    i = handleFuncDecl(i, Tree->children[j]);

    return  i;
}

int newVarNode(int i, Node *parent){
    int j = 0;
    j = new_child_node(parent);

    parent->children[j]->var = 1;
    parent->children[j]->ID = "var";

    int j2 = new_child_node(parent->children[j]);

    parent->children[j]->children[j2]->ID = tokens[i].ID;
   
    if(!strcmp(tokens[i+1].ID, "["))
        printf("deteced that %s is an array w/ index %s\n", tokens[i].ID, tokens[i+2].ID);

   
    return i+=1;
}

int newAssignNode(int i, Node *parent){
    if(!strcmp(tokens[i].ID, "=")){
        int j = 0;
        j = new_child_node(parent);
        parent->children[j]->assStmt = 1;
        parent->children[j]->ID = "=";
        //i++;

        if(tokens[i].type == T_NUM){
            printf("NUM: %s", tokens[i].num);
        }

    }
    
    ;
}

int newEqlStmtNode(int i, Node *eqlStmt){
    printf("handling eql stmt\n");
    
    int j = 0;
    j = new_child_node(eqlStmt);

    eqlStmt->children[j]->ID = "==";
    eqlStmt->children[j]->eqlStmt = 1;
    
    i = newVarNode(i, eqlStmt->children[j]);

    // add variable value 
    j = new_child_node(eqlStmt);
    eqlStmt->children[j]->ID = tokens[i+1].num;

    //i = handleEqlStmt(i, eqlStmt);
    return i;
}

int newIfStmtNode(int i, Node *Tree){
    int j = 0;
    j = new_child_node(Tree);

    Tree->children[j]->ID = "selection-stmt";
    Tree->children[j]->ifStmt = 1;

    i = handleIfStmt(i, Tree->children[j]);

    return i;
}

int handleVarDecl(int i, Node *varNode){
    int j = 0;


    if(varNode->compStmt == 1)
        printf("handleVarDecl Comp\n");


    // add "int"
    j = new_child_node(varNode);
    varNode->children[j]->ID = tokens[i].ID;    

    if(!strcmp(varNode->children[j]->ID, "void"))
        return i++;

    i++;

    // add variable ID
    j = new_child_node(varNode);
    varNode->children[j]->ID = tokens[i].ID;
    i++;

    if(!strcmp(tokens[i].ID, "[") && !(strcmp(tokens[i+1].ID, "]"))){    // add variable ID
        j = new_child_node(varNode);
        varNode->children[j]->ID = "\\[\\]";
        i++;
    }
    else if(!strcmp(tokens[i].ID, "[")){
        j = new_child_node(varNode);
        varNode->children[j]->ID = tokens[i+1].num;
        i++;
    }


    //if(strcmp(tokens[i].ID, ";"))
    //    printf("ERROR: No ';' after var decl\n");


    return i;
}

int handleFuncDecl(int i, Node *funcNode){
    int j = 0;

    //printf("handleFuncDecl token.ID : %s\n", tokens[i].ID);

    // add function return type
    j = new_child_node(funcNode);
    funcNode->children[j]->ID = tokens[i].ID;
    i++;

    // add function ID
    j = new_child_node(funcNode);
    funcNode->children[j]->ID = tokens[i].ID;  
    i++;

    // add param node
    j = new_child_node(funcNode);
    funcNode->children[j]->ID = "params";  

    i = handleFuncParams(i, funcNode->children[j]);

    // handle compound stmt
    j = new_child_node(funcNode);
    funcNode->children[j]->ID = "compound-stmt";
    funcNode->children[j]->compStmt = 1;

    i += 1;

    i = handleCompStmt(i, funcNode->children[j]);



    return i;
}

int handleFuncParams(int i, Node *params){
    // skip (
    if(strcmp(tokens[i++].ID, "(")){
        printf("ERROR: ( not detected\n");
        exit(1); // ERROR
    }

    int j = 0;
    while((!strcmp(tokens[i].ID, "int") || !strcmp(tokens[i].ID, "void"))){
        j = new_child_node(params);
        params->children[j]->ID = "param";
        
        i = handleVarDecl(i, params->children[j]);
        i++;
    }

    return i;
}

int handleCompStmt(int i, Node *compStmt){
    i++;

    while(strcmp(tokens[i].ID, "}")){
        if(tokens[i].type == T_ID){
            printf("! token id: %s\n", tokens[i].ID);
            i = newAssignNode(i, compStmt);
        }
        else if(tokens[i].type == T_KEY){
            if(isVarDecl(i)){
                i = newVarDeclNode(i, compStmt);
                printf("compstmt varDecl tokens[i].ID : %s\n", tokens[i+1].ID);
            }
            else if(!strcmp(tokens[i].ID, "if")){
                printf("ifstmt\n");
                i = newIfStmtNode(i, compStmt);
                i++;
            }
            else if(!strcmp(tokens[i].ID, "return")){
                printf("returnstmt\n");
                i+=2;
            }   
            else if(!strcmp(tokens[i].ID, "while")){
                printf("whilestmt\n");
                i += 19;
            }
        }
        else if(tokens[i].type == T_SYM)
            ;// SYM
            i++;
    }



    //printf("token[i].type : %s\n", tokens[i].ID);

    return i;
}

int handleIfStmt(int i, Node *ifStmtNode){
    i+=2;
    if(tokens[i].type == T_ID){
        if(!strcmp(tokens[i+1].ID, "==")){
            printf("new == stmt\n");
            i = newEqlStmtNode(i, ifStmtNode);
        }

    }

    // handle return stmt

    return i;
}

int handleEqlStmt(int i, Node *eqlStmt){
    printf("handling eql stmt\n");
    ;
}

void printTree(Node *Tree){

    Node *params;
    Node *child;
    Node *compStmt;

    if(Tree->root == 1)
        printf("[program\n");
    
    int i = 0;
    while(Tree->children[i] != NULL){
        child = Tree->children[i];
        if(child->funcDecl == 1){
            printf("  [fun-declaration\n");
            printf("    [%s]\n", child->children[0]->ID);
            printf("    [%s]\n", child->children[1]->ID);
            printf("    [params");

            params = child->children[2];
            int j = 0;

            if(!strcmp(params->children[j]->children[0]->ID, "void")){
                printf("]\n");
                j++;
            }
            else
                printf("\n");         

            while(params->children[j] != NULL){
                int k = 0;
                printf("      [%s ", params->children[j]->ID);
                while(params->children[j]->children[k] != NULL){
                    printf(" [%s]", params->children[j]->children[k++]->ID);
                }
                printf("]\n");
                j++;
            }

            compStmt = child->children[3];

            printCompStmt(compStmt);

        }
        else if(Tree->children[i]->varDecl == 1){
            printf("  [var-declaration ");
            int j = 0;
            while(Tree->children[i]->children[j] != NULL){
                printf(" [%s]", Tree->children[i]->children[j++]->ID);
            }

            printf("]\n");
        }
        else if(Tree->children[i]->var == 1);
        
        i++;
    }
}

void printCompStmt(Node *compStmt){
    printf("    [%s\n", compStmt->ID);
    
    int j = 0;
    while(compStmt->children[j] != NULL){

        if(compStmt->children[j]->varDecl == 1){
            printf("      [= [%s\n", compStmt->children[j]->ID);
        }
        else if(compStmt->children[j]->ifStmt == 1){
            printf("      [%s\n", compStmt->children[j]->ID);
            printIfStmt(compStmt->children[j]);
        }

        j++;
    }


}

void printIfStmt(Node *ifStmt){
    int j = 0;
    printf("        [%s ", ifStmt->children[0]->ID);
    printf("[%s", ifStmt->children[0]->children[0]->ID);
    printf(" [%s]]" ,ifStmt->children[0]->children[0]->children[0]->ID);
    printf(" [%s]", ifStmt->children[1]->ID);
    printf("]\n");
}

int main(int argc, char *argv[]){

    if(argc < 3) {
        printf("file names not detected!\n");
        printf("usage: ./lex <input file> <output file>\n");
        return 1;
    }

    // input and output file descriptors
    FILE *ifp, *ofp;
    
    if((ifp = fopen(argv[1], "r")) == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    if((ofp = fopen(argv[2], "w+")) == NULL){
        printf("Error preparing output file\n");
        return 1;
    }

    /* 
        should we store all tokens in an array? or pull them
        one at a time?? not sure yet...  let's do array with easy 
        backtracking for now. We'll be able to use gradescope to 
        verify size is big enough.
    */    
    int i = 0;

    tokens[i] = getToken(ifp);
    while(tokens[i].lineNumber != -1){
        tokens[++i] = getToken(ifp);
    }

    int j = 0;

    //printProgram(ofp);
    Node *Tree = get_new_tree();

    while(j != i){

        if(isFuncDecl(j)){            
            j = newFuncDeclNode(j, Tree);
        }
        else if(isVarDecl(j)){
            j = newVarDeclNode(j, Tree);
        }

        j++;
    }

    printTree(Tree);

    printf("\n-- exiting program normally\n");
}