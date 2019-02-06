/*
    compile with: 
            make

    run with:
            ./lex input.c output.txt

            (output not used yet)
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



int main(int argc, char *argv[]){

    if(argc < 3) {
        printf("file names not detected!\n");
        printf("usage: ./lex <input file> <output file>\n");
        return 1; // final version will exit here
    }

    FILE *fp; 
    
    if(!(fp = fopen(argv[1], "r"))) {
        printf("Error opening file\n");
        return 1;
    }
    
    char buf[256]; 
    int c;
    while((c = fgetc(fp)) != EOF){
        //printf("%d  %c  %d\n", c, c, isalnum(c));

        // if uppercase or lowercase letter
        if((c>64 && c<91) || (c>96 && c<123)){
            int i=0;
            do{
                buf[i++] = (char)c;
                c = fgetc(fp);
                //printf("%d  %c  %d\n", c, c, isalnum(c));

            }while(isalnum(c));
            buf[i] = '\0';

            if(isKeyword(buf)){
                printf("found keyword: %s\n", buf);
            }
        }


    }

    fclose(fp);
    printf("\nnormal exit\n");
    return 0;
}