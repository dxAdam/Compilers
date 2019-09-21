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

#include "parser.h"
#include "symtable.h"


#define REPORT_ERROR            0   // error printout through errorExit()
#define DEBUG                   0   // general debugging printouts through printf()
#define DEBUG_FUNC_ENTER        0   // function entrance debugging printouts through printf()
#define DEBUG_FUNC_EXIT         0   // function exit debugging printouts through printf()

token_t tokens[1000];

/*  NOTES:       ^-------------
                              |
    parameter i == tokens[] index in all functions below
*/


/**** *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
    utility functions 
*/

void errorExit(char *e, int i){ 
    // pass i == -1 if no token printout available\desired
    
    if(REPORT_ERROR){
        printf("ERROR %s\n----- line:  %d\n", e, tokens[i].lineNumber);
        if(i != -1)
            printf("----- token: %s\n", tokens[i].ID);
    }
    exit(1);
}

bool isVarDecl(int i){
    if(!strcmp(tokens[i].ID, "int")){
        if(tokens[i+3].ID[0] == S_CLOSING_BRACKET)
            errorExit("isVarDecl - empty index", i+3);
        else if(!strcmp(tokens[i+2].ID, ";") || !(strcmp(tokens[i+2].ID, "[")))
            return true;
    }
    // expand for future supported variable types

    return false;
}

bool isFuncDecl(int i){
    if( (!strcmp(tokens[i].ID, "void") && tokens[i+1].type == T_ID && !strcmp(tokens[i+2].ID, "("))
     || (!strcmp(tokens[i].ID, "int") &&  tokens[i+1].type == T_ID && !strcmp(tokens[i+2].ID, "(")) ){

        return true;
    }

    return false;
}

bool isAssignment(int i){
    while(tokens[i].ID[0] != S_SEMICOLON){
        if(tokens[i].ID[0] == S_EQUALS)
            return true;
        i++;
    }
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
    parent->children[j]->SYM = 1;
    parent->children[j]->ID = "var";

    // add child node for variable ID
    int j2 = new_child_node(parent->children[j]);

    // add variable ID to AST
    parent->children[j]->children[j2]->ID = tokens[i].ID;
    parent->children[j]->children[j2]->SYM = 1;

    sym_assigncheck(tokens[i].ID, INT, 1);


    if(DEBUG) printf("newVarNode - adding token: %s   j: %d    j2: %d\n", tokens[i].ID, j, j2);
   
    // check if array - if so theres an additional node that needs created
    if(!strcmp(tokens[i+1].ID, "[")){
        if(DEBUG)printf("newVarNode: deteced that %s is an array\n", tokens[i].ID);
        i+=2; //skip array ID and '['
        
        i = newOpNode(i, parent->children[j]);
    
        if(tokens[i].ID[0] == S_SEMICOLON){
            return i;
        }
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
    parent->children[j]->base10val = strtol(tokens[i].ID, NULL, 10);

    if(DEBUG)printf("newNumNode - adding token: %s   j: %d\n", tokens[i].ID, j);

    i++;

    if(DEBUG_FUNC_EXIT)printf("16 newNumNode exit - token: %s\n", tokens[i].ID);
    return i;
}

int newAssignNode(int i, Node *parent){
    if(DEBUG_FUNC_ENTER)printf("14 newAssignNode - token: %s\n", tokens[i].ID);

        if(tokens[i].type == T_NUM)
          errorExit("newAssignNode - assignment to expr", i);

        if(isFunction(i+2)){
            
            int j;
            j = new_child_node(parent);
            parent->children[j]->ID = "=";
            parent->children[j]->callAssign = 1;

            i = newVarNode(i,parent->children[j]);


            i = newCallNode(i+1, parent->children[j]);
        }
        else{
            i = newOpNode(i, parent);
        }

        if(DEBUG_FUNC_EXIT)printf("14 newAssignNode exit - token: %s\n", tokens[i].ID);
        return i;
}

int newEqlStmtNode(int i, Node *controlStmt){
    if(DEBUG_FUNC_ENTER)printf("13 newEqlStmtNode - token: %s\n", tokens[i].ID);

    if(tokens[i].type == T_ID || tokens[i].type == T_NUM)
        i = newOpNode(i, controlStmt);
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


    if(DEBUG_FUNC_EXIT)printf("11 newCallNode exit - token: %s\n", tokens[i].ID);
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

    i++;

    if(tokens[i].type == T_ID || tokens[i].type == T_NUM)
        i = newOpNode(i, retNode);
    else if(tokens[i].ID[0] == ';');
    else
    errorExit("newEqlStmtNode - unknown token in place of var/num", i);

    if(DEBUG_FUNC_EXIT)printf("10 newReturnNode exit - token: %s\n", tokens[i].ID);
    return i;
}



/*** *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
    operator functions
*/

int newOpNode(int i, Node *parent){
    if(DEBUG_FUNC_ENTER)printf("newOpNode token: %s\n", tokens[i].ID);
    
    Node* opNode;
    opNode = expression(&i);

    int j = get_free_index(parent);
    parent->children[j] = opNode;
    opNode->root = 1; //helps with printing

    if(tokens[i].ID[0] == S_CLOSE_PARENTHESIS)
        i++;

    

    if(DEBUG_FUNC_EXIT)printf("newOpNode exit - token: %s\n", tokens[i].ID);
    return i;
}

Node * expression(int *index){
    int i = *index;
    if(DEBUG_FUNC_ENTER)printf("expression - token: %s\n", tokens[i].ID);

    char *operator;
    Node *node;

    node = assignterm(&i);

    // highest precedence items fall into loop
    while(tokens[i].ID[0] == S_EQUALS){
        operator = tokens[i].ID;
        i++;
        node = get_op_node(node, assignterm(&i), operator);
        if(DEBUG)printf("expression made new opNode->ID: %s\n", node->ID);
    }

    *index = i;
    if(DEBUG_FUNC_EXIT)printf("expression exit - token: %s\n", tokens[i].ID);
    return node;
}

Node * assignterm(int *index){
    int i = *index;
    if(DEBUG_FUNC_ENTER)printf("compterm - token: %s   i: %d\n", tokens[i].ID, i);
    Node *node;
    char *tok;

    node = compterm(&i);

    // second highest precedence
    while(isComparison(i)){
        tok = tokens[i].ID;
        i++;
        node = get_op_node(node, compterm(&i), tok); 
        if(DEBUG)printf("compterm made new opNode->ID: %s\n", node->ID);
    }


    *index = i;
    if(DEBUG_FUNC_EXIT)printf("compterm  exit - token: %s   i: %d\n", tokens[i].ID, i);
    return node;


}

Node * compterm(int *index){
    int i = *index;
    if(DEBUG_FUNC_ENTER)printf("compterm - token: %s   i: %d\n", tokens[i].ID, i);
    Node *node;
    char *tok;

    node = term(&i);

    // lowest level precedence
    while(tokens[i].ID[0] == S_ADD || tokens[i].ID[0] == S_SUB){
        tok = tokens[i].ID;
        i++;
        node = get_op_node(node, term(&i), tok); 
        if(DEBUG)printf("compterm made new opNode->ID: %s\n", node->ID);
    }


    *index = i;
    if(DEBUG_FUNC_EXIT)printf("compterm  exit - token: %s   i: %d\n", tokens[i].ID, i);
    return node;


}

Node * term(int *index){
    int i = *index;
    if(DEBUG_FUNC_ENTER)printf("term - token: %s   i: %d\n", tokens[i].ID, i);
    Node *node;
    char *tok;

    node = factor(&i);

    while(tokens[i].ID[0] == S_MUL || tokens[i].ID[0] == S_DIV){
        tok = tokens[i].ID;
        i++;
        node = get_op_node(node, factor(&i), tok); 
        if(DEBUG)printf("term made new opNode->ID: %s\n", node->ID);
    }

    *index = i;
    if(DEBUG_FUNC_EXIT)printf("term  exit - token: %s   i: %d\n", tokens[i].ID, i);
    return node;
}

Node * factor(int *index){
    int i = *index;
    if(DEBUG_FUNC_ENTER)printf("factor - token: %s\n", tokens[i].ID);
    Node *node;

    if(tokens[i].ID[0] == S_OPEN_PARENTHESIS){
        i++;
        node = expression(&i);
        if(tokens[i].ID[0] != S_CLOSE_PARENTHESIS){
            errorExit("factor - no close paren", i);
        }
        else
        {
            i++;
        }
        
    }
    else if(isOpSym(i) || isComparison(i) || tokens[i].ID[0] == S_EQUALS) 
        errorExit("factor - unexpected operator", i);
    else{
        if(tokens[i].type == T_NUM){
            node = get_zeroed_node();
            node->ID = tokens[i].ID;
            node->num = 1;
            node->base10val = strtol(tokens[i].ID, NULL, 10);
            if(DEBUG)printf("factor put token: %s in node\n", tokens[i].ID);
        }
        else if(tokens[i].type == T_ID){
            node = get_zeroed_node();
            node->ID = "var";
            node->SYM = 1;

            node->right = get_zeroed_node();
            node->right->ID = tokens[i].ID; //variable ID
            node->left = NULL;

            char *tok = tokens[i].ID;

            if(DEBUG)printf("factor put token: %s in right child of \"var\" node\n", tokens[i].ID);

            // check if array
            if(tokens[i+1].ID[0] == S_OPENING_BRACKET){
                i+=2;
            
                node->right->left = expression(&i); 
               
                 //sym_assigncheck(tok, INT_ARRAY, node->right->left->base10val);
            }
            else
            {
                //sym_assigncheck(tok, INT, 1);
            }
            
            
        }
        //else errorExit("factor - unknown token", i);

        i++;
    }

    *index = i;
    if(DEBUG_FUNC_EXIT)printf("    factor exit - token: %s\n", tokens[i].ID);
    return node;
}

Node * get_op_node(Node *left, Node *right, char *op){
    Node *opNode = get_zeroed_node();
    opNode->ID = op;
    opNode->op = 1;
    opNode->left = left;
    opNode->right = right;

    if(left->num == 1 && right->num == 1){

        // see if we can do a math op
        if(op[0] == S_ADD){
            if(left->op == 1)
                opNode->base10val += left->base10val;
            else{        
                int a = strtol(left->ID, NULL, 10);
                int b = strtol(right->ID, NULL, 10);
                opNode->base10val =  a+b;
                //printf("opNode %s %s %s == %d\n", left->ID, op, right->ID, opNode->base10val);
            }
        }
        else if(op[0] == S_SUB){
            if(left->op == 1)
                opNode->base10val += left->base10val;
            else{
                int a = strtol(left->ID, NULL, 10);
                int b = strtol(right->ID, NULL, 10);
                opNode->base10val =  a-b;
                //printf("opNode %s %s %s == %d\n", left->ID, op, right->ID, opNode->base10val);
            }
        }
        else if(op[0] == S_DIV){
            if(left->op == 1)
                opNode->base10val += left->base10val;
            else{
                int a = strtol(left->ID, NULL, 10);
                int b = strtol(right->ID, NULL, 10);
                opNode->base10val =  a/b;
                //printf("opNode %s %s %s == %d\n", left->ID, op, right->ID, opNode->base10val);
            }
        }
        else if(op[0] == S_MUL){
            if(left->op == 1)
                opNode->base10val += left->base10val;
            else{        
                int a = strtol(left->ID, NULL, 10);
                int b = strtol(right->ID, NULL, 10);
                opNode->base10val =  a*b;
                //printf("opNode %s %s %s == %d\n", left->ID, op, right->ID, opNode->base10val);
            }
        }
    }

    if(DEBUG)printf("get_op_node   token: %s      base10val: %d\n", op, opNode->base10val);

    return opNode;
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

    char *tok = tokens[i].ID;

    // TO DO - below allows the declaration of variables to simple varNodes only

    // check if array is simple array declaration "a[]" (function paramter)
    if(tokens[i+1].ID[0] == S_OPENING_BRACKET && tokens[i+2].ID[0] == S_CLOSING_BRACKET){    // add variable ID
        j = new_child_node(parent);
        parent->children[j]->ID = "\\[\\]";
        add_symtable(tok, INT_ARRAY, 1);        
        i+=2;
    }
    else if(tokens[i+1].ID[0] ==  S_OPENING_BRACKET){ // full array declaration ex: "a[10]"
        j = get_free_index(parent);
        i+=2;
        parent->children[j] = expression(&i); // size of array
        add_symtable(tok, INT_ARRAY, parent->children[j]->base10val);        
    }
    else{
        if(DEBUG)printf("add symtable tok: %s\n", tok);
        add_symtable(tok, INT, 1);
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
    scope_in(tokens[i+1].ID);

    // add function return type
    j = new_child_node(funcNode);
    funcNode->children[j]->ID = tokens[i].ID;
    i++;

    // add function ID
    j = new_child_node(funcNode);
    funcNode->children[j]->ID = tokens[i].ID;  

    // add to sym table
    add_symtable(tokens[i].ID, FUNC, 1);
    scope_in(tokens[i].ID);

    i++;

    // add param node
    j = new_child_node(funcNode);
    funcNode->children[j]->ID = "params";  

    // skip '('
    i++;

    // if argument is not "void" call handleFucnParams()
    if(strcmp(tokens[i].ID, "void"))
        i = handleFuncParams(i, funcNode->children[j]);
    else
        i+=2;
    

    // should be "void" or last token in function arguments before ')'
    //i++;

    // skip ')'
    //if(!(tokens[i++].ID[0] != S_CLOSE_PARENTHESIS || tokens[i].ID[0] != S_COMMA))
    //    errorExit("handleFuncDecl - expecting ')' or ',' but got", i);

    if(tokens[i].ID[0] == S_OPENING_BRACE)
        i = newCompStmtNode(i, funcNode);
    else errorExit("handleFuncDecl -- no compound statment after function", i);


    scope_out();
    if(DEBUG_FUNC_ENTER)printf("01 handleFuncDecl exit - token: %s\n", tokens[i].ID);
    return i;
}

int handleCallArgs(int i, Node *argNode){
    if(DEBUG_FUNC_ENTER)printf("handleCallArgs - token: %s\n", tokens[i].ID);
    
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

        if(isOpSym(i+1) || isComparison(i+1))
            errorExit("handleFuncParams - op or comp symbol in params", i);
        
        if(isOpSym(i+2) || isComparison(i+2))
            errorExit("handleFuncParams - op or comp symbol in params", i);

        // create param node
        j = new_child_node(params);
        params->children[j]->ID = "param";
        
        // add var decl to param node
        i = handleVarDecl(i, params->children[j]);
    }

    if(DEBUG_FUNC_ENTER)printf("03 handleFuncParam exit - token: %s\n", tokens[i].ID);
    return i;
}

int handleControlStmt(int i, Node *controlNode){
    if(DEBUG_FUNC_ENTER)printf("30 handleControlStmt - token: %s\n", tokens[i].ID);    
    
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
    

    // current token should be '{' signaling the start of a compound statement
    if(tokens[i].ID[0] == S_OPENING_BRACE){
        i = newCompStmtNode(i, controlNode);       
    }
    else{
        if(tokens[i].type == T_NUM)
            i = newNumNode(i, controlNode);
        else if(tokens[i].type == T_ID)
            i = newVarNode(i, controlNode);
        else if(tokens[i].ID[0] == S_SEMICOLON)
            i = newSemicolonNode(i, controlNode); 
    }

    if(!strcmp(controlNode->ID, "selection-stmt")){
        if(!strcmp(tokens[i+1].ID, "else"))
            i += 2;
            if(tokens[i].ID[0] == S_OPENING_BRACE)
                i = newCompStmtNode(i, controlNode);
            else{
                if(tokens[i].type == T_NUM)
                    i = newNumNode(i, controlNode);
                else if(tokens[i].type == T_ID)
                    i = newVarNode(i, controlNode);
                else if(tokens[i].ID[0] == S_SEMICOLON)
                    i = newSemicolonNode(i, controlNode); 
            }
    }
    

    // should be returning ')' if no compound statment. We check
    //  for '{' above, if it exists we should call newCompStmt() then return '}' 
    if(DEBUG_FUNC_EXIT)printf("30 handleControlStmt exit - token: %s\n", tokens[i].ID);
    return i;
}

int handleCompStmt(int i, Node *compStmt){
    if(DEBUG_FUNC_ENTER)printf("50 handleCompStmt - token: %s\n", tokens[i].ID);    
    bool stmtDeclared = false;  // used to assure declarations do not come after statements

    while((tokens[i].ID[0] != S_CLOSING_BRACE) && (tokens[i].lineNumber != 0)){

        if(DEBUG)printf("handleCompStmt loop token : %s    linenum: %d\n", tokens[i].ID, tokens[i].lineNumber);
        
        
 /*ID*/ if(tokens[i].type == T_ID){
            stmtDeclared = true;
            
            if(isAssignment(i)){
                i = newAssignNode(i, compStmt);
            }
            else if(isFunction(i)){
                i = newCallNode(i, compStmt);
            }
            else{
                if(tokens[i+1].type == T_ID)
                    errorExit("handleCompStmt - two adjancent ID types", i);
                else
                    i = newOpNode(i, compStmt); 
            }
        }
 /*KEY*/else if(tokens[i].type == T_KEY){
            if(isVarDecl(i)){
                if(stmtDeclared == false){
                    i = newVarDeclNode(i, compStmt);
                    i--;
                }
                else errorExit("handleCompStmt - var declared after statement", i);
            }
            else if(!strcmp(tokens[i].ID, "if") || !strcmp(tokens[i].ID, "while")){
                i = newControlNode(i, compStmt);
            }
            else if(!strcmp(tokens[i].ID, "return")){
                i = newReturnNode(i, compStmt);
            }
            else if(!strcmp(tokens[i].ID, "else")){
                i = handleCompStmt(i+1, compStmt);
                return i;
            }
            else errorExit("handleCompStmt - unhandled key", i);
        }
 /*SYM*/else if(tokens[i].type == T_SYM){
            stmtDeclared = true;

            if(tokens[i].ID[0] == S_SEMICOLON)
                i = newSemicolonNode(i, compStmt);
            else if(tokens[i].ID[0] == S_OPENING_BRACE){
                i = newCompStmtNode(i, compStmt);
            }
            else if(tokens[i].ID[0] == S_OPEN_PARENTHESIS)
                i = newOpNode(i, compStmt);
            else
                errorExit("handleCompStmt - unknown sym", i);
            
        }
 /*NUM*/else if(tokens[i].type == T_NUM){
            stmtDeclared = true;

            if(isComparison(i+1)){
                if(isComparison(i+3))
                    errorExit("handleComp: double comparison", i);
                i = newOpNode(i, compStmt);
            }    
            else if(isOpSym(i+1))
                i = newOpNode(i, compStmt);
            else if(tokens[i+1].ID[0] == S_EQUALS)
                i = newAssignNode(i, compStmt);
            else if(tokens[i+1].ID[0] == ';'){
                i = newNumNode(i, compStmt);
            }
            else
                errorExit("handleComptStmt - unhandled num stmt", i);
            
        }

        i++;
    }

    if(DEBUG_FUNC_EXIT)printf("50 handleCompStmt exit - token: %s\n", tokens[i].ID);    
    return i; // return '}'
}




/*** *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   
    print functions
*/

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
        if(compStmt->children[j]->controlStmt == 1){
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
            printCompStmt(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->op == 1){
            printPostOrder(opf, compStmt->children[j]);
        }
        else if(compStmt->children[j]->num == 1)
            printPostOrder(opf, compStmt->children[j]);
        else if(compStmt->children[j]->SYM == 1)
            printPostOrder(opf, compStmt->children[j]);
        else if(compStmt->children[j]->callAssign = 1)
            printCallStmt(opf, compStmt->children[j]);

        j++;
    }

    fprintf(opf, "\n    ]\n"); // compound stmt closing bracket
}

void printControlStmt(FILE *opf, Node *ifStmt){
    // control stmt opening bracket and type (selection-stmt/iteration-stmt)
    fprintf(opf, "      [%s\n", ifStmt->ID); 

    // print condition
    printPostOrder(opf, ifStmt->children[0]);

    // if control statment node 1 has compound or other node
    if(ifStmt->children[1] != NULL){
        if(ifStmt->children[1]->compStmt == 1)
            printCompStmt(opf, ifStmt->children[1]);
        else if(ifStmt->children[1]->SYM == 1)
            printVar(opf, ifStmt->children[1]);
        else if(ifStmt->children[1]->num == 1)
            printNum(opf, ifStmt->children[1]);
        else if(ifStmt->children[1]->semicolon = 1)
            fprintf(opf, "\n        [%c]\n", ifStmt->children[1]->ID[0]);
    }

    // if control statment node 2 has compound or other node
    if(ifStmt->children[2] != NULL){
        if(ifStmt->children[2]->compStmt == 1)
            printCompStmt(opf, ifStmt->children[2]);
        else if(ifStmt->children[2]->SYM == 1)
            printVar(opf, ifStmt->children[2]);
        else if(ifStmt->children[2]->num == 1)
            printNum(opf, ifStmt->children[2]);
        else if(ifStmt->children[2]->semicolon = 1)
            fprintf(opf, "\n       [%c]\n", ifStmt->children[2]->ID[0]);
    }

    fprintf(opf, "\n      ]\n");
}

void printCallStmt(FILE *opf, Node *callStmt){
    if(callStmt->callAssign == 1){
        fprintf(opf, "        [%s", callStmt->ID); // =
        fprintf(opf, " [%s", callStmt->children[0]->ID); // var
        fprintf(opf, "[%s]]\n", callStmt->children[0]->children[0]->ID); // g

        callStmt = callStmt->children[1];
    
    }
    
    fprintf(opf, "        [%s\n", callStmt->ID); // [call

    fprintf(opf, "          [%s]\n", callStmt->children[0]->ID); // [args

    Node *args = callStmt->children[1];
    
    fprintf(opf, "          [%s", args->ID);
    
    int j = 0;
    while(args->children[j] != NULL){
        if(args->children[j]->SYM == 1){
            printVar(opf, args->children[j]);
        }
        else
            fprintf(opf, "[%s]", args->children[j]->ID);
        j++;
    }

    fprintf(opf, "]\n        ]]\n");
}

void printVar(FILE *opf, Node *varNode){
    if(varNode->SYM != 1)
        errorExit("printVar - expected var node", -1);

    // print "var"
    fprintf(opf, "  [%s", varNode->ID); //var opening bracket
    // variable id
    fprintf(opf, " [%s]" , varNode->children[0]->ID);
    // index if array
    if(varNode->children[1] != NULL)
        fprintf(opf, " [%s]", varNode->children[1]->ID);

    fprintf(opf, "]"); // var closing bracket
}

void printNum(FILE *opf, Node *numNode){
    if(numNode->num != 1)
        errorExit("printNum - expected num node", -1);

    // print value
    fprintf(opf, "[%s]", numNode->ID);
}

void printAssStmt(FILE *opf, Node *assStmt){
    fprintf(opf, "  $$[%s ", assStmt->ID);
    
    if(assStmt->children[0]->SYM == 1)
        printVar(opf, assStmt->children[0]);
    else
        printNum(opf, assStmt->children[0]);
    
    // could be a value or
    if(assStmt->children[1]->op == 1){
         printPostOrder(opf, assStmt->children[1]);
    }
    else if(assStmt->children[1]->callStmt == 1){
            fprintf(opf, "\n");
            printCallStmt(opf, assStmt->children[1]);
            fprintf(opf, "        ]");
    }
    else if(assStmt->children[1]->SYM ==  1)
        printVar(opf, assStmt->children[1]);
    else
        printNum(opf, assStmt->children[1]);
    
    fprintf(opf, "$]\n");
}

void printVarDecl(FILE *opf, Node *varDecl){
    fprintf(opf, "  [var-declaration ");
    int j = 0;
    while(varDecl->children[j] != NULL){
        fprintf(opf, " [%s]", varDecl->children[j++]->ID);
    }

    fprintf(opf, "]\n");
}

void printPostOrder(FILE *opf, Node *node){

    if(node == NULL)
        return;

    if(node->op != 1)
        fprintf(opf, " [%s", node->ID);
    else{
        if(node->root == 1)
            fprintf(opf, "        [%s", node->ID);
        else
            fprintf(opf, "\n        [%s", node->ID);
    }

    if(node->SYM != 1 && node->op != 1)
        fprintf(opf, "]");

    printPostOrder(opf, node->left);

    printPostOrder(opf, node->right);
    
    if(node->num != 1 && node->SYM != 1)
       fprintf(opf, "]");
    
    if(node->root == 1)
        fprintf(opf, "\n");
}

void printRetStmt(FILE *opf, Node *retStmt){
    fprintf(opf, "      [%s", retStmt->ID);

    if(retStmt->children[0] != NULL){
        printPostOrder(opf, retStmt->children[0]);
    } 
    fprintf(opf, "]");
}



int main(int argc, char *argv[]){

    if(argc < 3) {
        printf("file names not detected!\n");
        printf("usage: ./parser <input file> <output file>\n");
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

    scope_in("program");
    while(j < i){ // i is number of tokens

        if(isFuncDecl(j)){
            
            j = newFuncDeclNode(j, Tree);
            scope_out();
            if(DEBUG)printf("finished declaring function - token: %s\n", tokens[j].ID);
        }
        else if(isVarDecl(j)){
            j = newVarDeclNode(j, Tree);
            if(DEBUG)printf("finished declaring var - token: %s\n", tokens[j].ID);
        }
        else if(tokens[j].ID[0] == S_CLOSING_BRACE)
            j++;
        else
        {
            errorExit("Main - unknown stmt", j);
        }
    }
    scope_out();

    printTree(ofp, Tree);

    //print_symboltable();


    //if(DEBUG)
    //printf("\n\n\n-- exiting program normally\n\n\n");
}