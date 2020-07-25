#include <stdio.h>
#include <stdlib.h>
#include "src/lexer.h"

void print_lookahead(void)
{
	switch (scan_type) {
	case ERROR:
		printf("Error.\n");
		break;
	case END_OF_INPUT:
		break;
	case IDENT:
		printf("Identifier: \"%s\"\n", ident_buf);
		break;
	case NUMBER:
		printf("Number: %ld\n", number_scan);
		break;
	default:
		printf("Symbol: %s\n", symbol_strings[scan_type]);
		break;
	}
}

int main()
{
	char buf[100];
	while (fgets(buf, sizeof(buf), stdin)) {
		char *in = buf;
		do {
			int l = next(in);
			in += l;
			if (scan_type == ERROR) {
				printf("Error at %s\n", in);
				exit(1);
			}
			print_lookahead();
		} while (scan_type != END_OF_INPUT);
	}
}
