#include "pstring.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
#include <stdbool.h>

#define min(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_ < b_ ? a_ : b_; })

pstring string_alloc(size_t capacity)
{
    struct string_header *header = malloc(sizeof *header + capacity);
    pstring str = NULL;

    if(header)
    {
        header->length = 0;
        header->size = capacity;
        str = (char*)header + sizeof *header;

        memset(str, 0, header->size);
    }

    return str;
}

pstring string_cstr(const char str[static 1])
{
    size_t len = strlen(str);
    struct string_header *header = malloc(sizeof *header + len + 1);
    pstring newStr = NULL;

    if(header)
    {
        header->length = len;
        header->size = len + 1;
        newStr = (char*)header + sizeof *header;

        for(size_t i = 0; i < len; ++i)
            newStr[i] = str[i];

        newStr[len] = 0;
    }

    return newStr;
}

pstring string_cstr_alloc(const char str[static 1], size_t size)
{
    size_t stringlen = strlen(str);
    size_t len = stringlen > size ? stringlen : size;
    struct string_header *header = malloc(sizeof *header + len + 1);
    pstring newStr = NULL;

    if(header)
    {
        header->length = len;
        header->size = len + 1;
        newStr = (char*)header + sizeof *header;

        for(size_t i = 0; i < stringlen; ++i)
            newStr[i] = str[i];

        newStr[stringlen] = 0;
    }

    return newStr;
}

pstring string_cstr_size(size_t size, const char str[static size])
{
    struct string_header *header = malloc(sizeof *header + size+1);
    pstring newStr = NULL;

    if(header)
    {
        header->length = size;
        header->size = size+1;
        newStr = (char*)header + sizeof *header;

        for(size_t i = 0; i < size; ++i)
            newStr[i] = str[i];

        newStr[size] = 0;
    }

    return newStr;
}

void string_free(pstring str)
{
    assert(str);
    void *header = str - sizeof(struct string_header);
    free(header);
}

size_t string_format(pstring into, const char format[static 1], ...)
{
    va_list args;
    va_start(args, format);

    size_t num = string_vformat(into, format, args);
    
    va_end(args);

    return num;
}

size_t string_vformat(pstring into, const char format[static 1], va_list args)
{
    struct string_header *header = (struct string_header*)(into - sizeof *header);
    int ret = vsnprintf(into, header->size, format, args);
    if(ret > 0)
    {
        header->length = ret < header->size ? ret : header->size;
    }
    return ret;
}

size_t string_format_offset(pstring into, size_t offset, const char format[static 1], ...)
{
    va_list args;
    va_start(args, format);

    size_t num = string_vformat_offset(into, offset, format, args);
    
    va_end(args);

    return num;
}

size_t string_vformat_offset(pstring into, size_t offset, const char format[static 1], va_list args)
{
    struct string_header *header = (struct string_header*)(into - sizeof *header);
    int ret = vsnprintf(into + offset, header->size - offset, format, args);
    if(ret > 0)
    {
        header->length = ret < header->size ? ret : header->size;
    }
    return ret;
}

size_t string_size(pstring str)
{
    struct string_header *header = (struct string_header*)(str - sizeof *header);
    return header->size;
}

size_t string_length(pstring str)
{
    struct string_header *header = (struct string_header*)(str - sizeof *header);
    return header->length;
}

void string_recalc(pstring str)
{
    struct string_header *header = (struct string_header*)(str - sizeof *header);
    header->length = strnlen(str, header->size);
}

pstring string_copy(pstring str)
{
    struct string_header *header = (struct string_header*)(str - sizeof *header);
    if(header->length > 0)
    {
        return string_cstr_alloc(str, header->length + 1);
    }
    return string_alloc(1);
}

size_t string_copy_into_cstr(pstring into, const char *str)
{
    struct string_header *header = (struct string_header*)(into - sizeof *header);
    size_t intoSize = header->size;
    size_t strSize = strlen(str);
    memcpy(into, str, min(intoSize, strSize));
    string_recalc(into);
    return min(intoSize, strSize);
}

size_t string_copy_into(pstring into, pstring str)
{
    struct string_header *header = (struct string_header*)(into - sizeof *header);
    size_t intoSize = header->size;
    header = (struct string_header*)(str - sizeof *header);
    size_t strSize = header->size;
    memcpy(into, str, min(intoSize, strSize));
    string_recalc(into);
    return min(intoSize, strSize);
}

pstring string_substring(pstring str, size_t start, ssize_t end)
{
    struct string_header *header = (struct string_header*)(str - sizeof *header);
    assert(start <= header->length);
    size_t e = end < 0 ? header->length : (size_t)end;
    assert(start <= e);
    return string_cstr_size(e - start, str + start);
}

ssize_t string_first_index_of(pstring str, size_t offset, const char tok[static 1])
{
    struct string_header *header = (struct string_header*)(str - sizeof *header);
    size_t tokSize = strlen(tok);
    for(size_t i = offset; i <= header->length - tokSize; ++i)
    {
        bool hit = true;
        for(size_t j = 0; j < tokSize; ++j)
            hit &= str[i + j] == tok[j];
        if(hit) return i;
    }
    return -1;
}

ssize_t string_last_index_of(pstring str, size_t offset, const char tok[static 1])
{
    struct string_header *header = (struct string_header*)(str - sizeof *header);
    size_t tokSize = strlen(tok);
    ssize_t hitIdx = -1;
    for(size_t i = offset; i <= header->length - tokSize; ++i)
    {
        bool hit = true;
        for(size_t j = 0; j < tokSize; ++j)
            hit &= str[i + j] == tok[j];
        if(hit) hitIdx = i;
    }
    return hitIdx;
}

pstring* string_split(pstring str, const char tok[static 1])
{
    return NULL;
}

int string_cmp(pstring a, pstring b)
{
    struct string_header *aheader = (struct string_header*)(a - sizeof *aheader), *bheader = (struct string_header*)(b - sizeof *bheader);
    size_t len = aheader->length < bheader->length ? aheader->length : bheader->length;
    for(size_t i = 0; i < len; ++i)
    {
        int ca = a[i];
        int cb = b[i];
        if (ca - cb != 0) return ca - cb;
    }
    return aheader->length - bheader->length;
}

int string_icmp(pstring a, pstring b)
{
    struct string_header *aheader = (struct string_header*)(a - sizeof *aheader), *bheader = (struct string_header*)(b - sizeof *bheader);
    size_t len = aheader->length < bheader->length ? aheader->length : bheader->length;
    for(size_t i = 0; i < len; ++i)
    {
        int ca = tolower(a[i]);
        int cb = tolower(b[i]);
        if (ca - cb != 0) return ca - cb;
    }
    return aheader->length - bheader->length;
}

void string_toupper(pstring str)
{

}

void string_tolower(pstring str)
{

}

#define INITAL_TOK_BUFFER_CAP 256

struct stringtok* stringtok_start(pstring source)
{
    struct stringtok *tok = malloc(sizeof *tok);
    if(tok)
    {
        tok->source = source;
        tok->sourcelen = string_length(source);
        tok->next = 0;
        tok->buffer = calloc(INITAL_TOK_BUFFER_CAP, sizeof(char));
        tok->buffercap = INITAL_TOK_BUFFER_CAP;
        tok->done = 0;
    }
    return tok;
}

char* stringtok_next(struct stringtok tok[static 1], const char delim[static 1], size_t *numChars)
{
    if(tok->done) return NULL;

    ssize_t idx = string_first_index_of(tok->source, tok->next, delim);
    if(idx == -1) // no token found return the rest of the source
    {
        tok->done = 1;
        size_t len = tok->sourcelen - tok->next;
        if(numChars) *numChars = len;
        if(len >= tok->buffercap)
        {
            tok->buffercap *= 2;
            tok->buffer = realloc(tok->buffer, tok->buffercap);
        }
        for(size_t i = tok->next, j = 0; i < tok->sourcelen; ++i, ++j)
            tok->buffer[j] = tok->source[i];
        tok->buffer[tok->sourcelen - tok->next] = 0;
    }
    else
    {
        size_t len = idx - tok->next;
        if(numChars) *numChars = len;
        if(len >= tok->buffercap)
        {
            tok->buffercap *= 2;
            tok->buffer = realloc(tok->buffer, tok->buffercap);
        }
        for(size_t i = tok->next, j = 0; i < len; ++i, ++j)
            tok->buffer[j] = tok->source[i];
        tok->buffer[len] = 0;
        tok->next = idx + 1;
    }
    return tok->buffer;
}

void stringtok_reset(struct stringtok tok[static 1])
{
    tok->next = 0;
    tok->done = 0;
}

int stringtok_done(struct stringtok tok[static 1])
{
    return tok->done;
}

void stringtok_end(struct stringtok *tok)
{
    free(tok->buffer);
    free(tok);
}
