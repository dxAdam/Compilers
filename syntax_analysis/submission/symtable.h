/*

    symbol table and scope stack

*/

#include <stdio.h>
#include <assert.h>

#define MAXSYMLENGTH 30
#define MAXSCOPELEVEL 1024
#define MAXSYMBOLS 1024

enum type{INT, INT_ARRAY, CHAR, CHAR_ARRAY, FUNC, PARAM, ARG};

typedef struct symnode{
    char *symbol;
    char *scope;
    unsigned int type;
    int size;
} Symnode;

int sym_assigncheck(char *id, int type, int size);
Symnode get_entry(char *id);
char * getsym_scope(char *id);
int getsym_size(char *id);
int getsym_type(char *id);
int test_scope(int reqscope);
int add_symtable(char *id, int type, int size);
int print_symboltable();



int scope_in(char *id);
char *scope_out();
char *cur_scope();