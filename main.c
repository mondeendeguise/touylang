#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

enum token_type {
	// Reserve 0-255 for ASCII types

	TOKEN_IDENTIFIER = 256,
	TOKEN_NUMBER,
	TOKEN_STRING,

	TOKEN_PLUS_EQUALS,
	TOKEN_MINUS_EQUALS,
	TOKEN_TIMES_EQUALS,
	TOKEN_DIV_EQUALS,
	TOKEN_MOD_EQUALS,
	TOKEN_IS_EQUAL,
	TOKEN_IS_NOT_EQUAL,
	TOKEN_LOGICAL_AND,
	TOKEN_LOGICAL_OR,
	TOKEN_LESS_EQUALS,
	TOKEN_GREATER_EQUALS,

	TOKEN_DECL_EQUALS,
	TOKEN_DECL_DECL,

	TOKEN_PLUS_PLUS,
	TOKEN_MINUS_MINUS,

	TOKEN_SHIFT_LEFT,
	TOKEN_SHIFT_RIGHT,
	TOKEN_SHIFT_LEFT_EQUALS,
	TOKEN_SHIFT_RIGHT_EQUALS,

	TOKEN_BITWISE_AND_EQUALS,
	TOKEN_BITWISE_OR_EQUALS,
	TOKEN_BITWISE_XOR_EQUALS,

	TOKEN_MINUS_ARROW,
	TOKEN_EQUAL_ARROW,

	TOKEN_DOT_DOT,

	// Keywords

	TOKEN_KEYWORD_VOID,
	TOKEN_KEYWORD_CHAR,
	TOKEN_KEYWORD_INT,

	TOKEN_KEYWORD_U8,
	TOKEN_KEYWORD_U16,
	TOKEN_KEYWORD_U32,
	TOKEN_KEYWORD_U64,

	TOKEN_KEYWORD_S8,
	TOKEN_KEYWORD_S16,
	TOKEN_KEYWORD_S32,
	TOKEN_KEYWORD_S64,

	TOKEN_KEYWORD_FLOAT32,
	TOKEN_KEYWORD_FLOAT64,

	TOKEN_KEYWORD_ENUM,
	TOKEN_KEYWORD_STRUCT,
	TOKEN_KEYWORD_UNION,

	TOKEN_KEYWORD_DEFER,
	TOKEN_KEYWORD_RETURN,

	TOKEN_EOF,
	TOKEN_ERROR,
};

static const char *const TOKEN_STRINGS[] = {
	[TOKEN_PLUS_EQUALS] = "+=",
	[TOKEN_MINUS_EQUALS] = "-=",
	[TOKEN_TIMES_EQUALS] = "*=",
	[TOKEN_DIV_EQUALS] = "/=",
	[TOKEN_MOD_EQUALS] = "%=",
	[TOKEN_IS_EQUAL] = "==",
	[TOKEN_IS_NOT_EQUAL] = "!=",
	[TOKEN_LOGICAL_AND] = "&&",
	[TOKEN_LOGICAL_OR] = "||",
	[TOKEN_LESS_EQUALS] = "<=",
	[TOKEN_GREATER_EQUALS] = ">=",

	[TOKEN_DECL_EQUALS] = ":=",
	[TOKEN_DECL_DECL] = "::",

	[TOKEN_PLUS_PLUS] = "++",
	[TOKEN_MINUS_MINUS] = "--",

	[TOKEN_SHIFT_LEFT] = "<<",
	[TOKEN_SHIFT_RIGHT] = ">>",
	[TOKEN_SHIFT_LEFT_EQUALS] = "<<=",
	[TOKEN_SHIFT_RIGHT_EQUALS] = ">>=",

	[TOKEN_BITWISE_AND_EQUALS] = "&=",
	[TOKEN_BITWISE_OR_EQUALS] = "|=",
	[TOKEN_BITWISE_XOR_EQUALS] = "^=",

	[TOKEN_MINUS_ARROW] = "->",
	[TOKEN_EQUAL_ARROW] = "=>",

	[TOKEN_DOT_DOT] = "..",

	[TOKEN_KEYWORD_VOID] = "void",
	[TOKEN_KEYWORD_CHAR] = "char",
	[TOKEN_KEYWORD_INT] = "int",

	[TOKEN_KEYWORD_U8] = "u8",
	[TOKEN_KEYWORD_U16] = "u16",
	[TOKEN_KEYWORD_U32] = "u32",
	[TOKEN_KEYWORD_U64] = "u64",

	[TOKEN_KEYWORD_S8] = "s8",
	[TOKEN_KEYWORD_S16] = "s16",
	[TOKEN_KEYWORD_S32] = "s32",
	[TOKEN_KEYWORD_S64] = "s64",

	[TOKEN_KEYWORD_FLOAT32] = "float32",
	[TOKEN_KEYWORD_FLOAT64] = "float64",

	[TOKEN_KEYWORD_ENUM] = "enum",
	[TOKEN_KEYWORD_STRUCT] = "struct",
	[TOKEN_KEYWORD_UNION] = "union",

	[TOKEN_KEYWORD_DEFER] = "defer",
	[TOKEN_KEYWORD_RETURN] = "return",
};

struct string_view {
	const char *items;
	size_t count;
};

#define SV_FMT "%.*s"
#define SV_ARG(sv) (int) (sv).count, (sv).items

struct token {
	uint32_t type;

	int32_t l0;
	int32_t c0;

	int32_t l1;
	int32_t c1;

	union {
		uint64_t integer_value;
		float float32_value;
		double float64_value;
		struct string_view str;
	};

	uint32_t number_flags;
};

struct tokenizer {
	char *buffer;
	uint64_t buffer_size;
	uint64_t offset;
	int32_t line;
	int32_t col;

	struct token *tokens;
	size_t token_count;
};

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// TODO: bounds check buffer
// TODO: handle comments
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
			if(strncmp(t.str.items, TOKEN_STRINGS[i], MIN(t.str.count, strlen(TOKEN_STRINGS[i]))) == 0) {
				t.type = i;
				break;
			}
		}

		return t;
	}

	// Check for a number
	// TODO: check signedness, precision, base, etc.
	if(isdigit(*(p + *offset))) {
		t.type = TOKEN_NUMBER;
		t.str.count = 0;
		t.str.items = p + *offset;

		while(isdigit(*(p + *offset))) {
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

		t.integer_value = atol(tmp);

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

struct token peek_token(struct tokenizer *tk) {
	char *p = tk->buffer;
	uint64_t offset = tk->offset;
	int32_t line = tk->line;
	int32_t col = tk->col;

	return get_token(p, &offset, &line, &col);
}

struct token consume_token(struct tokenizer *tk) {
	return get_token(tk->buffer, &tk->offset, &tk->line, &tk->col);
}

// TODO: read file from argv
int main(void) {
	struct tokenizer tk = {0};

	FILE *fp = fopen("test.touy", "rb");
	fseek(fp, 0L, SEEK_END);
	tk.buffer_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	tk.buffer = malloc(tk.buffer_size);
	fread(tk.buffer, 1, tk.buffer_size, fp);
	fclose(fp);

	struct token t = consume_token(&tk);

	while(t.type != TOKEN_EOF) {
		// printf("%d\n", t.type);
		if(t.type == TOKEN_ERROR) {
			fprintf(stderr, "failed to parse token at %d:%d\n", t.l0, t.c0);
			break;
		} else if(t.type == TOKEN_IDENTIFIER) {
			printf("%d:%d:" SV_FMT "\n", t.l0, t.c0, SV_ARG(t.str));
		} else if(t.type == TOKEN_NUMBER) {
			printf("%d:%d:number(%ld)\n", t.l0, t.c0, t.integer_value);
		} else if(t.type > TOKEN_STRING && t.type < TOKEN_KEYWORD_VOID) {
			printf("%d:%d:(%s)\n", t.l0, t.c0, TOKEN_STRINGS[t.type]);
		} else if(t.type >= TOKEN_KEYWORD_VOID && t.type < TOKEN_EOF) {
			printf("%d:%d:keyword(%s)\n", t.l0, t.c0, TOKEN_STRINGS[t.type]);
		} else if(t.type < 256) {
			printf("%d:%d:%c\n", t.l0, t.c0, t.type);
		} else {
			printf("%d:%d:%d\n", t.l0, t.c0, t.type);
		}
		t = consume_token(&tk);
	}

	free(tk.buffer);
	return 0;
}
