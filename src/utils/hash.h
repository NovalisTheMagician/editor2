#pragma once

#include <stdint.h>
#include "pstring.h"

uint64_t murmurhash3(const char* str);

uint64_t djb2hash(const char *str);

#define hash(str) murmurhash3(str)
