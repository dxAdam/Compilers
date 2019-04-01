#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lex.h"


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


token_t create_token(int type, int linenum, char *val, char *ID){
    token_t token;

    if(type == T_NUM){
        token.type = type;
        token.lineNumber = linenum;
        strcpy(token.num, val);
        strcpy(token.ID, val); // this is a quick fix for a parsing problem
    }
    else{
        token.type = type;
        token.lineNumber = linenum;
        strcpy(token.ID, ID);
        strcpy(token.num, ID); // this is a quick fix for a parsing problem
    }
    return token;
}


int printToken(FILE *ofp, token_t token){

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


token_t getToken(FILE *ifp){

    char buf[MAXSTRINGSIZE];
    static int linenum = 1;
    int c;

    // begin reading 1 char at a time
    while((c = fgetc(ifp)) != EOF){
        if(c == '\n') linenum++;

        token_t token;

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
            
            ungetc(c, ifp);
            return token;
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
                ungetc(nextchar[0], ifp);
                return token;
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
                        return token;
                    }
                }
                else{
                    // double special char is not /* so tokenize normally if valid
                    if(!isSpecialSym(buf)){
                        ungetc(buf[1], ifp);
                        buf[1] = '\0';
                    }
                        token = create_token(T_SYM, linenum, 0, buf);
                        return token;
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
            ungetc(c, ifp);
            return token;
        }

        // if none of the above structures are entered then c
        //  is either a whitespace character or invalid
        if(!(c == S_SPACE || c == S_NEWLINE || c == S_TAB)){
            buf[0] = c;
            buf[1] = '\0';
            token = create_token(T_ERROR, linenum, buf, buf);
            return token;
        }
    }

    // we signal no more tokens by returning a token with linenum == -1
    return create_token(T_ERROR, -1, buf, buf);
}
