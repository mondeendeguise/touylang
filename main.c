#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>

// TODO: store tokens(?) for AST construction
//
//    A: do one pass of all the tokens in the file to get a count,
//       allocate a buffer, then rewind to do a second pass where we actually
//       store the tokens
//
//    B: do one pass with a dynamically resizing buffer of tokens
//       maybe after reaching EOF, dealloc unused memory? not necessary though

// TODO: codegen

// TODO: read file from argv
int main(void) {
	struct lexer l = {0};

	FILE *fp = fopen("test.touy", "rb");
	fseek(fp, 0L, SEEK_END);
	l.buffer_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	l.buffer = malloc(l.buffer_size);
	fread(l.buffer, 1, l.buffer_size, fp);
	fclose(fp);

	struct token t = consume_token(&l);

	while(t.type != TOKEN_EOF) {
		// printf("%d\n", t.type);
		if(t.type == TOKEN_ERROR) {
			fprintf(stderr, "failed to parse token at %d:%d\n", t.l0, t.c0);
			break;
		} else if(t.type == TOKEN_IDENTIFIER) {
			// printf("%d:%d:" SV_FMT "\n", t.l0, t.c0, SV_ARG(t.str));
			printf("%d\t%d\tIdentifier\t" SV_FMT "\n", t.l0, t.c0, SV_ARG(t.str));
		} else if(t.type == TOKEN_STRING) {
			printf("%d\t%d\tString    \t" SV_FMT "\n", t.l0, t.c0, SV_ARG(t.str));
		} else if(t.type == TOKEN_NUMBER) {
			// printf("%d:%d:number(%ld)\n", t.l0, t.c0, t.integer_value);
			if(t.number_flags & TOKEN_NUMBER_FLAG_FLOAT) {
				// TODO: figure out why float32_value keeps being 0 here
				printf("%d\t%d\tFloat     \t%f\n", t.l0, t.c0, t.float64_value);
			} else {
				printf("%d\t%d\tInteger   \t%ld\n", t.l0, t.c0, t.integer_value);
			}
		} else if(t.type > TOKEN_STRING && t.type < TOKEN_KEYWORD_VOID) {
			// printf("%d:%d:(%s)\n", t.l0, t.c0, TOKEN_STRINGS[t.type]);
			printf("%d\t%d\t%s\n", t.l0, t.c0, TOKEN_STRINGS[t.type]);
		} else if(t.type >= TOKEN_KEYWORD_VOID && t.type < TOKEN_EOF) {
			// printf("%d:%d:keyword(%s)\n", t.l0, t.c0, TOKEN_STRINGS[t.type]);
			printf("%d\t%d\t%s\n", t.l0, t.c0, TOKEN_STRINGS[t.type]);
		} else if(t.type < 256) {
			// printf("%d:%d:%c\n", t.l0, t.c0, t.type);
			printf("%d\t%d\t%c\n", t.l0, t.c0, t.type);
		} else {
			// printf("%d:%d:%d\n", t.l0, t.c0, t.type);
			printf("%d\t%d\t%d\n", t.l0, t.c0, t.type);
		}
		t = consume_token(&l);
	}

	free(l.buffer);
	return 0;
}
