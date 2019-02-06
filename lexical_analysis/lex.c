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

    //printf("buf: %s\n", buf);
    int i = 0;
    while(keywords[i++] != "\0"){
        if(strcmp(keywords[i], buf) == 0){
            //printf("%s   matches   %s\n", buf, keywords[i]);
            return 1;
        }
    }
    return 0; 
}

struct token_t create_token(int type, int linenum, int num, char *ID){
    struct token_t token;
    
    if(type == T_KEY || T_ID){
        token.type = type;
        token.lineNumber = linenum;
        token.num = 0;
        token.ID = ID;
    }

    return token;
}

int printToken(FILE *ofp, struct token_t token){
    if(token.type == T_KEY)
        fprintf(ofp, "(%d,KEY,\"%s\")\n", token.lineNumber, token.ID);
    else if(token.type == T_ID)
        fprintf(ofp, "(%d,ID,\"%s\")\n", token.lineNumber, token.ID);

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
        if(c == '\n') linenum++;
        //printf("%d  %c  %d\n", c, c, isalnum(c));

        // if uppercase or lowercase letter
        if((c>64 && c<91) || (c>96 && c<123)){
            int i=0;
            do{
                buf[i++] = (char)c;
                c = fgetc(ifp);
                //printf("%d  %c  %d\n", c, c, isalnum(c));

            }while(isalnum(c));
            buf[i] = '\0';
            
            struct token_t token;
            if(isKeyword(buf))
                token = create_token(T_KEY, linenum, 0, buf);
            else // string that is not keyword
                token = create_token(T_ID, linenum, 0, buf);
            printToken(ofp, token);
            continue;    
        }
    }

    fclose(ifp);
    fclose(ofp);
    printf("\nnormal exit\n");
    return 0;
}