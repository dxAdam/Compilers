/*
    Compiler Project Part 1 - Lexical Analysis

    compile with: 
            make

    run with:
            ./lex input.c output.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lex.h"

#define MAXSTRINGSIZE 256 // defines buffer size for reading file

int isKeyword(char buf[]){
    extern char *keywords[];

    int i = 0;
    while(keywords[i] != "\0"){
        if(strcmp(keywords[i++], buf) == 0){
            return 1;
        }
    }
    return 0; 
}


int isSpecialSym(char buf[]){
    extern char *special_symbols[];

    int i = 0;
    while(special_symbols[i] != "\0"){
        if(strcmp(special_symbols[i++], buf) == 0){
            return 1;
        }
    }
    return 0;
}


struct token_t create_token(int type, int linenum, char *val, char *ID){
    struct token_t token;

    if(type == T_NUM){
        token.type = type;
        token.lineNumber = linenum;
        token.num = val;
    }
    else{
        token.type = type;
        token.lineNumber = linenum;
        token.ID = ID;
    }
    return token;
}


int printToken(FILE *ofp, struct token_t token){
    if(token.type == T_KEY){
        fprintf(ofp, "(%d,KEY,\"%s\")\n", token.lineNumber, token.ID);
    }
    else if(token.type == T_ID){
        fprintf(ofp, "(%d,ID,\"%s\")\n", token.lineNumber, token.ID);
    }
    else if(token.type == T_SYM){
        fprintf(ofp, "(%d,SYM,\"%s\")\n", token.lineNumber, token.ID);
    }
    else if(token.type == T_NUM){
        fprintf(ofp, "(%d,NUM,\"%s\")\n", token.lineNumber, token.num);
    }
    else if(token.type == T_ERROR){
        fprintf(ofp, "(%d,ERROR,\"%s\")\n", token.lineNumber, token.ID);
    }

    fflush(ofp);
    return 1;
}


int handle_comment(int *linenum, FILE *ifp){
    int comment_start_linenum = *linenum;
    int found_end = 0;
    int c;

    while((c = getc(ifp)) != EOF && found_end == 0){ // skip comment look for '*/'
        if(c == '\n'){
            *linenum = *linenum + 1;
        }
        if(c == '*'){
            if((c =fgetc(ifp)) != EOF && c == '/'){
                // found end of comment
                return 1; 
            }
            else{
                ungetc(c, ifp);
            }
        }
    }

    *linenum = comment_start_linenum;

    return 0; // did not find end of comment

}

int main(int argc, char *argv[]){

    if(argc < 3) {
        printf("file names not detected!\n");
        printf("usage: ./lex <input file> <output file>\n");
        return 1;
    }

    FILE *ifp; 
    FILE *ofp;
    
    if((ifp = fopen(argv[1], "r")) == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    if((ofp = fopen(argv[2], "w+")) == NULL){
        printf("Error preparing output file\n");
        return 1;
    }

    char buf[MAXSTRINGSIZE];
    int linenum = 1;
    int c;

    // begin reading 1 char at a time
    while((c = fgetc(ifp)) != EOF){
        if(c == '\n') linenum++;

        struct token_t token;

        if(isalpha(c)){ // letter
            int i=0;
            do{
                buf[i++] = (char)c;
                c = fgetc(ifp);
            }while(isalnum(c)); // (letter|digit)*
            buf[i] = '\0';
            
            if(isKeyword(buf)){
                token = create_token(T_KEY, linenum, 0, buf);
            }
            else{ // string that is not keyword
                token = create_token(T_ID, linenum, 0, buf);
            }
            printToken(ofp, token);  
            
            ungetc(c, ifp);
            continue;
        }

        buf[0] = c;
        buf[1] = '\0';
        if(isSpecialSym(buf)){

            char nextchar[2];
            nextchar[0] = getc(ifp);
            nextchar[1] = '\0';

            if(!isSpecialSym(nextchar)){
                // next char is not a special symbol so tokenize current special char
                buf[1] = '\0';
                token = create_token(T_SYM, linenum, 0, buf);
                printToken(ofp, token);
                ungetc(nextchar[0], ifp);
            }
            else{
                // next char is also special symbol so tokenize double special char
                buf[1] = nextchar[0];
                buf[2] = '\0';

                // handle special case that double special char is /*
                if(!strcmp(buf, "/*")){
                    if(!handle_comment(&linenum, ifp)){
                        // end of comment not found print error
                        token = create_token(T_ERROR, linenum, 0, buf);
                        printToken(ofp, token);
                    }
                }
                else{
                    // double special char is not /* so tokenize normally if valid
                    if(!isSpecialSym(buf)){
                        ungetc(buf[1], ifp);
                        buf[1] = '\0';
                    }
                        token = create_token(T_SYM, linenum, 0, buf);
                        printToken(ofp, token);
                }
            }

            continue;
        }

        if(isdigit(c)){
            int i=0;
            do{ // read the rest of the digits
                buf[i++] = c;
                c = fgetc(ifp);
            }while(isdigit(c));
            buf[i] = '\0';

            token = create_token(T_NUM, linenum, buf, buf);
            printToken(ofp, token);

            ungetc(c, ifp);
            continue;
        }

        // if none of the above structures are entered then c
        //  is either a whitespace character or invalid
        if(!(c == S_SPACE || c == S_NEWLINE || c == S_TAB)){
            buf[0] = c;
            buf[1] = '\0';
            token = create_token(T_ERROR, linenum, buf, buf);
            printToken(ofp, token);
            return 0;
        }
    }

    fclose(ifp);
    fclose(ofp);
    return 0;
}
