#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// TODO: bounds check buffer
struct token get_token(const char *p, uint64_t *offset, int32_t *line, int32_t *col) {
	struct token t = {0};

	while(isspace(*(p + *offset))) {
		if(*(p + *offset) == '\n') {
			++(*line);
			*col = 0;
		} else {
			++(*col);
		}
		++(*offset);
	}

	// TODO: decide whether line, col should start at 0 or 1 internally
	t.l0 = (*line) + 1;
	t.c0 = (*col) + 1;

	t.l1 = -1;
	t.c1 = -1;

	// Check for an identifier
	if(isalpha(*(p + *offset)) || *(p + *offset) == '_') {
		t.type = TOKEN_IDENTIFIER;
		t.str.items = p + *offset;
		t.str.count = 0;

		while(isalnum(*(p + *offset)) || *(p + *offset) == '_') {
			++t.str.count;
			++(*col);
			++(*offset);
		}
		t.c1 = *col;

		// Check for a keyword
		for(size_t i = TOKEN_KEYWORD_VOID; i < TOKEN_EOF; ++i) {
			if(strncmp(t.str.items, TOKEN_STRINGS[i], MAX(t.str.count, strlen(TOKEN_STRINGS[i]))) == 0) {
				t.type = i;
				break;
			}
		}

		return t;
	}

	// Check for a number
	// TODO: check precision, base
	if(isdigit(*(p + *offset))) {
		t.type = TOKEN_NUMBER;
		t.str.count = 0;
		t.str.items = p + *offset;

		int is_float = 0;
		while(isdigit(*(p + *offset)) || *(p + *offset) == '.') {
			if(*(p + *offset) == '.') {
				if(is_float) {
					// TODO: better error handling
					//       print filename
					fprintf(stderr, "filename:%d:%d: unexpected '.'\n", *line+1, *col+1);
					t.type = TOKEN_ERROR;
					return t;
				}
				is_float = 1;
			}
			++t.str.count;
			++(*col);
			++(*offset);
		}
		t.c1 = *col;

		// TODO: get rid of this malloc
		//       clean this up somehow
		char *tmp = malloc(t.str.count+1);
		memcpy(tmp, t.str.items, t.str.count);
		tmp[t.str.count] = '\0';

		// TODO: more robust number flags/information tracking
		if(is_float) {
			t.number_flags |= TOKEN_NUMBER_FLAG_FLOAT;
			t.float32_value = strtof(tmp, NULL);
			t.float64_value = strtod(tmp, NULL);
		} else {
			t.integer_value = atol(tmp);
			if(t.integer_value > UINT8_MAX) t.number_flags |= TOKEN_NUMBER_FLAG_MIN_8_BIT;
			if(t.integer_value > UINT16_MAX) t.number_flags |= TOKEN_NUMBER_FLAG_MIN_16_BIT;
			if(t.integer_value > UINT32_MAX) t.number_flags |= TOKEN_NUMBER_FLAG_MIN_32_BIT;
			if(t.integer_value > UINT64_MAX) t.number_flags |= TOKEN_NUMBER_FLAG_MIN_64_BIT;
		}

		free(tmp);

		return t;
	}

	// Check for ASCII byte tokens
	// TODO: bounds check buffer
	char c = *(p + *offset);
	switch(c) {
	case 0:
	case EOF:
		++(*offset);
		++(*col);
		t.type = TOKEN_EOF;
		break;

	case '$': // TODO: find a use case for '$'
	case '#':
	case '?':
	case '^':

	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':

	case '\'':
	case '.':
	case ',':
	case ';':
		++(*offset);
		++(*col);
		t.type = c;
		break;

	// These tokens can be modified with additional characters
	// e.g. "x >> y" vs "x >>= y"

	case ':': {
			char c1 = *(p + *offset + 1);
			switch(c1) {
			case '=':
				++(*offset);
				++(*col);
				t.type = TOKEN_DECL_EQUALS;
				break;

			case ':':
				++(*offset);
				++(*col);
				t.type = TOKEN_DECL_DECL;
				break;

			default:
				t.type = c;
				break;
			}

			++(*offset);
			++(*col);

		} break;

	case '=': {
			char c1 = *(p + *offset + 1);
			switch(c1) {
			case '=':
				++(*offset);
				++(*col);
				t.type = TOKEN_IS_EQUAL;
				break;

			case '>':
				++(*offset);
				++(*col);
				t.type = TOKEN_EQUAL_ARROW;
				break;

			default:
				// TODO: add string token type
				t.type = c;
				break;
			}
			++(*offset);
			++(*col);

		} break;
		
	// TODO: check for multi-char tokens
	case '+':
	case '-': {
			char c1 = *(p + *offset + 1);
			switch(c1) {
			case '-':
				*offset += 2;
				*col += 2;
				t.type = TOKEN_MINUS_MINUS;
				break;

			case '=':
				*offset += 2;
				*col += 2;
				t.type = TOKEN_MINUS_EQUALS;
				break;

			case '>':
				*offset += 2;
				*col += 2;
				t.type = TOKEN_MINUS_ARROW;
				break;
			}
		} break;
	case '*':
	case '%':
	case '/': {
			char c1 = *(p + *offset + 1);
			switch(c1) {
			case '=':
				*offset += 2;
				*col += 2;
				t.type = TOKEN_DIV_EQUALS;
				break;

			case '/':
				*offset += 2;
				*col += 2;

				// newline should be trimmed by subsequent call
				// TODO: bounds check buffer
				while(c != '\n') {
					++(*offset);
					++(*col);
					c = *(p + *offset);
				}

				return get_token(p, offset, line, col);

			case '*':
				// TODO: bounds check buffer
				// TODO: check for nesting
				*offset += 2;
				*col += 2;
				
				while(c != '*' && c1 != '/') {
					++(*offset);
					c = *(p + *offset);
					c1 = *(p + *offset + 1);
					if(c == '\n') {
						++(*line);
						*col = 1;
					} else {
						++(*col);
					}
				}

				return get_token(p, offset, line, col);

			default:
				++(*offset);
				++(*col);
				t.type = c;
				break;
			}

		} break;

	case '|':
	case '&':
	case '~':

	case '!':
	case '<':
	case '>':

	case '\"':
		++(*offset);
		++(*col);
		t.type = TOKEN_ERROR;
		fprintf(stderr, "unhandled token: %c\n", c);
		break;

	default:
		++(*offset);
		++(*col);
		t.type = TOKEN_ERROR;
		fprintf(stderr, "unhandled token: %d\n", c);
		break;
	}

	return t;
}

struct token peek_token(struct lexer *l) {
	char *p = l->buffer;
	uint64_t offset = l->offset;
	int32_t line = l->line;
	int32_t col = l->col;

	return get_token(p, &offset, &line, &col);
}

struct token consume_token(struct lexer *l) {
	return get_token(l->buffer, &l->offset, &l->line, &l->col);
}
