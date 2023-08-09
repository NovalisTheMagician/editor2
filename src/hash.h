#pragma once

#include <stdint.h>
#include "pstring.h"

uint64_t murmurhash3_cstr(const char* str);
uint64_t murmurhash3_pstr(pstring str);

uint64_t djb2hash_cstr(const char *str);
uint64_t djb2hash_pstr(pstring str);

#define murmurhash3(str) _Generic((str), \
                                    char*: murmurhash3_cstr, \
                                    default: murmurhash3_pstr \
                                 )(str)

#define djb2hash(str) _Generic((str), \
                                char*: djb2hash_cstr, \
                                default: djb2hash_pstr \
                                )(str)

#define hash(str) murmurhash3(str)
