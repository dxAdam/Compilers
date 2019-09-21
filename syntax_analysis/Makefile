CC=gcc
CFLAGS=-I.

parser: symtable.o lex.o tree.o parser.o
	$(CC) -o parser symtable.o lex.o tree.o parser.o

clean:
	rm *.o
