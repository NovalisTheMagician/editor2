#include "debug.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#include "logging.h"

#undef malloc
#undef calloc
#undef free
#undef realloc

typedef enum AllocType
{
    AC_MALLOC,
    AC_STRING
} AllocType;

static const char* typeToString(AllocType type)
{
    switch(type)
    {
        case AC_MALLOC: return "MEMORY";
        case AC_STRING: return "STRING";
        default: return "UNKNOWN";
    }
}

struct Alloc
{
    struct Alloc *next;
    AllocType type;
    uintptr_t address;
    const char *origin;
    int line;
} *allocStart = NULL;

static pthread_mutex_t debugMutex = PTHREAD_MUTEX_INITIALIZER;

static FILE *debugLogFile;

static void insertAlloc(uintptr_t address, enum AllocType type, const char *origin, int line)
{
    pthread_mutex_lock(&debugMutex);
    if(allocStart == NULL)
    {
        allocStart = calloc(1, sizeof *allocStart);
        allocStart->address = address;
        allocStart->type = type;
        allocStart->origin = origin;
        allocStart->line = line;
    }
    else
    {
        struct Alloc *alloc = calloc(1, sizeof *alloc);
        alloc->address = address;
        alloc->type = type;
        alloc->origin = origin;
        alloc->line = line;
        alloc->next = allocStart;
        allocStart = alloc;
    }
    pthread_mutex_unlock(&debugMutex);
}

static void removeAlloc(uintptr_t address)
{
    struct Alloc *alloc = allocStart, *prev = NULL;
    pthread_mutex_lock(&debugMutex);
    while(alloc)
    {
        if(alloc->address == address)
        {
            struct Alloc *toFree = alloc;
            if(alloc->next == NULL && prev == NULL)
            {
                allocStart = NULL;
            }
            else if(alloc->next == NULL && prev)
            {
                prev->next = NULL;
            }
            else if(prev == NULL)
            {
                allocStart = alloc->next;
            }
            else
            {
                prev->next = alloc->next;
            }
            free(toFree);
            pthread_mutex_unlock(&debugMutex);
            return;
        }
        prev = alloc;
        alloc = alloc->next;
    }
    pthread_mutex_unlock(&debugMutex);
    assert(false && "address not found");
}

static void updateAlloc(uintptr_t oldAddress, uintptr_t newAddress, const char *origin, int line)
{
    struct Alloc *alloc = allocStart;
    pthread_mutex_lock(&debugMutex);
    while(alloc)
    {
        if(alloc->address == oldAddress)
        {
            alloc->address = newAddress;
            alloc->origin = origin;
            alloc->line = line;
            pthread_mutex_unlock(&debugMutex);
            break;
        }
        alloc = alloc->next;
    }
    pthread_mutex_unlock(&debugMutex);
}

void debug_init(const char *file)
{
    debugLogFile = fopen(file, "w");
    if(!debugLogFile)
        LogError("failed to create debug log file\n");
}

void debug_finish(void)
{
    struct Alloc *alloc = allocStart;
    pthread_mutex_lock(&debugMutex);
    while(alloc)
    {
        fprintf(debugLogFile, "Address: %p | Type: %s | From: %s:%d\n", (void*)alloc->address, typeToString(alloc->type), alloc->origin, alloc->line);
        struct Alloc *toFree = alloc;
        alloc = alloc->next;
        free(toFree);
    }
    pthread_mutex_unlock(&debugMutex);

    fclose(debugLogFile);
}

void debug_insertAddress(void *ptr, const char *file, int line)
{
    insertAlloc((uintptr_t)ptr, AC_MALLOC, file, line);
}

void* debug_malloc(size_t size, const char *file, int line)
{
    void *address = malloc(size);
    insertAlloc((uintptr_t)address, AC_MALLOC, file, line);
    return address;
}

void* debug_calloc(size_t num, size_t size, const char *file, int line)
{
    void *address = calloc(num, size);
    insertAlloc((uintptr_t)address, AC_MALLOC, file, line);
    return address;
}

void* debug_realloc(void *ptr, size_t size, const char *file, int line)
{
    uintptr_t oldAddress = (uintptr_t)ptr;
    void *address = realloc(ptr, size);
    updateAlloc(oldAddress, (uintptr_t)address, file, line);
    return address;
}

void debug_free(void *ptr, const char *, int)
{
    if(!ptr) return;
    removeAlloc((uintptr_t)ptr);
    free(ptr);
}
