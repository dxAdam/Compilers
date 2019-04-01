#include "tree.h"
#include "globals.h"

#define MAXSTRINGSIZE 256

#define S_PLUS                '+'
#define S_HYPHEN              '-'
#define S_ASTERISK            '*'
#define S_SLASH               '/'
#define S_LESS_THAN           '<' 
#define S_GREATER_THAN        '>'
#define S_EQUALS              '='
#define S_SEMICOLON           ';'
#define S_COMMA               ','
#define S_OPEN_PARENTHESIS    '('
#define S_CLOSE_PARENTHESIS   ')'
#define S_OPENING_BRACKET     '['
#define S_CLOSING_BRACKET     ']'
#define S_OPENING_BRACE       '}'
#define S_CLOSING_BRACE       '{'

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

token_t getToken(FILE *ofp);
int printToken(FILE *ofp, token_t token);

int newVarDeclNode(int i, Node *Tree);
int newFuncDeclNode(int i, Node *Tree);
int newReturnNode(int i, Node *compStmt);
int newNumNode(int i, Node *parent);

int handleVarDecl(int i, Node *varNode);
int handleVarArrDecl(int i, Node *varNode);
int handleFuncDecl(int i, Node *varNode);
int handleFuncParams(int i, Node *params);
int handleCompStmt(int i, Node *compStmt);
int handleIfStmt(int i, Node *compStmt);
int handleEqlStmt(int i, Node *eqlStmt);
int handleSimpleAddOp(int i, Node *parent);
int handleComplexAddOp(int i, int tmp, Node *parent);

void printTree(Node *Tree);
void printCompStmt(Node *compStmt);
void printIfStmt(Node *ifStmt);
void printAssStmt(Node *assStmt);
void printVarDecl(Node *varDecl);
void printSimpleAdd(Node *addStmt);
void printRetStmt(Node *retStmt);
void printVar(Node *varNode);


