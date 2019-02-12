/*
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

int isKeyword(char buf[]){
    extern char *keywords[];

    int i = 0;
    while(keywords[i++] != "\0"){
        if(strcmp(keywords[i], buf) == 0){
            //printf("%s   matches   %s\n", buf, keywords[i]);
            return 1;
        }
    }
    return 0; 
}

int isSpecialSym(char buf[]){
    extern char *special_symbols[];

    int i = 0;
    while(special_symbols[i] != "\0"){
        if(strcmp(special_symbols[i], buf) == 0){
            return 1;
        }
        i++;
    }
    return 0;
}

struct token_t create_token(int type, int linenum, char *val, char *ID){
    struct token_t token;

    if(type == T_KEY || type == T_ID || type == T_SYM || T_ERROR){
        token.type = type;
        token.lineNumber = linenum;
        token.ID = ID;
    }
    else if(type == T_NUM){
        token.type = type;
        token.lineNumber = linenum;
        token.num = val;
    }
    return token;
}

int printToken(FILE *ofp, struct token_t token){
    if(token.type == T_KEY){
        fprintf(ofp, "(%d,KEY,\"%s\")\n", token.lineNumber, token.ID);
        printf("(%d,KEY,\"%s\")\n", token.lineNumber, token.ID);
    }
    else if(token.type == T_ID){
        fprintf(ofp, "(%d,ID,\"%s\")\n", token.lineNumber, token.ID);
        printf("(%d,ID,\"%s\")\n", token.lineNumber, token.ID);

    }
    else if(token.type == T_SYM){
        fprintf(ofp, "(%d,SYM,\"%s\")\n", token.lineNumber, token.ID);
        printf("(%d,SYM,\"%s\")\n", token.lineNumber, token.ID);
    }
    else if(token.type == T_NUM){
        printf("printing NUM %s\n", token.num);
        fprintf(ofp, "(%d,NUM,\"%s\")\n", token.lineNumber, token.num);
        printf("(%d,NUM,\"%s\")\n", token.lineNumber, token.num);   
    }
    else if(token.type == T_ERROR){
        fprintf(ofp, "(%d,ERROR,\"%s\")\n", token.lineNumber, token.ID);
        printf("(%d,ERROR,\"%s\")\n", token.lineNumber, token.ID); 
    }
    return 1;
}

int main(int argc, char *argv[]){

    if(argc < 3) {
        printf("file names not detected!\n");
        printf("usage: ./lex <input file> <output file>\n");
        return 1; // final version will exit here
    }

    FILE *ifp; 
    FILE *ofp;
    
    if(!(ifp = fopen(argv[1], "r"))) {
        printf("Error opening file\n");
        return 1;
    }
    else if(!(ofp = fopen(argv[2], "w+"))){
        printf("Error preparing output file\n");
        return 1;
    }

    
    char buf[256]; 
    int linenum = 1;
    int c;
    while((c = fgetc(ifp)) != EOF){
        //printf("%c\n", c);
        if(c == '\n') linenum++;
        //printf("%d  %c  %d\n", c, c, isalnum(c));

        struct token_t token;

        // if uppercase or lowercase letter
        //if((c>64 && c<91) || (c>96 && c<123)){
        if(isalpha(c)){ // letter
            int i=0;
            do{
                buf[i++] = (char)c;
                c = fgetc(ifp);
                //printf("%d  %c  %d\n", c, c, isalnum(c));

            }while(isalnum(c)); // (letter|digit)*
            buf[i] = '\0';
            
            if(isKeyword(buf))
                token = create_token(T_KEY, linenum, 0, buf);
            else // string that is not keyword
                token = create_token(T_ID, linenum, 0, buf);
            printToken(ofp, token);  
            
            ungetc(c, ifp);
            continue;
        }

        buf[0] = c;
        buf[1] = '\0';


        if(isSpecialSym(buf)){
            // handle single character symbols first

            buf[0] = fgetc(ifp);

            if(!isSpecialSym(buf)){
                int tmp = buf[0];
                buf[0] = c;
                buf[1] = '\0';
                token = create_token(T_SYM, linenum, 0, buf);
                printToken(ofp, token);
                ungetc(tmp, ifp);
            }
            else if(c == '/' && buf[0] == '*'){
                int comment_start_linenum = linenum;

                int found_end = 0;
                while((c = fgetc(ifp)) != EOF && found_end == 0){ // skip comment look for '*/'
                    if(c == '\n') linenum++;

                    if(c == '*')
                        if(fgetc(ifp) == '/')
                            found_end = 1;
                        else
                            ungetc(c, ifp);
                }

                if(!found_end){
                    buf[0] = '/';
                    buf[1] = '*';
                    buf[2] = '\0';
                    token = create_token(T_ERROR, comment_start_linenum, 0, buf);
                    printToken(ofp, token);
                    while(fgetc(ifp) != EOF);
                }
            }
            continue;
        }

        if(isdigit(c)){
            int i=0;
            do{
                buf[i++] = c;
                c = fgetc(ifp);
                //printf("%d  %c  %d\n", c, c, isalnum(c));

            }while(isdigit(c));
            buf[i] = '\0';

            token = create_token(T_NUM, linenum, buf, buf);
            printToken(ofp, token);

            ungetc(c, ifp);
            continue;
        }

        if(!(c == S_SPACE || c == S_NEWLINE || c == S_TAB)){
            token = create_token(T_ERROR, linenum, 0, buf);
            printToken(ofp, token);
            fflush(stdout);
            exit(1);
        }

    }

    fclose(ifp);
    fclose(ofp);
    printf("\nnormal exit\n");
    return 0;
}