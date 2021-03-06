CC = clang
CFLAGS = -O0 -g -Wall -Wextra

ALL = lextest parsetest compiletest pl02c

all: $(ALL)

lextest: lextest.o lexer.o
	$(CC) $^ -o $@

lextest.o: lextest.c
	$(CC) $(CFLAGS) $^ -c -o $@


parsetest: parsetest.o parser.o lexer.o
	$(CC) $^ -o $@

parsetest.o: parsetest.c
	$(CC) $(CFLAGS) $^ -c -o $@

compiletest: compiletest.o parser.o lexer.o
	$(CC) $^ -o $@

compiletest.o: compiletest.c
	$(CC) $(CFLAGS) $^ -c -o $@

pl02c: pl02c.o parser.o lexer.o
	$(CC) $^ -o $@

pl02c.o: pl02c.c
	$(CC) $(CFLAGS) $^ -c -o $@


%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $< -c -o $@

.PHONY: clean
clean:
	rm -f *.o $(ALL)
