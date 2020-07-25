#ifndef PARSER_H
#define PARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lexer.h"
#include "zalloc.h"

typedef struct ast_node {
	Symbol type;
	union {
		long int number, count;
		char *ident;
	} val;
	struct ast_node *sub[];
} AST_Node;
// ^ `(IDENT  ,val.as_string)
// | `(NUMBER ,val.as_int)
// | `(,type  ,@(sub)) ; with (eq (length sub) val.count)

extern FILE *input_stream;

bool refill(void);
AST_Node *parse(void);
// ^ Returns a pointer to a Lisp-like parse tree.
//
//   The returned tree structure uses zone-based allocation.
//   That means only the node returned by parse() needs to be freed,
//   because everything else is allocated contiguously after it.

#endif
