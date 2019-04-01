CC=gcc
CFLAGS=-I.

parser: parser.o lex.o tree.o
	$(CC) -o parser parser.o lex.o tree.o

clean:
	rm *.o parser
