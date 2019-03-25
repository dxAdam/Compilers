CC=gcc
CFLAGS=-I.

parser: parser.o lex.o
	$(CC) -o parser parser.o lex.o

clean:
	rm *.o parser
