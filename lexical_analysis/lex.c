#include <stdio.h>
#include <stdlib.h>
#include "lex.h"

int main(int argc, char *argv[]){

    if(argc < 3) {
        printf("file names not detected!\n");
        printf("usage: ./lex <input file> <output file>\n");
        return 1; // final version will exit here
    }

    FILE *fp; 
    
    if(!(fp = fopen("main.c", "r"))) {
        printf("Error opening file\n");
        return 1;
    }

    int c;
    while((c = fgetc(fp)) != EOF){
        printf("%c", c);
    }

    //fclose(fp);
    return 0;
}