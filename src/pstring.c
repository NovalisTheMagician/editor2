#include "pstring.h"

#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

pstring pstr_alloc(size_t len)
{
    return (pstring){ .data = calloc(len+1, 1), .size = len };
}

pstring pstr_cstr(const char *cstr)
{
    if(!cstr) return pstr_alloc(0);

    size_t len = strlen(cstr);
    pstring string = { .data = calloc(len+1, 1), .size = len };
    memcpy(string.data, cstr, len);
    return string;
}

pstring pstr_cstr_size(const char *cstr, size_t size)
{
    size_t len = strlen(cstr);
    size_t s = len > size ? len : size;
    pstring string = pstr_alloc(s);
    if(cstr)
        memcpy(string.data, cstr, len);
    return string;
}

char* pstr_tocstr(pstring string)
{
    return string.data;
}

void pstr_free(pstring string)
{
    free(string.data);
}

pstring pstr_copy(pstring string)
{
    pstring copy = { .data = calloc(string.size+1, 1), .size = string.size };
    memcpy(copy.data, string.data, copy.size);
    return copy;
}

void pstr_copy_into_str(pstring *into, pstring string)
{
    size_t len = into->size < string.size ? into->size : string.size;
    memcpy(into->data, string.data, len);
}

void pstr_copy_into_cstr(pstring *into, const char *string)
{
    size_t slen = strlen(string);
    size_t len = into->size < slen ? into->size : slen;
    memcpy(into->data, string, len);
}

pstring pstr_substring(pstring string, size_t start, ssize_t end)
{
    assert(start < string.size);
    size_t e = end < 0 ? string.size : (size_t)end;
    assert(start < e);
    return (pstring){ .data = string.data + start, .size = e - start };
}

void pstr_upper(pstring string)
{
    for(size_t i = 0; i < string.size; ++i)
        string.data[i] = toupper(string.data[i]);
}

void pstr_lower(pstring string)
{
    for(size_t i = 0; i < string.size; ++i)
        string.data[i] = tolower(string.data[i]);
}

ssize_t pstr_first_index_of_str(pstring string, pstring tok)
{
    for(size_t i = 0; i < string.size - tok.size; ++i)
    {
        bool hit = true;
        for(size_t j = 0; j < tok.size; ++j)
            hit &= string.data[i + j] == tok.data[j];
        if(hit) return i;
    }
    return -1;
}

ssize_t pstr_last_index_of_str(pstring string, pstring tok)
{
    size_t hitIdx = -1;
    for(size_t i = 0; i < string.size - tok.size; ++i)
    {
        bool hit = true;
        for(size_t j = 0; j < tok.size; ++j)
            hit &= string.data[i + j] == tok.data[j];
        if(hit) hitIdx = i;
    }
    return hitIdx;
}

pstring pstr_tok_str(pstring *string, pstring tok)
{
    ssize_t idx = pstr_first_index_of_str(*string, tok);
    if(idx == -1) // no token found return the whole string
    {
        pstring copy = *string;
        string->data += string->size;
        string->size = 0;
        return copy;
    }

    string->data[idx] = '\0';

    pstring sub = pstr_substring(*string, 0, idx);
    string->data += idx+1;
    string->size -= idx+1;
    return sub;
}

ssize_t pstr_first_index_of_cstr(pstring string, const char *tok)
{
    size_t tokSize = strlen(tok);
    for(size_t i = 0; i <= string.size - tokSize; ++i)
    {
        bool hit = true;
        for(size_t j = 0; j < tokSize; ++j)
            hit &= string.data[i + j] == tok[j];
        if(hit) return i;
    }
    return -1;
}

ssize_t pstr_last_index_of_cstr(pstring string, const char *tok)
{
    size_t tokSize = strlen(tok);
    size_t hitIdx = -1;
    for(size_t i = 0; i < string.size - tokSize; ++i)
    {
        bool hit = true;
        for(size_t j = 0; j < tokSize; ++j)
            hit &= string.data[i + j] == tok[j];
        if(hit) hitIdx = i;
    }
    return hitIdx;
}

pstring pstr_tok_cstr(pstring *string, const char *tok)
{
    ssize_t idx = pstr_first_index_of_cstr(*string, tok);
    if(idx == -1) // no token found return the whole string
    {
        pstring copy = *string;
        string->data += string->size;
        string->size = 0;
        return copy;
    }

    string->data[idx] = '\0';

    pstring sub = pstr_substring(*string, 0, idx);
    string->data += idx+1;
    string->size -= idx+1;
    return sub;
}

int pstr_cmp_str(pstring a, pstring b)
{
    size_t len = a.size < b.size ? a.size : b.size;
    for(size_t i = 0; i < len; ++i)
    {
        int ca = a.data[i];
        int cb = b.data[i];
        if (ca - cb != 0) return ca - cb;
    }
    return a.size - b.size;
}

int pstr_cmp_cstr(pstring a, const char *b)
{
    size_t blen = strlen(b);
    size_t len = a.size < blen ? a.size : blen;
    for(size_t i = 0; i < len; ++i)
    {
        int ca = a.data[i];
        int cb = b[i];
        if (ca - cb != 0) return ca - cb;
    }
    return a.size - blen;
}

int pstr_icmp_str(pstring a, pstring b)
{
    size_t len = a.size < b.size ? a.size : b.size;
    for(size_t i = 0; i < len; ++i)
    {
        int ca = tolower(a.data[i]);
        int cb = tolower(b.data[i]);
        if (ca - cb != 0) return ca - cb;
    }
    return a.size - b.size;
}

int pstr_icmp_cstr(pstring a, const char *b)
{
    size_t blen = strlen(b);
    size_t len = a.size < blen ? a.size : blen;
    for(size_t i = 0; i < len; ++i)
    {
        int ca = tolower(a.data[i]);
        int cb = tolower(b[i]);
        if (ca - cb != 0) return ca - cb;
    }
    return a.size - blen;
}

#if 0
pstring_buf pstr_buf_alloc(size_t initialCapacity)
{

}

void pstr_buf_free(pstring_buf string_buf)
{

}

void pstr_buf_append_str(pstring_buf string_buf, pstring string)
{

}

pstring pstr_buf_finalize(pstring_buf string_buf)
{

}
#endif
