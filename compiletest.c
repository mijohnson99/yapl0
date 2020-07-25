#include <stdio.h>
#include <stdlib.h>
#include "src/parser.h"

void compile_node(AST_Node *n)
{
	switch (n->type) {
	case NUMBER:
		printf("\tpush\t%ld\n", n->val.number);
		break;
	case IDENT:
		printf("\tget\t%s\n", n->val.ident);
		break;
	case VAR:
		for (int i = 0; i < n->val.count; i++)
			printf("%s:\tres 1\n",
					n->sub[i]->val.ident);
		break;
	case CONST:
		for (int i = 0; i < n->val.count; i++)
			printf("%s equ %ld\n",
					n->sub[i]->sub[0]->val.ident,
					n->sub[i]->sub[1]->val.number);
		break;
	case PROGRAM:
		compile_node(n->sub[0]->sub[0]); // Consts
		compile_node(n->sub[0]->sub[1]); // Vars
		compile_node(n->sub[0]->sub[2]); // Procedures
		printf("_start:\n");
		compile_node(n->sub[0]->sub[3]); // Block statement
		printf("\tret\n");
		break;
	case PROCEDURE:
		compile_node(n->sub[1]->sub[0]); // Consts
		compile_node(n->sub[1]->sub[1]); // Vars
		compile_node(n->sub[1]->sub[2]); // Procedures
		printf("%s:\n", n->sub[0]->val.ident);
		compile_node(n->sub[1]->sub[3]); // Block statement
		printf("\tret\n");
		break;
	case CALL:
		printf("\tcall\t%s\n", n->sub[0]->val.ident);
		break;
	case IF:
		printf(".if:");
		compile_node(n->sub[0]);
		printf("\tjz\t.end\n");
		compile_node(n->sub[1]);
		printf(".then:");
		break;
	case WHILE:
		printf(".while:");
		compile_node(n->sub[0]);
		printf("\tjz\t.end\n");
		compile_node(n->sub[1]);
		printf("\tjmp\t.while\n");
		printf(".end:");
		break;
	case ASSIGN:
		compile_node(n->sub[1]);
		printf("\tset\t%s\n", n->sub[0]->val.ident);
		break;
#define SIMPLE_CASE(OP, STR) \
	case OP: \
		compile_node(n->sub[0]); \
		compile_node(n->sub[1]); \
		printf("\t" STR "\n"); \
		break
	SIMPLE_CASE(ADD, "add");
	SIMPLE_CASE(SUB, "sub");
	SIMPLE_CASE(MUL, "mul");
	SIMPLE_CASE(DIV, "div");
	SIMPLE_CASE(EQUAL, "eq");
	SIMPLE_CASE(NOT_EQUAL, "neq");
	SIMPLE_CASE(LESS, "less");
	SIMPLE_CASE(GREATER, "grt");
	SIMPLE_CASE(LESS_OR_EQUAL, "lte");
	SIMPLE_CASE(GREATER_OR_EQUAL, "gte");
	default:
		for (int i = 0; i < n->val.count; i++)
			compile_node(n->sub[i]);
	}
}

int main()
{
	input_stream = stdin;
	AST_Node *prog = parse();
	if (prog) {
		compile_node(prog);
		free(prog);
	} else {
		printf("Parser encountered an error\n");
	}
	return 0;
}
