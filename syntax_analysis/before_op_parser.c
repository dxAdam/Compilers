/*
    program: Top down parser for C- language
    dependencies: parser.h, lex.c, lex.h, tree.h, tree.c

    compilation: gcc -o parser parser.c lex.c tree.c

    usage: ./parser <input_file> <output_file>

    date: March 2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser.h"


#define REPORT_ERROR            1   // error printout through errorExit()
#define DEBUG                   1   // general debugging printouts through printf()
#define DEBUG_FUNC_ENTER        1   // function entrance debugging printouts through printf()
#define DEBUG_FUNC_EXIT         1   // function exit debugging printouts through printf()

token_t tokens[1000];

/*  NOTES:       ^-------------
                              |
    parameter i == tokens[] index in all functions below
*/



/**** *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
    utility functions 
*/

void errorExit(char *e, int i){ // pass i == -1 if no token printout desired
    if(REPORT_ERROR)
        printf("ERROR %s\n-----  line:  %d\n", e, tokens[i].lineNumber);
    if(i != -1)
        printf("----- token: %s\n", tokens[i].ID);
    exit(1);
}

bool isVarDecl(int i){
    if(!strcmp(tokens[i].ID, "int")){
        if(!strcmp(tokens[i+2].ID, ";") || !(strcmp(tokens[i+2].ID, "[")))
            return true;
    }
    // expand for future supported variable types

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
    if(tokens[i].ID[0] == S_ADD)
        return true;
    if(tokens[i].ID[0] == S_SUB)
        return true;
    if(tokens[i].ID[0] == S_MUL)
        return true;
    if(tokens[i].ID[0] == S_DIV)
        return true;

    return false;
}

bool checkCloseParen(int i){ // only handles very simple cases for now

    while(tokens[i].ID[0] != S_SEMICOLON){
        if(tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
            return true;
        i++;
    }
    return false;

    // true == detects ')' before ';'
    // false otherwise
}


/*** *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
    new node functions 
*/

int newVarDeclNode(int i, Node *Tree){
    if(DEBUG_FUNC_ENTER)printf("20 newVarDeclNode - token: %s\n", tokens[i].ID);

    int j = 0;
    j = new_child_node(Tree);

    Tree->children[j]->varDecl = 1;

    i = handleVarDecl(i, Tree->children[j]);

    if(DEBUG_FUNC_EXIT)printf("20 newVarDeclNode exit - token: %s\n", tokens[i].ID);
    return i;
}

int newFuncDeclNode(int i, Node *Tree){
    if(DEBUG_FUNC_ENTER)printf("19 newFuncDeclNode - token: %s\n", tokens[i].ID);

    int j=0;
    j = new_child_node(Tree);
    
    Tree->children[j]->funcDecl = 1;
    Tree->children[j]->func = 1;

    i = handleFuncDecl(i, Tree->children[j]);

    if(DEBUG_FUNC_EXIT)printf("19 newFuncNode exit - token: %s\n", tokens[i].ID);
    return  i;
}

int newCompStmtNode(int i, Node *parent){
    if(DEBUG_FUNC_ENTER)printf("newCompStmtNode - token: %s\n", tokens[i].ID);
    
    // skip '{'
    if(tokens[i++].ID[0] != S_OPENING_BRACE)
        errorExit("18 newCompStmtNode - expected '{' but got", i);
    

    int j = 0;
    j = new_child_node(parent);

    parent->children[j]->compStmt = 1;
    parent->children[j]->ID = "compound-stmt";

    i = handleCompStmt(i, parent->children[j]);

    if(DEBUG_FUNC_EXIT)printf("18 newCompStmtNode exit - token: %s\n", tokens[i].ID);
    return i;
}

int newVarNode(int i, Node *parent){ // var with value following assumed
    int j = 0;
    
    // creates node "var"
    j = new_child_node(parent);
    parent->children[j]->var = 1;
    parent->children[j]->ID = "var";

    // add child node for variable ID
    int j2 = new_child_node(parent->children[j]);

    if(DEBUG)printf("17 newVarNode - adding token: %s\n", tokens[i].ID);

    // add variable ID
    parent->children[j]->children[j2]->ID = tokens[i].ID;
    parent->children[j]->children[j2]->var = 1;
   
    // check if array - if so theres an additional node that needs created
    if(!strcmp(tokens[i+1].ID, "[")){
        //if(DEBUG)printf("newVarNode: deteced that %s is an array w/ index %s\n", tokens[i].ID, tokens[i+2].num);
        j2 = new_child_node(parent->children[j]);
        if(tokens[i+2].type == T_NUM)
            parent->children[j]->children[j2]->ID = tokens[i+2].num;
        else
            parent->children[j]->children[j2]->ID = tokens[i+2].ID;
        
        i+=3; // skip [num] symbols
    }

    i++;
    if(DEBUG)printf("17 newVarNode exit - token: %s\n", tokens[i].ID);
    return i;
}

int newNumNode(int i, Node *parent){
    if(DEBUG_FUNC_ENTER)printf("16 newNumNode - token: %s\n", tokens[i].ID);

    int j = 0;
    j = new_child_node(parent);

    parent->children[j]->num = 1;
    parent->children[j]->ID = tokens[i].ID;

    if(DEBUG)printf("newNumNode - adding token: %s\n", tokens[i].ID);

    i++;

    if(DEBUG_FUNC_EXIT)printf("16 newNumNode exit - token: %s\n", tokens[i].ID);
    return i;
}

int newAssignNode(int i, Node *parent){
    if(DEBUG_FUNC_ENTER)printf("14 newAssignNode - token: %s\n", tokens[i].ID);
        int j = 0;

        // create "=" node
        j = new_child_node(parent);
        parent->children[j]->assStmt = 1;
        parent->children[j]->ID = "=";

        // declare new assignment node variable for easier reading
        Node *assNode = parent->children[j];

        // create node for ID/NUM before '='
        if(tokens[i].type == T_ID)
            i = newVarNode(i, assNode);
        else if(tokens[i].type == T_NUM)
            i = newNumNode(i, assNode);
        else errorExit("newAssignNode - unhandled type before '='", i);

        if(tokens[i++].ID[0] != S_EQUALS) // calling function likely already checked
            errorExit("newAssignNode - '=' symbol expected", i);

        if(tokens[i].ID[0] == S_OPEN_PARENTHESIS)
            if(!checkCloseParen(i++)) // skip open paren for now  TO DO
                errorExit("newAssignNode - no close paren", i);


        // add what variable is assigned to 
        if(!strcmp(tokens[i+1].ID, ";")){             // simple assignment  ex: a = 1;
            if(tokens[i].type == T_ID)
                i = newVarNode(i, assNode);
            else if(tokens[i].type == T_NUM)
                i = newNumNode(i, assNode);
            else errorExit("newAssignNode - unknown token before ';'", i);
        }
        else if(isFunction(i)){                       // assignment to function
            i = newCallNode(i, assNode);
            i--; // TO DO - see if this i-- can be remove
        }
        else if(isComparison(i+1) || isOpSym(i+1)){   // assignment to operation/comparison
                i = newOpNode(i+1, assNode);
        }
        else errorExit("newAssignmentNode - unknown symbol after var/num", i);
        
    //i++;
    if(DEBUG_FUNC_EXIT)printf("14 newAssignNode exit - token: %s\n", tokens[i].ID);
    return i;   
}

int newEqlStmtNode(int i, Node *controlStmt){
    if(DEBUG_FUNC_ENTER)printf("13 newEqlStmtNode - token: %s\n", tokens[i].ID);
    int j = 0;

    // add comparison symbol (==, !=, <, etc...)   
    j = new_child_node(controlStmt);
    controlStmt->children[j]->ID = tokens[i+1].ID;
    controlStmt->children[j]->eqlStmt = 1;

    // add var/num nodes
    if(tokens[i].type == T_ID)
        i = newVarNode(i, controlStmt->children[j]);
    else if(tokens[i].type == T_NUM)
        i = newNumNode(i, controlStmt->children[j]);
    else if(tokens[i].ID[0] == S_OPEN_PARENTHESIS || tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
        i++; // skip parens for now
    else
        errorExit("newEqlStmtNode - unknown token in place of var/num", i);

    // skip 
    i++;

    if(tokens[i].type == T_ID)
        i = newVarNode(i, controlStmt->children[j]);
    else if(tokens[i].type == T_NUM)
        i = newNumNode(i, controlStmt->children[j]);
    else if(tokens[i].ID[0] == S_OPEN_PARENTHESIS || tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
        i++; // skip parens for now
    else
        errorExit("newEqlStmtNode - unknown token in place of var/num", i);

    if(DEBUG_FUNC_EXIT)printf("13 newEqlStmtNode exit - token: %s\n", tokens[i].ID);
    return i;
}

int newControlNode(int i, Node *compStmt){
    if(DEBUG_FUNC_ENTER)printf("12 newControlNode - token: %s\n", tokens[i].ID);
    int  j = 0;

    // add "selection-stmt" or "iteration-stmt" node
    j = new_child_node(compStmt);
    compStmt->children[j]->controlStmt = 1;
    if(!strcmp(tokens[i].ID, "if"))
        compStmt->children[j]->ID = "selection-stmt";
    else if(!strcmp(tokens[i].ID, "while"))
        compStmt->children[j]->ID = "iteration-stmt";
    else errorExit("newControlNode - unknown control statment", i);

    i = handleControlStmt(i, compStmt->children[j]);

    if(DEBUG_FUNC_EXIT)printf("12 newControlNode exit - token: %s\n", tokens[i].ID);
    return i;
}

int newCallNode(int i, Node *compStmt){
    if(DEBUG_FUNC_ENTER)printf("11 newCallNode - token: %s\n", tokens[i].ID);

    int j = 0;
    j = new_child_node(compStmt);

    compStmt->children[j]->ID = "call";
    compStmt->children[j]->callStmt = 1;

    Node *callNode = compStmt->children[j];

    // function name
    j = new_child_node(callNode);
    callNode->children[j]->ID = tokens[i].ID; // function ID

    // function args if not void
    if(strcmp(tokens[i].ID, "void")){
        j = new_child_node(callNode);
        callNode->children[j]->ID = "args";
        i = handleCallArgs(i, callNode->children[j]);
    }

    if(DEBUG_FUNC_EXIT)printf("11 newCallNode - token: %s\n", tokens[i].ID);
    return i;
}

int newSemicolonNode(int i, Node *compStmt){
    int j = 0;
    j = new_child_node(compStmt);
    compStmt->children[j]->ID = tokens[i].ID; // should be ";"
    compStmt->children[j]->semicolon = 1;

    return ++i;
}

int newReturnNode(int i, Node *compStmt){
    if(DEBUG_FUNC_ENTER)printf("10 newReturnlNode - token: %s\n", tokens[i].ID);
    int j=0;

    // add "return-stmt" node
    j = new_child_node(compStmt);
    Node * retNode = compStmt->children[j];
    retNode->returnStmt = 1;
    retNode->ID = "return-stmt";


    // see if return statement is simple or compound statement  TO DO - might be able to turn this into common function
    if(!strcmp(tokens[i+2].ID, ";")){
            i++;
            if(tokens[i].type == T_NUM)
                i = newNumNode(i, retNode);
            else if(tokens[i].type == T_ID)
                i = newVarNode(i, retNode);
            else errorExit("newReturnNode - unexpected token", i);  
    }

    if(DEBUG_FUNC_EXIT)printf("10 newReturnlNode exit - token: %s\n", tokens[i].ID);
    return i;
}


/*** *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
    operator functions
*/






int newOpNode(int i, Node *parent){
    printf("newOpNode token: %s", tokens[i].ID);
    int j = 0;

    if(tokens[i].ID[0] == S_OPEN_PARENTHESIS)
        ; //paren case




    return i;
}

Node * factor(int i){
    Node *node;

    if(tokens[i].type == T_NUM){
        node = get_zeroed_node();
        node->ID = tokens[i].ID;
        node->num = 1;
    }
    else if(tokens[i].ID[0] == S_OPEN_PARENTHESIS){
        node = expr(i);
    }

    return node;
}

Node * term(int i){
    Node* node = factor(i);

    i++;
    while(tokens[i].ID[0] == S_MUL || tokens[i].ID[0] == S_DIV){
        if(tokens[i].ID[0] == S_MUL)
            node->mul = 1;   //    '*'
        else
            node->div = 1;   //    '\'

        
    }

}


Node * expr(int i){

    Node *term = (Node *)malloc(sizeof(Node));

    char sym = tokens[i].ID[0];   //  "+", "-", "\", "*"

    if(sym == S_ADD)
        ;
    else if(sym == S_SUB)
        ;
    else if(sym == S_SUB)
        ;
    else if(sym == S_SUB)
        ;

    return term;
}







/*** *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
    handle functions
*/

int handleVarDecl(int i, Node *parent){
 /* since the only supported variable type in C- is "int" we can take some shortcuts
      -- function could be generalized for more variables types
 */
    if(DEBUG_FUNC_ENTER)printf("00 handleVarDecl - token: %s\n", tokens[i].ID);
    int j = 0;

    if(!strcmp(tokens[i].ID, "void")) errorExit("handleVarDecl - \"void\" passed to function", i);

    // add "int" node
    j = new_child_node(parent);
    parent->children[j]->ID = tokens[i].ID; // should be "int"  

    // if not "int" error
    if(strcmp(tokens[i].ID, "int")) errorExit("handleVarDecl - expected \"int\" but got", i);

    i++;

    // add variable ID
    j = new_child_node(parent);
    parent->children[j]->ID = tokens[i].ID;

    if(DEBUG)printf("handleVarDecl - put %s in node\n", tokens[i].ID);


    // TO DO - below allows the declaration of variables to simple varNodes only

    // check if array is simple array declaration "a[]" (function paramter)
    if(tokens[i+1].ID[0] == S_OPENING_BRACKET && tokens[i+2].ID[0] == S_CLOSING_BRACKET){    // add variable ID
        j = new_child_node(parent);
        parent->children[j]->ID = "\\[\\]";
        i+=2;
    }
    else if(tokens[i+1].ID[0] ==  S_OPENING_BRACKET){ // array is more complex array declaration "a[10]"
        i+=3;
        j = new_child_node(parent);
        parent->children[j]->ID = tokens[i].num; // size of array
    }

    // skip to ';' in case of normal declaration or ',' in case of function args
    if(!(tokens[++i].ID[0] != S_SEMICOLON || tokens[i].ID[0] != S_COMMA))
        errorExit("handleVarDecl - expected ';' or ',' but got", --i);

    i++;
    if(DEBUG_FUNC_EXIT)printf("00 handleVarDecl exit - token: %s\n", tokens[i].ID);
    return i;
}

int handleFuncDecl(int i, Node *funcNode){
    if(DEBUG_FUNC_ENTER)printf("01 handleFuncDecl - token: %s\n", tokens[i].ID);
    int j = 0;

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

    // skip '('
    i++;

    // if argument is not "void" call handleFucnParams()
    if(strcmp(tokens[i].ID, "void"))
        i = handleFuncParams(i, funcNode->children[j]);

    // should be "void" or last token in function arguments before ')'
    //i++;

    // skip ')'
    //if(!(tokens[i++].ID[0] != S_CLOSE_PARENTHESIS || tokens[i].ID[0] != S_COMMA))
    //    errorExit("handleFuncDecl - expecting ')' or ',' but got", i);

    if(tokens[i].ID[0] == S_OPENING_BRACE)
        i = newCompStmtNode(i, funcNode);
    else errorExit("handleFuncDecl -- no compound statment after function", i);

    if(DEBUG_FUNC_ENTER)printf("01 handleFuncDecl exit - token: %s\n", tokens[i].ID);
    return i;
}

int handleCallArgs(int i, Node *argNode){
    if(DEBUG_FUNC_ENTER)printf("handleCallArgs exit - token: %s\n", tokens[i].ID);
    
    // skip function ID
    i++;
    
    //skip "("
    if(tokens[i++].ID[0] != S_OPEN_PARENTHESIS)
        errorExit("handleCallArgs - No open paren after function name", i);
    
    // while token is not ';' iterate through them, verifying commas separate the passed args
    // TO DO int comma_flip = 0; // flip this bit to verify a comma comes ever other iteration
    while(tokens[i].ID[0] != S_SEMICOLON){
        //printf("while token: %s\n", tokens[i].ID);
        while(tokens[i].ID[0] == S_OPEN_PARENTHESIS || tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
            i++; //skip parens for now

        // token should be ID or NUM, comma ','
        if(tokens[i].type == T_NUM){
            i = newNumNode(i, argNode);
        }
        else if(tokens[i].type == T_ID){
            i = newVarNode(i, argNode);
        }
        else if(tokens[i].type == T_SYM){
            errorExit("handleCallArgs - unexpected symbol", i);
            // TO DO handle complex statement
        }         
        else errorExit("handleCallArgs - not ID or NUM", i);
    
        while(tokens[i].ID[0] == S_OPEN_PARENTHESIS || tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
            i++; // skip parens again

        // above statements increment i so now check if we reached either ');' or another ','
        //   -- we don't just check for ')' in case parens are part of the statement
        if(tokens[i].ID[0] != S_SEMICOLON){
            if(tokens[i].ID[0] == S_COMMA)
                i++;
            else errorExit("handleCallArgs - missing comma", i);
        }
    }


    if(DEBUG_FUNC_EXIT)printf("handleCallArgs exit - token: %s\n", tokens[i].ID);
    return i;
}

int handleFuncParams(int i, Node *params){
    if(DEBUG_FUNC_ENTER)printf("03 handleFuncParams enter - token: %s\n", tokens[i].ID);
        
    int j = 0;
    while(!strcmp(tokens[i].ID, "int")){
        // create param node
        j = new_child_node(params);
        params->children[j]->ID = "param";
        
        // add var decl to param node
        i = handleVarDecl(i, params->children[j]);
    }

    if(DEBUG_FUNC_ENTER)printf("03 handleFuncParam exit - token: %s\n", tokens[i].ID);
    return i;
}

int handleCompStmt(int i, Node *compStmt){
    if(DEBUG_FUNC_ENTER)printf("handleCompStmt - token: %s\n", tokens[i].ID);    
    bool stmtDeclared = false;  // used to assure declarations do not come after statements

    while((tokens[i].ID[0] != S_CLOSING_BRACE) && (tokens[i].lineNumber != 0)){

        if(DEBUG)printf("handleCompStmt loop token : %s    linenum: %d\n", tokens[i].ID, tokens[i].lineNumber);
        
        // an ID is an assmignment operation or function call
        if(tokens[i].type == T_ID){
            stmtDeclared = true;

            if(!strcmp(tokens[i+1].ID, "=") || !strcmp(tokens[i+1].ID, "[")){
                i = newAssignNode(i, compStmt);
            }
            else if(isOpSym(i+1)){
                i = newOpNode(i, compStmt);
                i++;
            }
            else if(isFunction(i))
                i = newCallNode(i, compStmt);
            else errorExit("handleCompStmt: Unknown ID stmt", i);

        }
        else if(tokens[i].type == T_KEY){
            if(isVarDecl(i)){
                if(stmtDeclared == false)
                    i = newVarDeclNode(i, compStmt);
                else errorExit("handleCompStmt - var declared after statement", i);
            }
            else if(!strcmp(tokens[i].ID, "if")){
                i = newControlNode(i, compStmt);
            }
            else if(!strcmp(tokens[i].ID, "return")){
                i = newReturnNode(i, compStmt);
            }   
            else if(!strcmp(tokens[i].ID, "while")){
                i = newControlNode(i, compStmt);
            }
            else errorExit("handleCompStmt - unhandled key", i);
            
        }
        else if(tokens[i].type == T_SYM){
            stmtDeclared = true;

            if(tokens[i].ID[0] == S_SEMICOLON)
                i = newSemicolonNode(i, compStmt);
            else if(tokens[i].ID[0] == S_OPENING_BRACE){
                i = newCompStmtNode(i, compStmt);
            }
        }
        else if(tokens[i].type == T_NUM){
            stmtDeclared = true;

            if(tokens[i+1].ID[0] == '=')
                errorExit("handleCompStmt - assign to num", i);
            else if(isOpSym(i+1)){
                //printf("detected op sym after NUM\n");
                i = newOpNode(i, compStmt);
                i++;
            }
            else if(tokens[i+1].ID[0] == ';'){
                i = newNumNode(i, compStmt);
                i++;
            }
        }

        i++;
    }

    if(DEBUG_FUNC_EXIT)printf("handleCompStmt exit - token: %s\n", tokens[i].ID);    
    return i; // return '}'
}

int handleControlStmt(int i, Node *controlNode){
    if(DEBUG_FUNC_ENTER)printf("handleControlStmt - token: %s\n", tokens[i].ID);    
    
    //skip if/while and '('
    i+=2;

    // see if token[i] is an ID/NUM and token[i+1] is a comparison
    if(tokens[i].type == T_ID || tokens[i].type == T_NUM){
        if(isComparison(i+1)){
            i = newEqlStmtNode(i, controlNode);
        }
        else errorExit("handleControlStmt - no comparison symbol", i);
    }
    else  errorExit("handleControlStmt - no ID type", i);
    
    // skip ')'
    if(tokens[i++].ID[0] != S_CLOSE_PARENTHESIS)
        errorExit("handleControlStmt - expecting ')' but got", i);

    // current token should be '{' signaling the start of a compound statement
    if(tokens[i].ID[0] == S_OPENING_BRACE){
        i = newCompStmtNode(i, controlNode);       
    }
    else errorExit("handleControlStmt - expecting '{' but got", i);

    // should be returning ')' if no compound statment. We check
    //  for '{' above, if it exists we should call newCompStmt() then return '}' 
    if(DEBUG_FUNC_EXIT)printf("handleControlStmt exit - token: %s\n", tokens[i].ID);
    return i;
}

int handleSimpleAddOp(int i, Node *parent){
    if(DEBUG_FUNC_ENTER)printf("handleSimpleAddOp - token: %s\n", tokens[i].ID);    
    int j = 0;

    // make Op node
    j = new_child_node(parent);   
    Node *opNode = parent->children[j];
    opNode->op = 1;
    opNode->ID = tokens[i+1].ID; // should be +, -, etc..

    // callers to simple add op should have verified
    if(tokens[i].type == T_NUM)
        i = newNumNode(i, opNode);
    else if(tokens[i-1].type == T_ID)
        i = newVarNode(i-1, parent->children[j]);

    // skip op symbol
    i++;

    if(tokens[i].type == T_NUM)
        i = newNumNode(i, opNode);
    else if(tokens[i].type == T_ID)
        newVarNode(i, opNode);

    if(DEBUG_FUNC_EXIT)printf("handleSimpleAddOp exit - token: %s\n", tokens[i].ID);   
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

            if(child->children[3] != NULL && child->children[3]->compStmt == 1){
                compStmt = child->children[3];
                printCompStmt(opf, compStmt);
            }

            fprintf(opf, "  ]\n");

        }
        else if(Tree->children[i]->varDecl == 1){
            printVarDecl(opf, Tree->children[i]);
        }
        
        i++;
    }
    fprintf(opf, "]\n");
}

void printCompStmt(FILE *opf, Node *compStmt){
    fprintf(opf, "    [compound-stmt\n"); // compound-stmt opening bracket

    int j = 0;
    while(compStmt->children[j] != NULL){
        if(compStmt->children[j]->assStmt == 1){
            printAssStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->controlStmt == 1){
            printControlStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->varDecl == 1){
            printVarDecl(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->returnStmt == 1){
            printRetStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->callStmt == 1){
            printCallStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->semicolon == 1){
            fprintf(opf, "      [%s]\n", compStmt->children[j]->ID);
        }
        else if(compStmt->children[j]->compStmt == 1){
            if(DEBUG)printf("printCompStmt - printCompStmt()\n");
            printCompStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->simpleAddStmt == 1){
            printSimpleAdd(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->num == 1)
            fprintf(opf, "      [%s]\n", compStmt->children[j]->ID);

        j++;
    }

    fprintf(opf, "    ]\n"); // compound stmt closing bracket
}

void printControlStmt(FILE *opf, Node *ifStmt){
    // control stmt opening bracket and type (selection-stmt/iteration-stmt)
    fprintf(opf, "      [%s\n", ifStmt->ID); 

    // comparison symbol (==, !=, >, etc...)
    fprintf(opf, "        [%s ", ifStmt->children[0]->ID);

    // print variable ID if var type or assume it's a num
    if(ifStmt->children[0]->children[0]->var == 1)                   // 1st
        printVar(opf, ifStmt->children[0]->children[0]);
    else
        fprintf(opf, "[%s]", ifStmt->children[0]->children[0]->ID);
    
    if(ifStmt->children[0]->children[1]->var == 1)                   // 2nd
        printVar(opf, ifStmt->children[0]->children[1]);
    else if(ifStmt->children[0]->children[1]->num == 1)
        fprintf(opf, "[%s]", ifStmt->children[0]->children[1]->ID);
    else{
        /* TO DO - handle case that control expression is a complex statment
            or unknown symbol.  Currently only simple ops work */
        errorExit("printControlStmt - unknown node type", -1);
    }

    fprintf(opf, "]\n"); // closing assignment bracket

    // if control statment has a compound statement tied to it print now
    if(ifStmt->children[1]->compStmt == 1){
        printf("printing comp from control stmt\n");
        printCompStmt(opf, ifStmt->children[1]);
    }

    fprintf(opf, "      ]\n");
}

void printCallStmt(FILE *opf, Node *callStmt){
    fprintf(opf, "        [%s\n", callStmt->ID);
    fprintf(opf, "          [%s] \n", callStmt->children[0]->ID);

    Node *argNode = callStmt->children[1];
    
    fprintf(opf, "          [%s ", argNode->ID); // "[args "

    int j = 0;
    while(argNode->children[j] != NULL){
        if(argNode->children[j]->var == 1){
            printVar(opf, argNode->children[j]);
        }
        else
            fprintf(opf, " [%s] ", argNode->children[j]->ID);
        j++;
    }

    fprintf(opf, "]\n");
}

void printVar(FILE *opf, Node *varNode){
    // should already have been verified this is a var node (Node->var == 1)

    if(varNode->var != 1)
        errorExit("printVar - expected var node", -1);

    // print "var"
    fprintf(opf, "[%s", varNode->ID); //var opening bracket
    // variable id
    fprintf(opf, " [%s]" , varNode->children[0]->ID);
    // index if array
    if(varNode->children[1] != NULL)
        fprintf(opf, " [%s]", varNode->children[1]->ID);

    fprintf(opf, "]"); // var closing bracket
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
    fprintf(opf, "]");

    
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
    fprintf(opf, "      [%s", retStmt->ID);

    if(retStmt->children[0]->var == 1){
        printVar(opf, retStmt->children[0]);
    }

    fprintf(opf, "]\n");
}

void some_syn_checks(int imax){
    int paren_count   = 0;
    int bracket_count = 0;
    int brace_count   = 0;
    
    char *tmp;

    if(imax == 0)
        errorExit("empty program", -1);

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

        // check if any numbers larger than 2^16 have been used
        if(tokens[i].type == T_NUM){
            if(strtol(tokens[i].num, &tmp, 10) >= 2147483648){
                errorExit("syn checks - int too large", -1);
            }
        }
    }

    if(paren_count != 0)
        errorExit("Uneven parens", -1);
    
    if(brace_count != 0)
        errorExit("Uneven braces", -1);
    
    if(bracket_count != 0)
        errorExit("Uneven brackets", -1);

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

    while(j < i){ // i is number of tokens

        if(isFuncDecl(j)){
            j = newFuncDeclNode(j, Tree);
            if(DEBUG)printf("finished declaring function - token: %s\n", tokens[i].ID);
        }
        else if(isVarDecl(j)){
            j = newVarDeclNode(j, Tree);
            if(DEBUG)printf("finished declaring var - token: %s\n", tokens[i].ID);

        }
        else
        {
            printf("Main - unknown stmt - token: %s\n", tokens[j].ID);
            errorExit("Main - unknown stmt", j);
        }
    }

    printTree(ofp, Tree);

    printf("\n-- exiting program normally\n");
}