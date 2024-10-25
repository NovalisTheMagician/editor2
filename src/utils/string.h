#pragma once

#include <stddef.h>

char* CopyString(const char *string);
char* CopyStringLen(const char *string, size_t len);
char* NormalizePath(char *path);
