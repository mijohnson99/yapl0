#include <stdio.h>
#include <stdlib.h>
#include "src/parser.h"

void print_node(AST_Node *n)
{
	switch (n->type) {
	case NUMBER:
		printf("%ld", n->val.number);
		break;
	case IDENT:
		printf("%s", n->val.ident);
		break;
	default:
		printf("(%s", symbol_strings[n->type]);
		for (int i = 0; i < n->val.count; i++) {
			putchar(' ');
			print_node(n->sub[i]);
		}
		printf(")");
		break;
	}
}

int main()
{
	input_stream = stdin;
	AST_Node *prog = parse();
	if (prog) {
		printf("Parsing completed successfully\n");
		print_node(prog);
		putchar('\n');
		free(prog);
	} else {
		printf("Parser encountered an error\n");
	}
	return 0;
}
