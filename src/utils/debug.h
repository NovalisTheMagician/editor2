#pragma once

#include <stdlib.h>
#include <malloc.h>
#include "pstring.h"

void debug_init(const char *file);
void debug_finish(int mallocOffset);
void debug_adjust(int offset);
void* debug_malloc(size_t size, const char *file, int line);
void* debug_calloc(size_t num, size_t size, const char *file, int line);
void* debug_realloc(void *ptr, size_t size, const char *file, int line);
void debug_free(void *ptr, const char *file, int line);

pstring debug_pstr_alloc(size_t len, const char *file, int line);
pstring debug_pstr_cstr(const char *cstr, const char *file, int line);
pstring debug_pstr_cstr_alloc(const char *cstr, size_t size, const char *file, int line);
pstring debug_pstr_cstr_size(size_t size, const char *cstr, const char *file, int line);
pstring debug_pstr_copy(pstring string, const char *file, int line, const char *varname);
void debug_pstr_free(pstring str, const char *file, int line, const char *varname);

#ifndef NO_MEMORY_DEBUG
#define malloc(size) debug_malloc(size, __FILE__, __LINE__)
#define free(ptr) debug_free(ptr, __FILE__, __LINE__)
#define calloc(num, size) debug_calloc(num, size, __FILE__, __LINE__)
#define realloc(ptr, size) debug_realloc(ptr, size, __FILE__, __LINE__)
#endif

#ifndef NO_STRING_DEBUG
#define string_alloc(len) debug_pstr_alloc(len, __FILE__, __LINE__)
#define string_cstr(cstr) debug_pstr_cstr(cstr, __FILE__, __LINE__)
#define string_cstr_alloc(cstr, size) debug_pstr_cstr_alloc(cstr, size, __FILE__, __LINE__)
#define string_cstr_size(size, cstr) debug_pstr_cstr_size(size, cstr, __FILE__, __LINE__)
#define string_copy(str) debug_pstr_copy(str, __FILE__, __LINE__, #str)
#define string_free(str) debug_pstr_free(str, __FILE__, __LINE__, #str)
#endif
