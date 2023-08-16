#pragma once

#include <stdlib.h>

typedef struct pstring
{
    char *data;
    size_t size, capacity;
} pstring;

typedef struct pstring_buf
{
    char *data;
    size_t size, capacity;
} pstring_buf;

pstring pstr_alloc(size_t len);
pstring pstr_cstr(const char *cstr);
pstring pstr_cstr_size(const char *cstr, size_t size);
char* pstr_tocstr(pstring string);
void pstr_free(pstring string);
pstring pstr_replace(pstring old, pstring new);

pstring pstr_copy(pstring string);
void pstr_copy_into_str(pstring *into, pstring string);
void pstr_copy_into_cstr(pstring *into, const char *string);

size_t pstr_format(pstring *into, const char *format, ...);

pstring pstr_substring(pstring string, size_t start, ssize_t end);
void pstr_upper(pstring string);
void pstr_lower(pstring string);
ssize_t pstr_first_index_of_str(pstring string, pstring tok);
ssize_t pstr_last_index_of_str(pstring string, pstring tok);
pstring pstr_tok_str(pstring *string, pstring tok);

ssize_t pstr_first_index_of_cstr(pstring string, const char *tok);
ssize_t pstr_last_index_of_cstr(pstring string, const char *tok);
pstring pstr_tok_cstr(pstring *string, const char *tok);

int pstr_cmp_str(pstring a, pstring b);
int pstr_cmp_cstr(pstring a, const char *b);

int pstr_icmp_str(pstring a, pstring b);
int pstr_icmp_cstr(pstring a, const char *b);

#define pstr_first_index_of(string, tok) _Generic((tok), \
                                                char*: pstr_first_index_of_cstr, \
                                                default: pstr_first_index_of_str \
                                                )(string, tok)

#define pstr_last_index_of(string, tok) _Generic((tok), \
                                                char*: pstr_last_index_of_cstr, \
                                                default: pstr_last_index_of_str \
                                                )(string, tok)

#define pstr_tok(string, tok) _Generic((tok), \
                                        char*: pstr_tok_cstr, \
                                        default: pstr_tok_str \
                                        )(string, tok)

#define pstr_cmp(a, b) _Generic((b), \
                                char*: pstr_cmp_cstr, \
                                default: pstr_cmp_str \
                                )(a, b)

#define pstr_icmp(a, b) _Generic((b), \
                                char*: pstr_icmp_cstr, \
                                default: pstr_icmp_str \
                                )(a, b)

#define pstr_copy_into(into, string) _Generic((string), \
                                    char*: pstr_copy_into_cstr, \
                                    default: pstr_copy_into_str \
                                    )(into, string)

pstring_buf pstr_buf_alloc(size_t initialCapacity);
void pstr_buf_free(pstring_buf string_buf);
void pstr_buf_append_str(pstring_buf string_buf, pstring string);
pstring pstr_buf_finalize(pstring_buf string_buf);
