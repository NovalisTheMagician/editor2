#pragma once

#include "cglm/struct.h" // IWYU pragma: keep

typedef struct EditorVertexType
{
    vec2s position;
    vec4s color;
    vec2s texCoord;
    uint32_t texId;
} EditorVertexType;

typedef struct RealtimeVertexType
{
    vec3s position;
    vec4s color;
    vec2s texCoord;
    uint32_t texId;
} RealtimeVertexType;

typedef uint32_t Index_t;
