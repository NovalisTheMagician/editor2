#pragma once

#include <stdlib.h>

typedef struct pstring
{
    char *data;
    size_t size;
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

pstring pstr_copy(pstring string);
pstring pstr_substring(pstring string, size_t start, ssize_t end);
void pstr_upper(pstring string);
void pstr_lower(pstring string);
ssize_t pstr_first_index_of_str(pstring string, pstring tok);
ssize_t pstr_last_index_of_str(pstring string, pstring tok);
pstring pstr_tok_str(pstring *string, pstring tok);

ssize_t pstr_first_index_of_cstr(pstring string, const char *tok);
ssize_t pstr_last_index_of_cstr(pstring string, const char *tok);
pstring pstr_tok_cstr(pstring *string, const char *tok);

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

pstring_buf pstr_buf_alloc(size_t initialCapacity);
void pstr_buf_free(pstring_buf string_buf);
void pstr_buf_append_str(pstring_buf string_buf, pstring string);
pstring pstr_buf_finalize(pstring_buf string_buf);
