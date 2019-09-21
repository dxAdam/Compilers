#include "tree.h"

#define MAXSTRINGSIZE 256

#define S_ADD                 '+'
#define S_SUB                 '-'
#define S_MUL                 '*'
#define S_DIV                 '/'
#define S_LESS_THAN           '<' 
#define S_GREATER_THAN        '>'
#define S_EQUALS              '='
#define S_SEMICOLON           ';'
#define S_COMMA               ','
#define S_OPEN_PARENTHESIS    '('
#define S_CLOSE_PARENTHESIS   ')'
#define S_OPENING_BRACKET     '['
#define S_CLOSING_BRACKET     ']'
#define S_OPENING_BRACE       '{'
#define S_CLOSING_BRACE       '}'

// for multiple character symbols we can use values after 255
#define S_LT_EQUAL            256    //   <=
#define S_GT_EQUAL            257    //   >=
#define S_EQUAL_EQUAL         258    //   ==
#define S_NOT_EQUAL           259    //   !=
#define S_START_COMMENT       260    //   /*
#define S_END_COMMENT         261    //   */

// define keywords
#define K_ELSE                270
#define K_IF                  271
#define K_INT                 272
#define K_RETURN              273
#define K_VOID                274
#define K_WHILE               275

// whitespace
#define S_SPACE               ' '
#define S_NEWLINE             10
#define S_TAB                 9

// types
#define T_ID                  280
#define T_NUM                 281
#define T_KEY                 282
#define T_SYM                 283
#define T_ERROR               284



typedef struct token {
    int type;
    int lineNumber;
    char num[MAXSTRINGSIZE];      // digit digit*
    char ID[MAXSTRINGSIZE];       // lettter(letter|digit)*
} token_t;


typedef enum { false, true } bool;


token_t getToken(FILE *ofp);

void errorExit(char *s, int i);

bool isVarDecl(int i);
bool isFuncDecl(int i);
bool isFunction(int i);
bool isAssignment(int i);
bool isOpSym(int i);
bool checkCloseParen(int i);

int newVarDeclNode(int i, Node *Tree);
int newFuncDeclNode(int i, Node *Tree);
int newIfNode(int i, Node *compStmt);
int newWhileNode(int i, Node *compStmt);
int newReturnNode(int i, Node *compStmt);
int newCallNode(int i, Node *compStmt);
int newNumNode(int i, Node *parent);
int newSemicolonNode(int i, Node *compStmt);
int newOpNode(int i, Node *parent);

Node * expression(int *index);
Node * factor(int *index);
Node * term(int *index);
Node * compterm(int *index);
Node * assignterm(int *index);
Node * get_op_node(Node *left, Node *right, char *op);

int handleVarDecl(int i, Node *varNode);
int handleVarArrDecl(int i, Node *varNode);
int handleFuncDecl(int i, Node *varNode);
int handleFuncParams(int i, Node *params);
int handleCompStmt(int i, Node *compStmt);
int handleControlStmt(int i, Node *controlNode);
int handleEqlStmt(int i, Node *eqlStmt);
int handleSimpleAddOp(int i, Node *parent);
int handleComplexAddOp(int i, int tmp, Node *parent);
int handleCallStmt(int i, Node *callStmt);
int handleCallArgs(int i, Node *argNode);

void printTree(FILE * opf, Node *Tree);
void printCompStmt(FILE * opf, Node *compStmt); // if / while
void printControlStmt(FILE * opf, Node *ifStmt);
void printAssStmt(FILE * opf, Node *assStmt);
void printVarDecl(FILE * opf, Node *varDecl);
void printInOrder(FILE *opf, Node *node);
void printPostOrder(FILE *opf, Node *node);
void printRetStmt(FILE * opf, Node *retStmt);
void printVar(FILE * opf, Node *varNode);
void printCallStmt(FILE * opf, Node *callStmt);
void printNum(FILE *opf, Node *numNode);


