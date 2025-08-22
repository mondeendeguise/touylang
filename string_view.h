#ifndef _STRING_VIEW_H
#define _STRING_VIEW_H

#include <stddef.h>

struct string_view {
	const char *items;
	size_t count;
};

#define SV_FMT "%.*s"
#define SV_ARG(sv) (int) (sv).count, (sv).items

#endif // _STRING_VIEW_H
