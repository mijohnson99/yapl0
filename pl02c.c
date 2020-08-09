#include <stdio.h>
#include <stdlib.h>
#include "src/parser.h"

void compile_node(AST_Node *n)
{
	switch (n->type) {
	case NUMBER:
		printf("%ld", n->val.number);
		break;
	case IDENT:
		printf("%s", n->val.ident);
		break;
	case VAR:
		for (int i = 0; i < n->val.count; i++)
			printf("long int %s;\n",
					n->sub[i]->val.ident);
		break;
	case CONST:
		for (int i = 0; i < n->val.count; i++)
			printf("const long int %s = %ld;\n",
					n->sub[i]->sub[0]->val.ident,
					n->sub[i]->sub[1]->val.number);
		break;
	case PROGRAM:
		printf("#include <stdio.h>\n");
		compile_node(n->sub[0]->sub[0]); // Consts
		compile_node(n->sub[0]->sub[1]); // Vars
		compile_node(n->sub[0]->sub[2]); // Procedures
		printf("int main()\n{\n");
		compile_node(n->sub[0]->sub[3]); // Block statement
		printf("return 0;\n}\n");
		break;
	case PROCEDURE:
		compile_node(n->sub[1]->sub[0]); // Consts
		compile_node(n->sub[1]->sub[1]); // Vars
		compile_node(n->sub[1]->sub[2]); // Procedures
		printf("void pl0_%s(void)\n{\n", n->sub[0]->val.ident);
		compile_node(n->sub[1]->sub[3]); // Block statement
		printf("}\n");
		break;
	case ODD:
		printf("(");
		compile_node(n->sub[0]);
		printf(")&1");
		break;
	case CALL:
		printf("pl0_%s();\n", n->sub[0]->val.ident);
		break;
	case IF:
		printf("if (");
		compile_node(n->sub[0]);
		printf(") {\n");
		compile_node(n->sub[1]);
		printf("}\n");
		break;
	case WHILE:
		printf("while (");
		compile_node(n->sub[0]);
		printf(") {\n");
		compile_node(n->sub[1]);
		printf("}\n");
		break;
	case READ:
		printf("scanf(\"%%ld\", &%s);\n", n->sub[0]->val.ident);
		break;
	case WRITE:
		printf("printf(\"%%ld\\n\", %s);\n", n->sub[0]->val.ident);
		break;
	case ASSIGN:
		printf("%s = ", n->sub[0]->val.ident);
		compile_node(n->sub[1]);
		printf(";\n");
		break;
#define SIMPLE_CASE(OP, STR) \
	case OP: \
		printf("("); \
		compile_node(n->sub[0]); \
		printf(") "STR" ("); \
		compile_node(n->sub[1]); \
		printf(")"); \
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
