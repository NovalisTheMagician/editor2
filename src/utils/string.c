#include "string.h"

#include <string.h>

#include "../memory.h" // IWYU pragma: keep

char* CopyString(const char *string)
{
    char *copy = malloc(strlen(string) + 1);
    strcpy(copy, string);
    return copy;
}

char* CopyStringLen(const char *string, size_t len)
{
    char *copy = malloc(len + 1);
    strncpy(copy, string, len+1);
    return copy;
}
