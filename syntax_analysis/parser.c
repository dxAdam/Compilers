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

    // add variable ID
    //printf("newVarNode: token id %s\n", tokens[i].ID);
    parent->children[j]->children[j2]->ID = tokens[i].ID;
    parent->children[j]->children[j2]->var = 1;
   
    if(!strcmp(tokens[i+1].ID, "[")){
        //printf("newVarNode: deteced that %s is an array w/ index %s\n", tokens[i].ID, tokens[i+2].num);
        j2 = new_child_node(parent->children[j]);
        if(tokens[i+2].type == T_NUM)
            parent->children[j]->children[j2]->ID = tokens[i+2].num;
        else
            parent->children[j]->children[j2]->ID = tokens[i+2].ID;

        i+=3;
    }

    //printf("newVarNode: exit\n");

    return ++i;
}

int newNumNode(int i, Node *parent){
    int j = 0;
    j = new_child_node(parent);

    parent->children[j]->var = 1;
    parent->children[j]->ID = tokens[i].ID;

    printf("put %s in node\n", tokens[i].ID);

    return i;
}

int newOpNode(int i, Node *opNode){
    int tmp = i;
    int i_max;

    //printf("new op node token: %s\n", tokens[i].ID);

    while(strcmp(tokens[tmp].ID, ";")) tmp++;

    i_max = tmp;

    // back up from ; to op symbol
    tmp -= 2;

        if(!strcmp(tokens[tmp].ID, "*") || !strcmp(tokens[tmp].ID, "\\")){
            //printf("!!  %s %s %s\n", tokens[tmp-1].ID, tokens[tmp].ID, tokens[tmp+1].ID);
            if(tmp == i+1){
                //printf("handling simpleAddOp\n");
                handleSimpleAddOp(i, opNode);
            }
            else{
                //printf("handling complexAddOp\n");
                tmp = handleComplexAddOp(i, tmp, opNode);
            }
        }
        else if(!strcmp(tokens[tmp].ID, "+") || !strcmp(tokens[tmp].ID, "-")){
            //printf("!!  %s %s %s\n", tokens[tmp-1].ID, tokens[tmp].ID, tokens[tmp+1].ID);
            if(tmp == i+1){
                //printf("handling simpleAddOp\n");
                handleSimpleAddOp(i, opNode);
            }
            else{
                //printf("handling complexAddOp\n");
                tmp = handleComplexAddOp(i, tmp, opNode);
            }
                
        }
   
    return i_max;
}

// parent could be compStmt or program
int newAssignNode(int i, Node *parent){
        int j = 0;
        j = new_child_node(parent);
        parent->children[j]->assStmt = 1;
        parent->children[j]->ID = "=";

        Node *assNode = parent->children[j];

        i = newVarNode(i, assNode);

        // skip "="
        i++;

        //printf("new assign node tokens[i].ID : %s\n", tokens[i].ID);
        //printf("new assign node tokens[i+1].ID : %s\n", tokens[i+1].ID);


        // add what variable is = to    //refine for complex stmts

        if(!strcmp(tokens[i+1].ID, ";")){
            int j2 = new_child_node(assNode);

            if(tokens[i].type == T_NUM)
                assNode->children[j2]->ID = tokens[i].num;
            else if(tokens[i].type == T_ID)
                assNode->children[j2]->ID = tokens[i].ID;
        }
        else if(tokens[i+1].type == T_SYM){
            //printf("making new op node\n");
            i = newOpNode(i, assNode);
        }
            

        return i;   
}

int newEqlStmtNode(int i, Node *ifStmt){
    //printf("handling eql stmt\n");
    
    int j = 0;
    j = new_child_node(ifStmt);

    ifStmt->children[j]->ID = "==";
    ifStmt->children[j]->eqlStmt = 1;
    
    i = newVarNode(i, ifStmt->children[j]);

    // add variable value
    int j2; 
    j2 = new_child_node(ifStmt->children[j]);
    ifStmt->children[j]->children[j2]->ID = tokens[i+1].num;

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

int newReturnNode(int i, Node *compStmt){
    
    int j=0;
    j = new_child_node(compStmt);

    Node * retNode = compStmt->children[j];
    retNode->returnStmt = 1;
    retNode->ID = "return-stmt"; // should be return

    //printf("made ret node j:%d\n", j);


    // see if return statement is simple of compound statement
    if(!strcmp(tokens[i+2].ID, ";")){
            i++;
            if(tokens[i].type == T_NUM)
                newNumNode(i, retNode);
            else if(tokens[i].type == T_ID)
                newVarNode(i, retNode);
    }
    i++;

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


    i++;


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
        
        // an ID is an assignment operation or function call
        if(tokens[i].type == T_ID){
            if(!strcmp(tokens[i+1].ID, "=") || !strcmp(tokens[i+1].ID, "[")){
                printf("! token id: %s\n", tokens[i].ID);
                i = newAssignNode(i, compStmt);
            }
            while(strcmp(tokens[i].ID, ";"))i++;
        }
        else if(tokens[i].type == T_KEY){
            if(isVarDecl(i)){
                //printf("compstmt varDecl tokens[i].ID : %s\n", tokens[i+1].ID);
                i = newVarDeclNode(i, compStmt);
                
            }
            else if(!strcmp(tokens[i].ID, "if")){
                printf("ifstmt\n");
                i = newIfStmtNode(i, compStmt);
            }
            else if(!strcmp(tokens[i].ID, "return")){
                printf("returnstmt\n");
                //while(strcmp(tokens[i].ID, ";"))i++;
                newReturnNode(i, compStmt);
            }   
            else if(!strcmp(tokens[i].ID, "while")){
                printf("whilestmt\n");
                while(strcmp(tokens[i].ID, ";"))i++;

            }
        }
        else if(tokens[i].type == T_SYM)
            ;    
        i++;

    
    }



    //printf("token[i].type : %s\n", tokens[i].ID);

    return i;
}

int handleIfStmt(int i, Node *ifStmtNode){
    i+=2;
    if(tokens[i].type == T_ID){
        if(!strcmp(tokens[i+1].ID, "==")){
            //printf("new == stmt\n");
            i = newEqlStmtNode(i, ifStmtNode);
        }

    }

    // handle compound stmt
    int j=0;
    j = new_child_node(ifStmtNode);
    ifStmtNode->children[j]->ID = "compound-stmt";
    ifStmtNode->children[j]->compStmt = 1;

    i = handleCompStmt(i, ifStmtNode->children[j]);
    
    //printf("handle if stmt : tokens[i].ID : %s\n", tokens[i].ID);
    
    // skip "}"

    return i;
}

int handleSimpleAddOp(int i, Node *parent){
    i += 1;
    int j = 0;
    j = new_child_node(parent);

    parent->children[j]->simpleAddStmt = 1;
    parent->children[j]->ID = tokens[i].ID;
    //printf("handleSimple: added %s\n", tokens[i].ID);
    
    int j2 = 0;
    if(tokens[i-1].type == T_NUM){
        j2 = new_child_node(parent->children[j]);
        parent->children[j]->children[j2]->ID = tokens[i-1].ID;
        //printf("handleSimple: added %s\n", tokens[i-1].ID);

    }
    else if(tokens[i-1].type == T_ID){
        newVarNode(i-1, parent->children[j]);
        //printf("handleSimple: added %s\n", tokens[i-1].ID);

    }

    if(tokens[i+1].type == T_NUM){
        j2 = new_child_node(parent->children[j]);
        parent->children[j]->children[j2]->ID = tokens[i+1].ID;
        //printf("handleSimple: added %s\n", tokens[i+1].ID);

    }
    else if(tokens[i+1].type == T_ID){
        newVarNode(i+1, parent->children[j]);
        //printf("handleSimple: added %s\n", tokens[i+1].ID);

    }

    return i;
}

int handleComplexAddOp(int i, int tmp, Node *assNode){
    
    //printf("tokens[tmp].ID : %s\n", tokens[tmp].ID);

    int j = 0;
    j = new_child_node(assNode);

    Node *opNode = assNode->children[j];

    opNode->simpleAddStmt = 1; //change to complex later?
    opNode->ID = tokens[tmp].ID;
    
    //printf("handleComplex: added %s\n", tokens[tmp].ID);

    // store current operator and replace with ; to use recursion
    char c[2];
    c[0] = *tokens[tmp].ID;

    //printf("stashing %s\n", tokens[tmp].ID);

    tokens[tmp].ID[0] = ';';

    newOpNode(i, opNode);
    
    tokens[tmp].ID[0] = *c;

    //printf("$restored %s\n", tokens[tmp].ID);

    // skip op symbol
    tmp++;

    j = new_child_node(opNode);
    //printf("complexAdd j: %d   tokens[tmp].ID : %s\n", j, tokens[tmp].ID);
    opNode->children[j]->ID = tokens[tmp].ID;
    
    return tmp++;
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
                j++;
            }      

            while(params->children[j] != NULL){
                int k = 0;
                printf("\n      [%s ", params->children[j]->ID);
                while(params->children[j]->children[k] != NULL){
                    printf(" [%s]", params->children[j]->children[k++]->ID);
                }
                printf("]");
                j++;
            }

            printf("]\n");

            compStmt = child->children[3];

            printCompStmt(compStmt);

            printf("  ]\n");

        }
        else if(Tree->children[i]->varDecl == 1){
            printVarDecl(Tree->children[i]);
        }
        
        i++;
    }
}


void printCompStmt(Node *compStmt){
    printf("    [%s\n", compStmt->ID);
    
    int j = 0;
    while(compStmt->children[j] != NULL){

        //printf("compStmt->children[%d] == %d\n",j, compStmt->children[j]->ID);

        if(compStmt->children[j]->assStmt == 1){
            //printf("      [%s\n", compStmt->children[j]->ID);
            printAssStmt(compStmt->children[j]);
        }
        else if(compStmt->children[j]->ifStmt == 1){
            printf("      [%s\n", compStmt->children[j]->ID);
            printIfStmt(compStmt->children[j]);
        }
        else if(compStmt->children[j]->varDecl == 1){
            printVarDecl(compStmt->children[j]);
        }
        else if(compStmt->children[j]->returnStmt == 1){
            //printf("printRetStmt\n");
            printRetStmt(compStmt->children[j]);
        }
        //printf("compstmt end\n");

        j++;
    }
    printf("       ]\n");

}

void printIfStmt(Node *ifStmt){
    // selection-stmt
    printf("        [%s ", ifStmt->children[0]->ID);
    // val

    printVar(ifStmt->children[0]->children[0]);

    // variable value
    printf(" [%s]", ifStmt->children[0]->children[1]->ID);
    printf("]\n");

    printCompStmt(ifStmt->children[1]);

    printf("    ]\n");
}

void printVar(Node *varNode){
    printf("[%s", varNode->ID);
    // variable id
    printf(" [%s]]" , varNode->children[0]->ID);
}

void printAssStmt(Node *assStmt){
    printf("      [%s ", assStmt->ID);
    // var
    printf("[%s", assStmt->children[0]->ID);
    // variable id
    printf(" [%s]" ,assStmt->children[0]->children[0]->ID);
    
    // if index print which one
    if(assStmt->children[0]->children[1] != NULL){
        printf(" [%s]]", assStmt->children[0]->children[1]->ID);
    }
    else
    {
        printf("]");
    }
    
    // could be a value or add/sub mult/div 
    if(assStmt->children[1]->simpleAddStmt == 1){
        printSimpleAdd(assStmt->children[1]);
    }
    else
        printf(" [%s]", assStmt->children[1]->ID);
    printf("]\n");
}

void printVarDecl(Node *varDecl){
    printf("  [var-declaration ");
    int j = 0;
    while(varDecl->children[j] != NULL){
        printf(" [%s]", varDecl->children[j++]->ID);
    }

    printf("]\n");
}

void printSimpleAdd(Node *addStmt){
            printf("\n        [%s ", addStmt->ID);

    if(addStmt->children[0]->simpleAddStmt != 1){
        if(addStmt->children[0]->children[0] == NULL)
            printf("[%s]", addStmt->children[0]->ID);
        else
            printf("[%s [%s]]", addStmt->children[0]->ID, addStmt->children[0]->children[0]->ID);
    }
    else{

        printSimpleAdd(addStmt->children[0]);
    }

    if(addStmt->children[1]->simpleAddStmt != 1){
        if(addStmt->children[1]->children[0] == NULL)
            printf("[%s]", addStmt->children[1]->ID);
        else
            printf("[%s [%s]]", addStmt->children[1]->ID, addStmt->children[1]->children[0]->ID);
    }
    else{
        printf("\n        [%s", addStmt->ID);

        printSimpleAdd(addStmt->children[1]);
    }
    printf("]");
}

void printRetStmt(Node *retStmt){
    printf("      [%s ", retStmt->ID);

    if(retStmt->children[0]->var == 1){
        printVar(retStmt->children[0]);
    }

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
