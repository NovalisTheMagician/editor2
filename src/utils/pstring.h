#pragma once

#include <stdlib.h>
#include <stdarg.h>

typedef char *pstring;

struct string_header
{
    size_t size;
    size_t length;
};

#define string(str) string_cstr(str)

pstring string_alloc(size_t capacity);
pstring string_cstr(const char str[static 1]);
pstring string_cstr_alloc(const char str[static 1], size_t size);
pstring string_cstr_size(size_t size, const char str[static size]);

void string_free(pstring str);

size_t string_format(pstring into, const char format[static 1], ...);
size_t string_vformat(pstring into, const char format[static 1], va_list args);

size_t string_size(pstring str);
size_t string_length(pstring str);

pstring string_copy(pstring str);
pstring string_substring(pstring str, size_t start, ssize_t end);

ssize_t string_first_index_of(pstring str, size_t offset, const char tok[static 1]);
ssize_t string_last_index_of(pstring str, size_t offset, const char tok[static 1]);
pstring* string_split(pstring str, const char tok[static 1]);

int string_cmp(pstring a, pstring b);
int string_icmp(pstring a, pstring b);

void string_toupper(pstring str);
void string_tolower(pstring str);

struct stringtok
{
    pstring source;
    size_t sourcelen;
    const char *delim;
    size_t next;
    char *buffer;
    size_t buffercap;
    int done;
};

struct stringtok* stringtok_start(pstring source, const char delim[static 1]);
char* stringtok_next(struct stringtok tok[static 1], size_t *numChars);
void stringtok_reset(struct stringtok tok[static 1]);
void stringtok_end(struct stringtok *tok);
