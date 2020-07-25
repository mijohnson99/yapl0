CC = clang
CFLAGS = -O0 -g

ALL = lextest parsetest compiletest

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


%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $< -c -o $@

.PHONY: clean
clean:
	rm -f *.o $(ALL)
