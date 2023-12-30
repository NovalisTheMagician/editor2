#pragma once

#include <stdbool.h>

struct weakptr_t
{
    bool valid;
    void *ptr;
};
