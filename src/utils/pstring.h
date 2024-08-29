#pragma once

#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>

typedef char *pstring;

struct string_header
{
    size_t size;
    size_t length;
};

#define string(str) string_cstr(str)

pstring string_alloc(size_t capacity);
pstring string_cstr(const char *str);
pstring string_cstr_alloc(const char *str, size_t size);
pstring string_cstr_size(size_t size, const char str[static size]);

void string_free(pstring str);

size_t string_format(pstring into, const char *format, ...);
size_t string_vformat(pstring into, const char *format, va_list args);

size_t string_format_offset(pstring into, size_t offset, const char *format, ...);
size_t string_vformat_offset(pstring into, size_t offset, const char *format, va_list args);

size_t string_size(pstring str);
size_t string_length(pstring str);
void string_recalc(pstring str);

pstring string_copy(pstring str);
size_t string_copy_into_cstr(pstring into, const char *str);
size_t string_copy_into(pstring into, pstring str);
pstring string_substring(pstring str, size_t start, ssize_t end);

ssize_t string_first_index_of(pstring str, size_t offset, const char *tok);
ssize_t string_last_index_of(pstring str, size_t offset, const char *tok);
pstring* string_split(pstring str, const char *tok);

int string_cmp(pstring a, pstring b);
int string_icmp(pstring a, pstring b);

void string_toupper(pstring str);
void string_tolower(pstring str);

struct stringtok
{
    pstring source;
    size_t sourcelen;
    size_t next;
    char *buffer;
    size_t buffercap;
    int done;
};

struct stringtok* stringtok_start(pstring source);
char* stringtok_next(struct stringtok *tok, const char *delim, size_t *numChars);
void stringtok_reset(struct stringtok *tok);
int stringtok_done(struct stringtok *tok);
void stringtok_end(struct stringtok *tok);
