#include "hash.h"

uint64_t murmurhash3_cstr(const char* str)
{
    uint64_t h = 0x12345678;
    for (; *str; ++str) 
    {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

uint64_t murmurhash3_pstr(pstring str)
{
    uint64_t h = 0x12345678;
    for (size_t i = 0; i < str.size; ++i) 
    {
        h ^= str.data[i];
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

uint64_t djb2hash_cstr(const char *str)
{
    uint64_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

uint64_t djb2hash_pstr(pstring str)
{
    uint64_t hash = 5381;
    for(size_t i = 0; i < str.size; ++i)
        hash = ((hash << 5) + hash) + str.data[i]; /* hash * 33 + c */

    return hash;
}
