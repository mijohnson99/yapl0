#include "parser.h"

// Local zone-based allocator
static void *parse_zone = NULL;
static inline void *zalloc(signed int s)
{
	return zalloc_r(s, &parse_zone);
}

// Input functions
FILE *input_stream;
static char linebuf[200];
static char *line = NULL;
static inline void advance(void)
{
	// Advances the lexer position by one token
	line += next(line);
}
bool refill(void)
{
	bool b = fgets(linebuf, sizeof(linebuf), input_stream);
	line = linebuf;
	advance();
	return b;
}

// Parser utilities
bool accept(Symbol s)
{
	while (scan_type == END_OF_INPUT)
		refill();
	if (scan_type == s) {
		advance();
		return true;
	}
	return false;
}
bool expect(Symbol s)
{
	if (accept(s))
		return true;
	fprintf(stderr, "Unexpected symbol (Wanted `%s`, got `%s`)\n",
			symbol_strings[s], symbol_strings[scan_type]);
	fprintf(stderr, "On line:\t%s", linebuf);
	return false;
}

// Node utility functions (allocation, etc.)
static inline AST_Node *new_node(Symbol t)
{
	AST_Node *node = zalloc(sizeof(AST_Node));
	node->type = t;
	node->val.count = 0;
	return node;
}
static inline AST_Node *new_ident(char *s)
{
	AST_Node *node = new_node(IDENT);
	node->val.ident = s;
	return node;
}
static inline AST_Node *new_number(long int n)
{
	AST_Node *node = new_node(NUMBER);
	node->val.number = n;
	return node;
}
static inline void add_child(AST_Node *parent, AST_Node *child)
{
	// Takes advantage of zone-based allocation and AST_Node structure.
	// Parameter `child` should be allocated BEFORE parent.
	parent->val.count++;
	*(AST_Node **)zalloc(sizeof(AST_Node *)) = child;
}

/*
 *		Recursive Descent Parser Begins
 *			--- H E R E ---
 */

AST_Node *program(void);
AST_Node *block(void);
AST_Node *statement(void);
AST_Node *condition(void);
AST_Node *expression(void);
AST_Node *term(void);
AST_Node *factor(void);

AST_Node *ident(void)
{
	while (scan_type == END_OF_INPUT)
		refill();
	if (scan_type != IDENT)
		return NULL;
	char *s = zalloc(ident_buf_len + 1);
	for (int i = 0; i <= ident_buf_len; i++)
		s[i] = ident_buf[i];
	advance();
	return new_ident(s);
}
AST_Node *number(void)
{
	if (scan_type != NUMBER)
		return NULL;
	long int n = number_scan;
	advance();
	return new_number(n);
}

#define MAX_PROG 16384
AST_Node *parse(void)
{
	void *parse_start = malloc(MAX_PROG * sizeof(AST_Node));
	parse_zone = parse_start;
	AST_Node *ret = new_node(PROGRAM);
	add_child(ret, NULL); // Placeholder spot
	AST_Node *prog = program();
	if (prog) {
		ret->sub[0] = prog;
		ret = realloc(ret, parse_zone - parse_start);
		parse_zone = NULL;
		return ret;
	} else {
		free(parse_start);
		parse_zone = NULL;
		return NULL;
	}
}

// program = block "." .
AST_Node *program(void)
{
	AST_Node *b = block();
	if (b && expect(PERIOD)) {
		return b;
	} else {
		return NULL;
	}
}

// block = [ "const" ident "=" number {"," ident "=" number} ";"]
//         [ "var" ident {"," ident} ";"]
//         { "procedure" ident ";" block ";" } statement .
#define MAX_VARS 64
#define MAX_CONSTS MAX_VARS
#define MAX_PROCS MAX_VARS
AST_Node *assign(AST_Node *id, AST_Node *rhs)
{
	AST_Node *n = new_node(ASSIGN);
	add_child(n, id);
	add_child(n, rhs);
	return n;
}
AST_Node *block_consts(void)
{
	int n = 0;
	AST_Node *defs[MAX_CONSTS];
	if (accept(CONST)) {
		do {
			AST_Node *id = ident();
			expect(EQUAL);
			AST_Node *num = number();
			if (id && num)
				defs[n++] = assign(id, num);
		} while (accept(COMMA));
		expect(SEMICOLON);
	}

	AST_Node *node = new_node(CONST);
	for (int i = 0; i < n; i++)
		add_child(node, defs[i]);
	return node;
}
AST_Node *block_vars(void)
{
	int n = 0;
	AST_Node *ids[MAX_CONSTS];
	if (accept(VAR)) {
		do {
			AST_Node *id = ident();
			if (id)
				ids[n++] = id;
		} while (accept(COMMA));
		expect(SEMICOLON);
	}

	AST_Node *node = new_node(VAR);
	for (int i = 0; i < n; i++)
		add_child(node, ids[i]);
	return node;
}
AST_Node *procedure_def(void)
{
	AST_Node *node, *id, *body;
	if (!accept(PROCEDURE))
		return NULL;
	id = ident();
	expect(SEMICOLON);
	body = block();
	expect(SEMICOLON);

	node = new_node(PROCEDURE);
	add_child(node, id);
	add_child(node, body);
	return node;
}
AST_Node *block_procs(void)
{
	int n = 0;
	AST_Node *procs[MAX_PROCS];
	for (;;) {
		AST_Node *p = procedure_def();
		if (!p)
			break;
		else
			procs[n++] = p;
	}

	AST_Node *node = new_node(PROCS);
	for (int i = 0; i < n; i++)
		add_child(node, procs[i]);
	return node;
}
AST_Node *block(void)
{
	AST_Node *loc_consts = block_consts();
	AST_Node *loc_vars = block_vars();
	AST_Node *loc_procs = block_procs();
	AST_Node *body = statement();

	AST_Node *node = new_node(BLOCK);
	add_child(node, loc_consts);
	add_child(node, loc_vars);
	add_child(node, loc_procs);
	add_child(node, body);
	return node;
}

// statement = [ ident ":=" expression
//               | "call" ident
//               | "?" ident
//               | "!" expression
//               | "begin" statement {";" statement } "end"
//               | "if" condition "then" statement
//               | "while" condition "do" statement ].
#define MAX_STATEMENTS MAX_VARS
AST_Node *statement(void)
{
	AST_Node *node = ident();
	if (node) {
		AST_Node *name = node;
		expect(ASSIGN);
		AST_Node *expr = expression();

		node = new_node(ASSIGN);
		add_child(node, name);
		add_child(node, expr);
	} else if (accept(CALL)) {
		AST_Node *name = ident();

		node = new_node(CALL);
		add_child(node, name);
	} else if (accept(READ)) {
		AST_Node *name = ident();

		node = new_node(READ);
		add_child(node, name);
	} else if (accept(WRITE)) {
		AST_Node *expr = expression();

		node = new_node(WRITE);
		add_child(node, expr);
	} else if (accept(BEGIN)) {
		int n = 0;
		AST_Node *stmts[MAX_STATEMENTS];
		do {
			stmts[n++] = statement();
		} while (accept(SEMICOLON));
		expect(END);

		node = new_node(BEGIN);
		for (int i = 0; i < n; i++)
			add_child(node, stmts[i]);
	} else if (accept(IF)) {
		AST_Node *cond = condition();
		expect(THEN);
		AST_Node *stmt = statement();

		node = new_node(IF);
		add_child(node, cond);
		add_child(node, stmt);
	} else if (accept(WHILE)) {
		AST_Node *cond = condition();
		expect(DO);
		AST_Node *stmt = statement();

		node = new_node(WHILE);
		add_child(node, cond);
		add_child(node, stmt);
	} else
		printf("statement(): MALFORMED\n");
	return node;
}

// condition = "odd" expression |
//             expression ("="|"#"|"<"|"<="|">"|">=") expression .
AST_Node *condition(void)
{
	AST_Node *node = NULL;
	if (accept(ODD)) {
		AST_Node *expr = expression();

		node = new_node(ODD);
		add_child(node, expr);
	} else {
		AST_Node *lhs = expression();

		Symbol op = (Symbol)0;
		for (Symbol i = EQUAL; i <= GREATER; i++) {
			if (accept(i)) {
				op = i;
				break;
			}
		}
		if (!op) {
			fprintf(stderr, "Unexpected symbol. (Wanted operator, got %s)\n",
					symbol_strings[scan_type]);
			return NULL;
		}
		AST_Node *rhs = expression();

		node = new_node(op);
		add_child(node, lhs);
		add_child(node, rhs);
	}
	return node;
}

// expression = [ "+"|"-"] term { ("+"|"-") term}.
/*
 * 	Translation plan:
 *	-1 + 2 - 3 + 4
 *	scan -   =>
 *	scan 1   => 1
 *	recall - => (- 0 1)
 *	scan + 2 => (+ (- 0 1) 2)
 *	scan - 3 => (- (+ (- 0 1) 2) 3)
 *	scan + 4 => (+ (- (+ (- 0 1) 2) 3) 4); finished
 */
AST_Node *expression(void)
{
	AST_Node *e;
	// Consume first term
	if (!accept(ADD) && accept(SUB)) {
		AST_Node *z = new_number(0);
		AST_Node *t = term();
		e = new_node(SUB);
		add_child(e, z);
		add_child(e, t);
	} else {
		e = term();
	}
	// Consume subsequent terms
	for (;;) {
		Symbol op;
		if (accept(ADD))
			op = ADD;
		else if (accept(SUB))
			op = SUB;
		else
			break;

		AST_Node *t = term();
		AST_Node *m = new_node(op);
		add_child(m, e);
		add_child(m, t);
		e = m;
	}
	return e;
}

// term = factor {("*"|"/") factor}.
AST_Node *term(void)
{
	AST_Node *t = factor();
	for (;;) {
		Symbol op;
		if (accept(MUL))
			op = MUL;
		else if (accept(DIV))
			op = DIV;
		else
			break;

		AST_Node *f = factor();
		AST_Node *m = new_node(op);
		add_child(m, t);
		add_child(m, f);
		t = m;
	}
	return t;
}

// factor =
//     ident
//     | number
//     | "(" expression ")" .
AST_Node *factor(void)
{
	AST_Node *node = NULL;
	if ((node = ident())) {
		return node;
	} else if ((node = number())) {
		return node;
	} else {
		expect(LPAREN);
		node = expression();
		expect(RPAREN);
		return node;
	}
	return NULL;
}
