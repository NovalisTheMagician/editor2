#include "common.h"

#undef malloc
#undef calloc
#undef free

static FILE *debugLogFile;

static int debug_malloc_count = 0;

void debug_init(const char *file)
{
    debugLogFile = fopen(file, "w");
    if(!debugLogFile)
        printf("failed to create debug log file\n");
}

void debug_finish(void)
{
    fprintf(debugLogFile, "\n\nBALANCE: %d\n", debug_malloc_count);
    fclose(debugLogFile);
}

void* debug_malloc(size_t size, const char *file, int line)
{
    fprintf(debugLogFile, "+m| %s: %d\n", file, line);
    debug_malloc_count++;
    return malloc(size);
}

void* debug_calloc(size_t num, size_t size, const char *file, int line)
{
    fprintf(debugLogFile, "+c| %s: %d\n", file, line);
    debug_malloc_count++;
    return calloc(num, size);
}

void debug_free(void *ptr, const char *file, int line)
{
    fprintf(debugLogFile, "-f| %s: %d\n", file, line);
    free(ptr);
    debug_malloc_count--;
}
