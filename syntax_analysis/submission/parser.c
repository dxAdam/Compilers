#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

// turn on/off for error printout
#define REPORT_ERROR 1

token_t tokens[1000];

// keep track of parens
int paren_count = 0;

void errorExit(char *e){
    if(REPORT_ERROR)
        printf("ERROR: %s\n", e);
    exit(1);
}

bool isVarDecl(int i){
    if(!strcmp(tokens[i].ID, "int")){
        if(!strcmp(tokens[i+2].ID, ";") || !(strcmp(tokens[i+2].ID, "[")))
            return true;
    }

    return false;
}

bool isFuncDecl(int i){
    if(!strcmp(tokens[i].ID, "void") && !strcmp(tokens[i+2].ID, "("))
        return true;
    if(!strcmp(tokens[i].ID, "int") && !strcmp(tokens[i+2].ID, "("))
        return true;

    return false;
}

bool isComparison(int i){
    if(!strcmp(tokens[i].ID, "=="))
        return true;
    if(!strcmp(tokens[i].ID, "!="))
        return true;
    if(!strcmp(tokens[i].ID, ">="))
        return true;
    if(!strcmp(tokens[i].ID, "<="))
        return true;
    if(!strcmp(tokens[i].ID, ">"))
        return true;
    if(!strcmp(tokens[i].ID, "<"))
        return true;

    return false;
}

bool isFunction(int i){
    if(tokens[i].type == T_ID && tokens[i+1].ID[0] == S_OPEN_PARENTHESIS){
        if(checkCloseParen(i))
            return true;
    }
    
    return false;
}

bool isOpSym(int i){
    if(tokens[i].ID[0] == S_PLUS)
        return true;
    if(tokens[i].ID[0] == S_HYPHEN)
        return true;
    if(tokens[i].ID[0] == S_SLASH)
        return true;
    if(tokens[i].ID[0] == S_ASTERISK)
        return true;

    return false;
}

bool checkCloseParen(int i){
    // if there's ")" before ";" then return true
    while(tokens[i].ID[0] != S_SEMICOLON){
        if(tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
            return true;
        i++;
    }
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

int newCompStmtNode(int i, Node *parent){
    int j = 0;
    j = new_child_node(parent);
    parent->children[j]->compStmt =1;
    parent->children[j]->ID = "compound-stmt";

    i = handleCompStmt(i, parent->children[j]);
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

    parent->children[j]->num = 1;
    parent->children[j]->ID = tokens[i].ID;

    return i;
}

int newOpNode(int i, Node *opNode){
    int tmp = i;
    int i_max;

    while(strcmp(tokens[tmp].ID, ";")) tmp++;

    i_max = tmp -1;

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


        // add what variable is = to 
        if(!strcmp(tokens[i+1].ID, ";")){
            int j2 = new_child_node(assNode);

            if(tokens[i].type == T_NUM)
                assNode->children[j2]->ID = tokens[i].num;
            else if(tokens[i].type == T_ID)
                assNode->children[j2]->ID = tokens[i].ID;
        }
        else if(isFunction(i)){
            i = newCallNode(i, assNode);
            i--;
        }
        else if(tokens[i+1].type == T_SYM){
            //printf("making new op node\n");
            if(tokens[i+1].ID[0] == S_EQUALS);
                i = newOpNode(i, assNode);
        }
            

        return ++i;   
}

int newEqlStmtNode(int i, Node *ifStmt){
    //printf("handling eql stmt\n");
    
    int j = 0;
    j = new_child_node(ifStmt);

    ifStmt->children[j]->ID = tokens[i+1].ID;
    ifStmt->children[j]->eqlStmt = 1;
    
    i = newVarNode(i, ifStmt->children[j]);

    // add variable value
    int j2; 
    j2 = new_child_node(ifStmt->children[j]);
    ifStmt->children[j]->children[j2]->ID = tokens[i+1].num;

    i+=3;

    return i;
}

int newIfNode(int i, Node *compStmt){
    int j = 0;
    j = new_child_node(compStmt);

    compStmt->children[j]->ID = "selection-stmt";
    compStmt->children[j]->ifWhileStmt = 1;

    i = handleIfStmt(i, compStmt->children[j]);

    return i;
}

int newWhileNode(int i, Node *compStmt){
    int j = 0;
    j = new_child_node(compStmt);

    compStmt->children[j]->ID = "iteration-stmt";
    compStmt->children[j]->ifWhileStmt = 1;

    i = handleWhileStmt(i, compStmt->children[j]);

    return i;
}

int newCallNode(int i, Node *compStmt){
    int j = 0;
    j = new_child_node(compStmt);

    compStmt->children[j]->ID = "call";
    compStmt->children[j]->callStmt = 1;

    Node *callNode = compStmt->children[j];

    // function name
    j = new_child_node(callNode);
    callNode->children[j]->ID = tokens[i].ID; // function ID

    // function args
    j = new_child_node(callNode);
    callNode->children[j]->ID = "args";

    i = handleCallArgs(i, callNode->children[j]);

    //printf("exiting newCallNode token : %s\n", tokens[i].ID);

    return i;
}

int newSemicolonNode(int i, Node *compStmt){
    int j = 0;
    j = new_child_node(compStmt);
    compStmt->children[j]->ID = tokens[i].ID; // should be ";"
    compStmt->children[j]->semicolon = 1;

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

    //printf("handleVarDecl for %s", tokens[i].ID);

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
        i+=3;
    }

    //else if(!strcmp(tokens[i].ID, "("))
    //    printf("var decl to function\n");



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

    // empty function case
    if(tokens[i+1].ID[0] == S_CLOSING_BRACE)
        return i;

    // handle compound stmt
    j = new_child_node(funcNode);
    funcNode->children[j]->ID = "compound-stmt";
    funcNode->children[j]->compStmt = 1;

    i++;

    i = handleCompStmt(i, funcNode->children[j]);

    return i;
}

int handleCallArgs(int i, Node *argNode){
    // skip function ID
    i++;

    //printf("hanlding call args token: %s\n", tokens[i].ID);
    
    //skip "("
    i++;
    
    while(tokens[i].ID[0] != S_SEMICOLON){
        // token should be ID or NUM
        if(tokens[i].type == T_NUM){
            i = newNumNode(i, argNode);
            //printf("added num node\n");
            i++;
        }
        else if(tokens[i].type == T_ID){
            i = newVarNode(i, argNode);
            //printf("added var node\n");
            i++;
        }
        else if(tokens[i].type == T_SYM){
            errorExit("handleCallArgs: Unexpected symbol");
            
        }
        else
            errorExit("handleCallArgs: Not ID or NUM");
    
        if(tokens[i+1].ID[0] != S_SEMICOLON){
            i++;
            if(tokens[i].ID[0] == S_COMMA)
                i++;
            else
                errorExit("handleCallArgs: missing comma");
        }
    }

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

        //printf("handleComplex token : %s    linenum: %d\n", tokens[i].ID, tokens[i].lineNumber);
        
        // an ID is an assignment operation or function call
        if(tokens[i].type == T_ID){
            if(!strcmp(tokens[i+1].ID, "=") || !strcmp(tokens[i+1].ID, "[")){
                //printf("! token id: %s\n", tokens[i].ID);
                i = newAssignNode(i, compStmt);
            }
            else if(isOpSym(i+1)){
                printf("detected op sym after ID");
                // op sym next
            }
            else if(isFunction(i))
                i = newCallNode(i, compStmt);
            else
                errorExit("handleCompStmt: Unknown ID stmt");
            
        }
        else if(tokens[i].type == T_KEY){
            if(isVarDecl(i)){
                i = newVarDeclNode(i, compStmt);
            }
            else if(!strcmp(tokens[i].ID, "if")){
                i = newIfNode(i, compStmt);
            }
            else if(!strcmp(tokens[i].ID, "return")){
                //printf("returnstmt\n");
                //while(strcmp(tokens[i].ID, ";"))i++;
                i = newReturnNode(i, compStmt);
            }   
            else if(!strcmp(tokens[i].ID, "while")){
                //printf("whilestmt\n");
                i = newWhileNode(i, compStmt);
            }
            else
            {
                printf("Compound Stmt unhandled key: %s\n", tokens[i].ID);
                exit(1);
            }
            
        }
        else if(tokens[i].type == T_SYM){
            if(tokens[i].ID[0] == S_SEMICOLON)
                i = newSemicolonNode(i, compStmt);
            else if(tokens[i].ID[0] == S_OPENING_BRACE)
            {
                i = newCompStmtNode(i, compStmt);
            }
        }
        else if(tokens[i].type == T_NUM){
            if(isOpSym(i+1)){
                printf("detected op sym after NUM\n");
                i = newOpNode(i, compStmt);
                i++;
            }
        }

        i++;
    }



    //printf("token[i].type : %s\n", tokens[i].ID);

    return i;
}

int handleIfStmt(int i, Node *ifStmtNode){
    i+=2;
    if(tokens[i].type == T_ID){
        if(isComparison(i+1)){
            i = newEqlStmtNode(i, ifStmtNode);
        }
        else errorExit("If stmt no comparison\n");
    }
    else errorExit("If stmt no ID type");

    i = newCompStmtNode(i, ifStmtNode);

    return i;
}

int handleWhileStmt(int i, Node *whileStmt){
    i+=2;
    if(tokens[i].type == T_ID){
        if(isComparison(i+1)){
            i = newEqlStmtNode(i, whileStmt);
        }
        else errorExit("While stmt no comparison\n");
    }
    else errorExit("While stmt no ID type");

    // handle while stmt
    i = newCompStmtNode(i, whileStmt);

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
    
    return tmp;
}

void printTree(FILE *opf, Node *Tree){

    Node *params;
    Node *child;
    Node *compStmt;

    if(Tree->root == 1)
        fprintf(opf, "[program\n");
    
    int i = 0;
    while(Tree->children[i] != NULL){
        child = Tree->children[i];
        if(child->funcDecl == 1){
            fprintf(opf, "  [fun-declaration\n");
            fprintf(opf, "    [%s]\n", child->children[0]->ID);
            fprintf(opf, "    [%s]\n", child->children[1]->ID);
            fprintf(opf, "    [params");

            params = child->children[2];
            int j = 0;

            if(!strcmp(params->children[j]->children[0]->ID, "void")){
                j++;
            }      

            while(params->children[j] != NULL){
                int k = 0;
                fprintf(opf, "\n      [%s ", params->children[j]->ID);
                while(params->children[j]->children[k] != NULL){
                    fprintf(opf, " [%s]", params->children[j]->children[k++]->ID);
                }
                fprintf(opf, "]");
                j++;
            }

            fprintf(opf, "]\n");

            fprintf(opf, "    [compound-stmt\n");
            if(child->children[3] != NULL){
                compStmt = child->children[3];
                printCompStmt(opf, compStmt);
            }
            fprintf(opf, "    ]\n  ]\n");

        }
        else if(Tree->children[i]->varDecl == 1){
            printVarDecl(opf, Tree->children[i]);
        }
        
        i++;
    }
    fprintf(opf, "]");
}

void printCompStmt(FILE *opf, Node *compStmt){
    
    int j = 0;
    while(compStmt->children[j] != NULL){

        //fprintf(opf, "compStmt->children[%d] == %d\n",j, compStmt->children[j]->ID);

        if(compStmt->children[j]->assStmt == 1){
            //fprintf(opf, "      [%s\n", compStmt->children[j]->ID);
            printAssStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->ifWhileStmt == 1){
            fprintf(opf, "      [%s\n", compStmt->children[j]->ID);
            printIfStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->varDecl == 1){
            printVarDecl(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->returnStmt == 1){
            //fprintf(opf, "printRetStmt\n");
            printRetStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->callStmt == 1){
            printCallStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->semicolon == 1){
            fprintf(opf, "      [%s]\n", compStmt->children[j]->ID);
        }
        else if(compStmt->children[j]->compStmt == 1){
            fprintf(opf, "      [compound-stmt");
            printCompStmt(opf, compStmt->children[j]);
            fprintf(opf, "\n     ]\n");
        }
        else if(compStmt->children[j]->simpleAddStmt == 1){
            printSimpleAdd(opf, compStmt->children[j]);
        }
        //fprintf(opf, "compstmt end\n");

        j++;
    }

}

void printIfStmt(FILE *opf, Node *ifStmt){
    // selection-stmt
    fprintf(opf, "        [%s ", ifStmt->children[0]->ID);
    // val

    printVar(opf, ifStmt->children[0]->children[0]);

    // variable value
    fprintf(opf, " [%s]", ifStmt->children[0]->children[1]->ID);
    fprintf(opf, "]\n");

    printCompStmt(opf, ifStmt->children[1]);

    fprintf(opf, "    ]\n");
}

void printCallStmt(FILE *opf, Node *callStmt){
    fprintf(opf, "        [%s\n", callStmt->ID);
    fprintf(opf, "          [%s] \n", callStmt->children[0]->ID);

    Node *argNode = callStmt->children[1];
    
    fprintf(opf, "          [%s", argNode->ID);

    int j = 0;
    while(argNode->children[j] != NULL){
        if(argNode->children[j]->var == 1){
            fprintf(opf, " ");
            printVar(opf, argNode->children[j]);
        }
        else
            fprintf(opf, " [%s]", argNode->children[j]->ID);
        j++;
    }

    fprintf(opf, "]\n");
}

void printVar(FILE *opf, Node *varNode){
    fprintf(opf, "[%s", varNode->ID);
    // variable id
    fprintf(opf, " [%s]" , varNode->children[0]->ID);
    // index
    if(varNode->children[1] != NULL)
        fprintf(opf, " [%s]", varNode->children[1]->ID);

    fprintf(opf, "]");
}

void printAssStmt(FILE *opf, Node *assStmt){
    fprintf(opf, "      [%s ", assStmt->ID);
    // var
    fprintf(opf, "[%s", assStmt->children[0]->ID);
    // variable id
    fprintf(opf, " [%s]" ,assStmt->children[0]->children[0]->ID);
    
    // if index print which one
    if(assStmt->children[0]->children[1] != NULL){
        fprintf(opf, " [%s]]", assStmt->children[0]->children[1]->ID);
    }
    else
    {
        fprintf(opf, "]");
    }
    
    // could be a value or add/sub mult/div 
    if(assStmt->children[1]->simpleAddStmt == 1){
        printSimpleAdd(opf, assStmt->children[1]);
    }
    else if(assStmt->children[1]->callStmt == 1){
            fprintf(opf, "\n");
            printCallStmt(opf, assStmt->children[1]);
            fprintf(opf, "        ]");
    }
    else
        fprintf(opf, " [%s]", assStmt->children[1]->ID);
    fprintf(opf, "]\n");
}

void printVarDecl(FILE *opf, Node *varDecl){
    fprintf(opf, "  [var-declaration ");
    int j = 0;
    while(varDecl->children[j] != NULL){
        fprintf(opf, " [%s]", varDecl->children[j++]->ID);
    }

    fprintf(opf, "]\n");
}

void printSimpleAdd(FILE *opf, Node *addStmt){
            fprintf(opf, "\n        [%s ", addStmt->ID);

    if(addStmt->children[0]->simpleAddStmt != 1){
        if(addStmt->children[0]->children[0] == NULL)
            fprintf(opf, "[%s]", addStmt->children[0]->ID);
        else
            fprintf(opf, "[%s [%s]]", addStmt->children[0]->ID, addStmt->children[0]->children[0]->ID);
    }
    else{

        printSimpleAdd(opf, addStmt->children[0]);
    }

    if(addStmt->children[1]->simpleAddStmt != 1){
        if(addStmt->children[1]->children[0] == NULL)
            fprintf(opf, "[%s]", addStmt->children[1]->ID);
        else
            fprintf(opf, "[%s [%s]]", addStmt->children[1]->ID, addStmt->children[1]->children[0]->ID);
    }
    else{
        fprintf(opf, "\n        [%s", addStmt->ID);

        printSimpleAdd(opf, addStmt->children[1]);
    }
    fprintf(opf, "]");
}

void printRetStmt(FILE *opf, Node *retStmt){
    fprintf(opf, "      [%s ", retStmt->ID);

    if(retStmt->children[0]->var == 1){
        printVar(opf, retStmt->children[0]);
    }

    fprintf(opf, "]\n");
}

void some_syn_checks(int imax){
    int paren_count   = 0;
    int bracket_count = 0;
    int brace_count   = 0;

    int i;
    for(i=0; i<imax; i++){
        if(tokens[i].ID[0] == S_OPEN_PARENTHESIS)
            paren_count++;
        else if(tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
            paren_count--;
        else if(tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
            paren_count--;
        else if(tokens[i].ID[0] == S_OPENING_BRACE)
            brace_count++;
        else if(tokens[i].ID[0] == S_CLOSING_BRACE)
            brace_count--;
        else if(tokens[i].ID[0] == S_OPENING_BRACKET)
            bracket_count++;
        else if(tokens[i].ID[0] == S_CLOSING_BRACKET)
            bracket_count--;
    }

    if(paren_count != 0)
        errorExit("Uneven parens");
    
    if(brace_count != 0)
        errorExit("Uneven braces");
    
    if(bracket_count != 0)
        errorExit("Uneven brackets");

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

    some_syn_checks(i);

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

    printTree(ofp, Tree);

    printf("\n-- exiting program normally\n");
}