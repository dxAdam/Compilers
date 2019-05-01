#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtable.h"

#define KILLPROGRAM 0   // 1 - exits program on error
#define DEBUG 0

Symnode ST[MAXSYMBOLS];
int st_top = 0;

char *scopestack[MAXSCOPELEVEL];
int scope_top = 0;

extern int isKeyword(char buf[]);



/* 
    symbol table functions
*/

char * getsym_scope(char *id){

    //printf("id: %ld\n", (unsigned long)id);
    //printf("st_stop: %d\n", st_top);

    int i = 0;
    while(i < st_top){
        if(!strcmp(id,ST[i].symbol))
            return ST[i].scope;
        i++;
    }

    return "-1";
}

int getsym_type(char *id){
    int i = 0;
    while(i < st_top){
        if(!strcmp(id,ST[i].symbol)){
            return ST[i].type;
        }
        i++;
    }

    return -1;
}

int getsym_size(char *id){
    int i = 0;
    while(i < MAXSCOPELEVEL){
        if(id == ST[i].symbol)
            return ST[i].size;
        i++;
    }

    return -1;
}

int add_symtable(char *id, int type, int size){
    
    if(DEBUG)printf("ST id: %s\n", id);
    
    if(getsym_type(id) != -1)
        return 1;

    
    if(size < 0){
        printf("SEMANTIC ERROR: array size less than zero\n\tsize: %d\n", size);
        if(KILLPROGRAM)exit(1);
    }

    if(isKeyword(id)){
        printf("SEMANTIC ERROR: sym is keyword\n\tsymbol: %s\n", id);
        if(KILLPROGRAM)exit(1);
    }

    int s = 0;


    if(DEBUG)printf("ST add symbol: %s    scope: %s\n", id, scopestack[scope_top]);


    ST[st_top].symbol = id;
    ST[st_top].type = type;
    ST[st_top].scope = scopestack[scope_top];
    ST[st_top].size = size;
    ST[st_top].pos = 0;
    st_top++;

    return 1;
}

int sym_exists(char *id){
    if(getsym_type(id)) // returns -1 on no sym
        return 1;

    return 0;
}

int set_pos(char *id, int pos){
    
    int i = 0;
    while(i < st_top){
        if(!strcmp(id,ST[i].symbol)){
            ST[i].pos = pos;
            return 1;
        }
        i++;
    }

    return -1;
}

int get_pos(char *id){
        int i = 0;
        while(i < st_top){
        if(!strcmp(id,ST[i].symbol)){
            return ST[i].pos;
        }
        i++;
    }
}

// checks if sym is safe to use in assignment
int sym_assigncheck(char *id, int type, int size){

    Symnode entry;
    entry.size = -2;

    int i = 0;
    while(i < st_top){
        if(!strcmp(id, ST[i].symbol)){
            entry = ST[i];
        }
        i++;
    }


    if(entry.size == -2){
        printf("ERROR: assignment to symbol that doesn't exist  symbol: %s\n", id);
        exit(1);
    }


    if(entry.type != type){
        if(!(entry.type == INT_ARRAY && type == INT))
        {
            printf("ERROR: assignment to symbol of different type  symbol: %s\n", entry.symbol);
            exit(1);
        }
    }


    if(strcmp(entry.scope, cur_scope()) && strcmp(entry.scope, "program")){
        printf("ERROR: assignment to symbol in different  scope: %s\n", entry.symbol); 
        exit(1);
    }

    if(size > entry.size || size < 0){
        printf("ERROR: assignment to invalid indice  symbol: %s\n", entry.symbol);
        exit(1);
    }

    return 1;

}




int print_symboltable(){
    int i = 0;
    printf("\n  SYMBOL TABLE:\n\n    symbol      type     scope      size\n\n");
    while(i < st_top){
        printf("%10s%10d%10s%10d\n", ST[i].symbol, ST[i].type, ST[i].scope, ST[i].size);
        i++;
    }
    printf("\n");
}



/* 
    scope functions
*/

char * cur_scope(){
    return scopestack[scope_top];
}

int scope_in(char *id){
    scope_top++;
    assert(scope_top < MAXSCOPELEVEL);
    
    scopestack[scope_top] = id;
    
    return scope_top;
}

char * scope_out(){
    scope_top--;
    assert(scope_top >= 0);
    return scopestack[scope_top+1];
}

int test_scope(int reqscope){
    if(reqscope == scope_top)
        return 1;
    else
        return 0;
}

int print_scopeVars(FILE* opf, char* scope){
    int i = 0;
    
    while(i < st_top){
        if(!strcmp(ST[i].scope, scope) && ST[i].type < 3){
            fprintf(opf, "%s: .space %d\n", ST[i].symbol, ST[i].size*4);
        }
        i++;
    }
}


int maindriver(){

    char SYM1[256];
    char SYM2[256];


    strcpy(SYM1, "sym1");
    strcpy(SYM2, "sym2");


    printf("SYM1: %s\n", SYM1);
    printf("SYM2: %s\n", SYM2);

    scope_in("1");

    add_symtable(SYM1, CHAR_ARRAY, 256);

    scope_in("2");

    add_symtable(SYM2, CHAR, 256);

    print_symboltable();


    printf("sym_asigncheck() == %d\n", sym_assigncheck(SYM1, INT, 1));

    printf("getsym_scope(SYM1) == %s\n", getsym_scope(SYM1));
    printf("getsym_type(SYM2)  == %s\n", getsym_scope(SYM2));

    return 1;
}
