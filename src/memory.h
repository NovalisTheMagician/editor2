#pragma once

#include <malloc.h>

typedef struct
{
    void *start;
    void *current;
    void *end;
    void *previous;
} arena_t;

arena_t arena_create(size_t totalSize);
void arena_destroy(arena_t *arena);
void* arena_alloc(arena_t *arena, size_t size);
void* arena_calloc(arena_t *arena, size_t num, size_t size);
void arena_reset_last_alloc(arena_t *arena);
