#include <stdio.h>
#include <stdlib.h>
#include "src/parser.h"

void newline(int depth)
{
	putchar('\n');
	while (depth --> 0)
		putchar('\t');
}

void compile_node(AST_Node *n, int depth)
{
	switch (n->type) {
	case NUMBER:
		printf("%ld", n->val.number);
		break;
	case IDENT:
		printf("%s", n->val.ident);
		break;
	case VAR:
		for (int i = 0; i < n->val.count; i++) {
			newline(depth);
			printf("long int %s;",
					n->sub[i]->val.ident);
		}
		break;
	case CONST:
		for (int i = 0; i < n->val.count; i++) {
			newline(depth);
			printf("const long int %s = %ld;",
					n->sub[i]->sub[0]->val.ident,
					n->sub[i]->sub[1]->val.number);
		}
		break;
	case PROGRAM:
		printf("#include <stdio.h>");
		compile_node(n->sub[0]->sub[0], depth); // Consts
		compile_node(n->sub[0]->sub[1], depth); // Vars
		compile_node(n->sub[0]->sub[2], depth); // Procedures
		newline(depth);
		printf("int main()");
		newline(depth);
		printf("{");
		compile_node(n->sub[0]->sub[3], depth+1); // Block statement
		newline(depth+1);
		printf("return 0;");
		newline(depth);
		printf("}");
		newline(depth);
		break;
	case PROCEDURE:
		newline(depth);
		compile_node(n->sub[1]->sub[0], depth); // Consts
		compile_node(n->sub[1]->sub[1], depth); // Vars
		compile_node(n->sub[1]->sub[2], depth); // Procedures
		newline(depth);
		printf("void pl0_%s(void)", n->sub[0]->val.ident);
		newline(depth);
		printf("{");
		compile_node(n->sub[1]->sub[3], depth+1); // Block statement
		newline(depth);
		printf("}");
		break;
	case ODD:
		printf("(");
		compile_node(n->sub[0], depth);
		printf(")&1");
		break;
	case CALL:
		newline(depth);
		printf("pl0_%s();", n->sub[0]->val.ident);
		break;
	case IF:
		newline(depth);
		printf("if (");
		compile_node(n->sub[0], depth);
		printf(") {");
		compile_node(n->sub[1], depth+1);
		newline(depth);
		printf("}");
		break;
	case WHILE:
		newline(depth);
		printf("while (");
		compile_node(n->sub[0], depth);
		printf(") {");
		compile_node(n->sub[1], depth+1);
		newline(depth);
		printf("}");
		break;
	case READ:
		newline(depth);
		printf("scanf(\"%%ld\", &%s);", n->sub[0]->val.ident);
		break;
	case WRITE:
		newline(depth);
		printf("printf(\"%%ld\\n\", %s);", n->sub[0]->val.ident);
		break;
	case ASSIGN:
		newline(depth);
		printf("%s = ", n->sub[0]->val.ident);
		compile_node(n->sub[1], depth);
		printf(";");
		break;
#define SIMPLE_CASE(OP, STR) \
	case OP: \
		compile_node(n->sub[0], depth); \
		printf(" "STR" "); \
		compile_node(n->sub[1], depth); \
		break
	SIMPLE_CASE(ADD, "+");
	SIMPLE_CASE(SUB, "-");
	SIMPLE_CASE(MUL, "*");
	SIMPLE_CASE(DIV, "/");
	SIMPLE_CASE(EQUAL, "==");
	SIMPLE_CASE(NOT_EQUAL, "!=");
	SIMPLE_CASE(LESS, "<");
	SIMPLE_CASE(GREATER, ">");
	SIMPLE_CASE(LESS_OR_EQUAL, "<=");
	SIMPLE_CASE(GREATER_OR_EQUAL, ">=");
	default:
		for (int i = 0; i < n->val.count; i++)
			compile_node(n->sub[i], depth);
	}
}
#define compile_program(X) compile_node(X, 0)

int main()
{
	input_stream = stdin;
	AST_Node *prog = parse();
	if (prog) {
		compile_program(prog);
		free(prog);
	} else {
		printf("Parser encountered an error\n");
	}
	return 0;
}
