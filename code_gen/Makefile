CC=gcc
CFLAGS=-I.

codegen: symtable.o lex.o tree.o parser.o codegen.o
	$(CC) -o codegen symtable.o lex.o tree.o parser.o codegen.o

clean:
	rm *.o codegen

