#include "debug.h"

#include <stdio.h>

#include "logging.h"

#undef malloc
#undef calloc
#undef free
#undef realloc

#undef string_alloc
#undef string_cstr
#undef string_cstr_size
#undef string_cstr_alloc
#undef string_copy
#undef string_free

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
    void *address;
    const char *origin;
    int line;
} *allocStart = NULL;

static FILE *debugLogFile;

static int debug_malloc_count = 0;

static void insertAlloc(void *address, enum AllocType type, const char *origin, int line)
{
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
}

static void removeAlloc(void *address)
{
    struct Alloc *alloc = allocStart, *prev = NULL;
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
            break;
        }
        prev = alloc;
        alloc = alloc->next;
    }
    //assert(false, "address not found");
}

static void updateAlloc(void *oldAddress, void *newAddress, const char *origin, int line)
{
    struct Alloc *alloc = allocStart;
    while(alloc)
    {
        if(alloc->address == oldAddress)
        {
            alloc->address = newAddress;
            alloc->origin = origin;
            alloc->line = line;
            break;
        }
        alloc = alloc->next;
    }
}

void debug_init(const char *file)
{
    debugLogFile = fopen(file, "w");
    if(!debugLogFile)
        LogError("failed to create debug log file\n");
}

void debug_finish(int mallocOffset)
{
    //fprintf(debugLogFile, "\n\nBALANCE (offset=%d): %d\n", mallocOffset, debug_malloc_count + mallocOffset);

    struct Alloc *alloc = allocStart;
    while(alloc)
    {
        fprintf(debugLogFile, "Address: %p | Type: %s | From: %s:%d\n", alloc->address, typeToString(alloc->type), alloc->origin, alloc->line);
        struct Alloc *toFree = alloc;
        alloc = alloc->next;
        free(toFree);
    }

    fclose(debugLogFile);
}

void debug_adjust(int offset)
{
    debug_malloc_count += offset;
}

void* debug_malloc(size_t size, const char *file, int line)
{
    //fprintf(debugLogFile, "+m|%04d| %s: %d\n", ++debug_malloc_count, file, line);
    void *address = malloc(size);
    insertAlloc(address, AC_MALLOC, file, line);
    return address;
}

void* debug_calloc(size_t num, size_t size, const char *file, int line)
{
    //fprintf(debugLogFile, "+c|%04d| %s: %d\n", ++debug_malloc_count, file, line);
    void *address = calloc(num, size);
    insertAlloc(address, AC_MALLOC, file, line);
    return address;
}

void* debug_realloc(void *ptr, size_t size, const char *file, int line)
{
    void *address = realloc(ptr, size);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuse-after-free"
    updateAlloc(ptr, address, file, line);
#pragma GCC diagnostic pop
    return address;
}

void debug_free(void *ptr, const char *file, int line)
{
    if(!ptr) return;
    //fprintf(debugLogFile, "-f|%04d| %s: %d\n", --debug_malloc_count, file, line);
    removeAlloc(ptr);
    free(ptr);
}

pstring debug_pstr_alloc(size_t len, const char *file, int line)
{
    //fprintf(debugLogFile, "+s|%04d| %s: %d\n", ++debug_malloc_count, file, line);
    pstring string = string_alloc(len);
    insertAlloc(string, AC_STRING, file, line);
    return string;
}

pstring debug_pstr_cstr(const char *cstr, const char *file, int line)
{
    //fprintf(debugLogFile, "+s|%04d| %s: %d (%s)\n", ++debug_malloc_count, file, line, cstr);
    pstring string = string_cstr(cstr);
    insertAlloc(string, AC_STRING, file, line);
    return string;
}

pstring debug_pstr_cstr_alloc(const char *cstr, size_t size, const char *file, int line)
{
    //fprintf(debugLogFile, "+s|%04d| %s: %d (%s)\n", ++debug_malloc_count, file, line, cstr);
    pstring string = string_cstr_alloc(cstr, size);
    insertAlloc(string, AC_STRING, file, line);
    return string;
}

pstring debug_pstr_cstr_size(size_t size, const char *cstr, const char *file, int line)
{
    //fprintf(debugLogFile, "+s|%04d| %s: %d (%s)\n", ++debug_malloc_count, file, line, cstr);
    pstring string = string_cstr_size(size, cstr);
    insertAlloc(string, AC_STRING, file, line);
    return string;
}

pstring debug_pstr_copy(pstring string, const char *file, int line, const char *varname)
{
    //fprintf(debugLogFile, "+s|%04d| %s: %d {%s} (%s)\n", ++debug_malloc_count, file, line, varname, string);
    pstring copy = string_copy(string);
    insertAlloc(copy, AC_STRING, file, line);
    return copy;
}

void debug_pstr_free(pstring str, const char *file, int line, const char *varname)
{
    if(!str) return;
    //fprintf(debugLogFile, "-f|%04d| %s: %d {%s} (%s)\n", --debug_malloc_count, file, line, varname, str);
    removeAlloc(str);
    string_free(str);
}
