#pragma once

#include <stdint.h>

#include "vecmath.h"

typedef struct EditorVertexType
{
    Vec2 position;
    Color color;
    Vec2 texCoord;
} EditorVertexType;

typedef struct RealtimeVertexType
{
    Vec3 position;
    Color color;
    Vec2 texCoord;
} RealtimeVertexType;

typedef uint32_t Index_t;
