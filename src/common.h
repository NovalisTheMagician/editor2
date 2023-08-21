#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdalign.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glad2/gl.h>
#include <cglm/cglm.h>

#include "pstring.h"
#include "hash.h"

#define COUNT_OF(arr) (sizeof(arr)/sizeof(0[arr]))

#define min(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_ < b_ ? a_ : b_; })
#define max(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_ > b_ ? a_ : b_; })

#if defined(_DEBUG)

void debug_init(const char *file);
void debug_finish(void);
void* debug_malloc(size_t size, const char *file, int line);
void* debug_calloc(size_t num, size_t size, const char *file, int line);
void debug_free(void *ptr, const char *file, int line);

#define malloc(size) debug_malloc(size, __FILE__, __LINE__)
#define free(ptr) debug_free(ptr, __FILE__, __LINE__)
#define calloc(num, size) debug_calloc(num, size, __FILE__, __LINE__)
#endif
