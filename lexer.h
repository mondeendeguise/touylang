#ifndef _LEXER_H
#define _LEXER_H

#include "string_view.h"

#include <stddef.h>
#include <stdint.h>

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

	[TOKEN_KEYWORD_FLOAT32] = "f32",
	[TOKEN_KEYWORD_FLOAT64] = "f64",

	[TOKEN_KEYWORD_ENUM] = "enum",
	[TOKEN_KEYWORD_STRUCT] = "struct",
	[TOKEN_KEYWORD_UNION] = "union",

	[TOKEN_KEYWORD_DEFER] = "defer",
	[TOKEN_KEYWORD_RETURN] = "return",
};

enum token_number_flags {
	TOKEN_NUMBER_FLAG_MIN_8_BIT  = 1,
	TOKEN_NUMBER_FLAG_MIN_16_BIT = 2,
	TOKEN_NUMBER_FLAG_MIN_32_BIT = 4,
	TOKEN_NUMBER_FLAG_MIN_64_BIT = 8,
	TOKEN_NUMBER_FLAG_SIGNED     = 16,
	TOKEN_NUMBER_FLAG_FLOAT      = 32,
};

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

struct lexer {
	char *buffer;
	uint64_t buffer_size;
	uint64_t offset;
	int32_t line;
	int32_t col;

	struct token *tokens;
	size_t token_count;
	size_t token_current;
};

struct token get_token(const char *p, uint64_t *offset, int32_t *line, int32_t *col);
struct token peek_token(struct lexer *l);
struct token consume_token(struct lexer *l);

void token_error(struct token t, int32_t line, int32_t col, const char *fmt, ...);

#endif // _LEXER_H
