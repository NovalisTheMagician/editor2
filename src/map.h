#pragma once

#include "common.h"

struct Line
{
    uint32_t a, b;
    uint32_t type;
};

struct Sector
{
    uint32_t *lines;
    uint32_t numLines;
};

struct Map
{
    char *file;
};
