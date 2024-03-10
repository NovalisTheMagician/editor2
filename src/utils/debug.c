#include "debug.h"

#include <stdio.h>

#include "logging.h"

#undef malloc
#undef calloc
#undef free

#undef string_alloc
#undef string_cstr
#undef string_cstr_size
#undef string_cstr_alloc
#undef string_copy
#undef string_free

static FILE *debugLogFile;

static int debug_malloc_count = 0;

void debug_init(const char *file)
{
    debugLogFile = fopen(file, "w");
    if(!debugLogFile)
        LogError("failed to create debug log file\n");
}

void debug_finish(int mallocOffset)
{
    fprintf(debugLogFile, "\n\nBALANCE (offset=%d): %d\n", mallocOffset, debug_malloc_count + mallocOffset);
    fclose(debugLogFile);
}

void debug_adjust(int offset)
{
    debug_malloc_count += offset;
}

void* debug_malloc(size_t size, const char *file, int line)
{
    fprintf(debugLogFile, "+ m|%04d| %s: %d\n", ++debug_malloc_count, file, line);
    return malloc(size);
}

void* debug_calloc(size_t num, size_t size, const char *file, int line)
{
    fprintf(debugLogFile, "+ c|%04d| %s: %d\n", ++debug_malloc_count, file, line);
    return calloc(num, size);
}

void debug_free(void *ptr, const char *file, int line)
{
    if(!ptr) return;
    fprintf(debugLogFile, "- f|%04d| %s: %d\n", --debug_malloc_count, file, line);
    free(ptr);
}

pstring debug_pstr_alloc(size_t len, const char *file, int line)
{
    fprintf(debugLogFile, "+sa|%04d| %s: %d\n", ++debug_malloc_count, file, line);
    return string_alloc(len);
}

pstring debug_pstr_cstr(const char *cstr, const char *file, int line)
{
    fprintf(debugLogFile, "+sz|%04d| %s: %d (%s)\n", ++debug_malloc_count, file, line, cstr);
    return string_cstr(cstr);
}

pstring debug_pstr_cstr_alloc(const char *cstr, size_t size, const char *file, int line)
{
    fprintf(debugLogFile, "+sz|%04d| %s: %d (%s)\n", ++debug_malloc_count, file, line, cstr);
    return string_cstr_alloc(cstr, size);
}

pstring debug_pstr_cstr_size(size_t size, const char *cstr, const char *file, int line)
{
    fprintf(debugLogFile, "+sz|%04d| %s: %d (%s)\n", ++debug_malloc_count, file, line, cstr);
    return string_cstr_size(size, cstr);
}

pstring debug_pstr_copy(pstring string, const char *file, int line, const char *varname)
{
    fprintf(debugLogFile, "+sc|%04d| %s: %d {%s} (%s)\n", ++debug_malloc_count, file, line, varname, string);
    return string_copy(string);
}

void debug_pstr_free(pstring str, const char *file, int line, const char *varname)
{
    if(!str) return;
    fprintf(debugLogFile, "-sf|%04d| %s: %d {%s} (%s)\n", --debug_malloc_count, file, line, varname, str);
    string_free(str);
}
