CC=gcc
CFLAGS=-g
DEPS = lex.h

%.o: %.c $(DEPS)
	@$(CC) -c -o $@ $< $(CFLAGS)

lex: lex.o 
	@$(CC) -o lex lex.o 

clean:
	rm -f *.o
