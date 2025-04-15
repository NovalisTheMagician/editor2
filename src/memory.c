#include "memory.h"

#include <stdalign.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#ifndef align_up
#define align_up(num, align) (((num) + ((align) - 1)) & ~((align) - 1))
#endif

arena_t arena_create(size_t totalSize)
{
    arena_t arena = { 0 };
    arena.start = malloc(totalSize);
    assert(arena.start);
    arena.current = arena.start;
    arena.end = arena.start + totalSize;
    arena.previous = arena.current;
    return arena;
}

void arena_destroy(arena_t *arena)
{
    free(arena->start);
    arena->start = arena->current = arena->end = arena->previous = NULL;
}

void* arena_alloc(arena_t *arena, size_t size)
{
    const size_t alignment = alignof(max_align_t);
    arena->previous = arena->current;
    void *current = (void*)align_up((uintptr_t)arena->current, alignment);
    assert((current + size) < arena->end);
    arena->current = current + size;
    return arena->current;
}

void* arena_calloc(arena_t *arena, size_t num, size_t size)
{
    void *data = arena_alloc(arena, num * size);
    memset(data, 0, size);
    return data;
}

void arena_reset_last_alloc(arena_t *arena)
{
    arena->current = arena->previous;
}
