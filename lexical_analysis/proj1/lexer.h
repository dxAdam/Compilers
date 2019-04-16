/* define special symbols - use ASCII values for single character
   symbols - used names found at https://www.ascii-code.com/

   for reference:   Symbols   ASCII CODES
                    0-9       48-57
                    A-Z       65-90
                    a-z       97-122         
*/


// most of this isn't used but we may want it for the parser project

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


char *keywords[] = {"else",   "if",   "int",
                    "return", "void", "while", "\0"};
                                    // "\0" marks end of arrays

char *special_symbols[] = {"+",  "-",  "*",  "/",  "<",  "<=",
                           ">",  ">=", "==", "!=", "=",  ";",
                           ",",  "(",  ")",  "[",  "]",  "{", 
                           "}",  "/*", "\0"};

struct token_t {
    int type;
    int lineNumber;
    char *num;      // digit digit*
    char *ID;       // lettter(letter|digit)*
};

// function prototypes
int isKeyword(char buf[]);
int isSpecialSym(char buf[]);
struct token_t create_token(int type, int linenum, char *num, char *ID);
int printToken(FILE *ofp, struct token_t token);
int handle_comment(int *linenum, FILE *ifp);

