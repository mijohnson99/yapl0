#include "lexer.h"

const char *symbol_strings[] = {
	SYMBOLS(AS_STRING_ARRAY)
};

char ident_buf[MAX_IDENT_LEN];
int ident_buf_len = 0;
long int number_scan;
Symbol scan_type = END_OF_INPUT;

static inline bool is_alpha(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
static inline bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

// Identifier handling
static inline bool is_ident(const char *src)
{
	return is_alpha(src[0]) || src[0] == '_';
}
static int lex_ident(const char *src)
{
	int l;
	for (l = 0; is_ident(&src[l]) || is_digit(src[l]); l++)
		ident_buf[l] = src[l];
	ident_buf[l] = '\0';
	ident_buf_len = l;
	return l;
}

// Number handling
static inline bool is_number(const char *src)
{
	return is_digit(src[0]);
}
static int lex_number(const char *src)
{
	int l;
	number_scan = 0;
	for (l = 0; is_digit(src[l]); l++) {
		number_scan *= 10;
		number_scan += src[l] - '0';
	}
	return l;
}

// The lexical analyzer itself
static const char *symbol_strs[] = {
	SYMBOLS(AS_STRING_ARRAY)
};
static int prefix(const char *p, const char *s)
{ // Returns length of p if p is p prefix of s, 0 otherwise
	int i = 0;
	while (p[i] && p[i] == s[i])
		i++;
	return !p[i] ? i : 0;
}
static int leading_space(const char *src)
{
	int i = 0;
	while (src[i] && src[i] <= ' ')
		i++;
	return i;
}
int next(const char *src)
{ // Returns length of first token
	int len = 0;
	len += leading_space(src);
	src += len;
	if (!src[0]) {
		scan_type = END_OF_INPUT;
		return 0;
	}
	for (Symbol i = (Symbol)0; i <= LAST_ATOM; i++) {
		int p = prefix(symbol_strs[i], src);
		if (p) {
			len += p;
			scan_type = i;
			return len;
		}
	}
	if (is_ident(src)) {
		len += lex_ident(src);
		scan_type = IDENT;
		return len;
	} else if (is_number(src)) {
		len += lex_number(src);
		scan_type = NUMBER;
		return len;
	}
	scan_type = ERROR;
	return 0;
}
