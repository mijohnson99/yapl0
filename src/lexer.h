#ifndef LEXER_H
#define LEXER_H
#include <stdbool.h>

#define SYMBOLS(X) \
	X("const", CONST) \
	X("var", VAR) \
	X("procedure", PROCEDURE) \
	X("call", CALL) \
	X("begin", BEGIN) \
	X("end", END) \
	X("if", IF) \
	X("then", THEN) \
	X("while", WHILE) \
	X("do", DO) \
	X("odd", ODD) \
	X("?", READ) \
	X("!", WRITE) \
	X(":=", ASSIGN) \
	X(";", SEMICOLON) \
	X("=", EQUAL) \
	X("#", NOT_EQUAL) \
	X("<", LESS) \
	X("<=", LESS_OR_EQUAL) \
	X(">", GREATER) \
	X(">=", GREATER_OR_EQUAL) \
	X("+", ADD) \
	X("-", SUB) \
	X("*", MUL) \
	X("/", DIV) \
	X("(", LPAREN) \
	X(")", RPAREN) \
	X(",", COMMA) \
	X(".", PERIOD)\
\
	X("Program", PROGRAM) \
	X("Procedures", PROCS) \
	X("Block", BLOCK) \
	X("Error", ERROR) \
	X("End of input", END_OF_INPUT) \
	X("Identifier", IDENT) \
	X("Number", NUMBER)

#define AS_STRING_ARRAY(a,b) [b] = a,
#define AS_ENUM(a,b) b,

#define LAST_ATOM PERIOD

typedef enum symbol {
	SYMBOLS(AS_ENUM)
} Symbol;

extern const char *symbol_strings[];

#define MAX_IDENT_LEN 32
extern char ident_buf[MAX_IDENT_LEN];
extern int ident_buf_len;
extern long int number_scan;
extern Symbol scan_type;

int next(const char *);

#endif
