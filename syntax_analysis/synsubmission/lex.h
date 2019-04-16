/* define special symbols - use ASCII values for single character
   symbols - used names found at https://www.ascii-code.com/

   for reference:   Symbols   ASCII CODES
                    0-9       48-57
                    A-Z       65-90
                    a-z       97-122         
*/

#define MAXSTRINGSIZE 256
#define MAXLINENUM 6 //6 digits
#define MAXTYPESIZE 3

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


char *keywords[] = {"else",   "if",   "int",
                    "return", "void", "while", "\0"};
                                    // "\0" marks end of arrays

char *special_symbols[] = {"+",  "-",  "*",  "/",  "<",  "<=",
                           ">",  ">=", "==", "!=", "=",  ";",
                           ",",  "(",  ")",  "[",  "]",  "{", 
                           "}",  "/*", "\0"};

typedef struct token {
    int type;
    int lineNumber;
    char num[MAXSTRINGSIZE];      // digit digit*
    char ID[MAXSTRINGSIZE];       // lettter(letter|digit)*
} token_t;

// function prototypes
int isKeyword(char buf[]);
int isSpecialSym(char buf[]);
token_t create_token(int type, int linenum, char *num, char *ID);
int printToken(FILE *ofp, token_t token);
int handle_comment(int *linenum, FILE *ifp);
token_t getToken(FILE *ofp);


