#define MAXSTRINGSIZE 256

typedef struct token {
    int type;
    int lineNumber;
    char num[MAXSTRINGSIZE];      // digit digit*
    char ID[MAXSTRINGSIZE];       // lettter(letter|digit)*
} token_t;

token_t getToken(FILE *ofp);
int printToken(FILE *ofp, token_t token);
