#pragma once

#if defined(_DEBUG)
#include <stdlib.h>

void debug_init(const char *file);
void debug_finish(void);
void* debug_malloc(size_t size, const char *file, int line);
void* debug_calloc(size_t num, size_t size, const char *file, int line);
void debug_free(void *ptr, const char *file, int line);

#define malloc(size) debug_malloc(size, __FILE__, __LINE__)
#define free(ptr) debug_free(ptr, __FILE__, __LINE__)
#define calloc(num, size) debug_calloc(num, size, __FILE__, __LINE__)
#endif
