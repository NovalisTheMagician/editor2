#pragma once

#include <tgmath.h>
#include "utils.h"

typedef double real_t;

typedef struct
{
    real_t x, y;
} Vec2;

typedef struct
{
    real_t x, y, z;
} Vec3;

typedef struct
{
    real_t x, y, z, w;
} Vec4;

typedef struct
{ 
    float r, g, b, a;
} Color;

static inline Vec2 vec2_add(Vec2 a, Vec2 b)
{
    return (Vec2){ a.x + b.x, a.y + b.y };
}

static inline Vec2 vec2_sub(Vec2 a, Vec2 b)
{
    return (Vec2){ a.x - b.x, a.y - b.y };
}

static inline Vec2 vec2_scale(Vec2 a, real_t b)
{
    return (Vec2){ a.x * b, a.y * b };
}

static inline Vec2 vec2_mul(Vec2 a, Vec2 b)
{
    return (Vec2){ a.x * b.x, a.y * b.y };
}

static inline real_t vec2_dot(Vec2 a, Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

static inline real_t vec2_cross(Vec2 a, Vec2 b)
{
    return a.x * b.y - b.x * a.y;
}

static inline real_t vec2_distance2(Vec2 a, Vec2 b)
{
    real_t dx = b.x - a.x;
    real_t dy = b.y - a.y;
    return dx*dx + dy*dy;
}

static inline real_t vec2_distance(Vec2 a, Vec2 b)
{
    return sqrt(vec2_distance2(a, b));
}

static inline real_t vec2_len2(Vec2 a)
{
    return a.x*a.x + a.y*a.y;
}

static inline real_t vec2_len(Vec2 a)
{
    return sqrt(vec2_len2(a));
}

static inline Vec2 vec2_normalize(Vec2 a)
{
    real_t len = vec2_len(a);
    return (Vec2){ a.x / len, a.y / len };
}

static inline Vec2 vec2_maxv(Vec2 a, Vec2 b)
{
    return (Vec2){ max(a.x, b.x), max(a.y, b.y) };
}

static inline Vec2 vec2_minv(Vec2 a, Vec2 b)
{
    return (Vec2){ min(a.x, b.x), min(a.y, b.y) };
}

static inline bool vec2_eqv(Vec2 a, Vec2 b)
{
    return eq(a.x, b.x) && eq(a.y, b.y);
}

//////////////////////////////////////////////////////////////////

static inline Vec3 vec3_add(Vec3 a, Vec3 b)
{
    return (Vec3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline Vec3 vec3_sub(Vec3 a, Vec3 b)
{
    return (Vec3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

static inline Vec3 vec3_scale(Vec3 a, real_t b)
{
    return (Vec3){ a.x*b, a.y*b, a.z*b };
}

static inline Vec3 vec3_mul(Vec3 a, Vec3 b)
{
    return (Vec3){ a.x*b.x, a.y*b.y, a.z*b.z };
}

static inline real_t vec3_dot(Vec3 a, Vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline Vec3 vec3_cross(Vec3 a, Vec3 b)
{
    return (Vec3){ a.y*b.z - b.z*a.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

static inline real_t vec3_distance2(Vec3 a, Vec3 b)
{
    real_t dx = b.x - a.x;
    real_t dy = b.y - a.x;
    real_t dz = b.z - a.z;
    return dx*dx + dy*dy + dz*dz;
}

static inline real_t vec3_distance(Vec3 a, Vec3 b)
{
    return sqrt(vec3_distance2(a, b));
}

static inline real_t vec3_len2(Vec3 a)
{
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

static inline real_t vec3_len(Vec3 a)
{
    return sqrt(vec3_len2(a));
}

static inline Vec3 vec3_normalize(Vec3 a)
{
    real_t len = vec3_len(a);
    return (Vec3){ a.x / len, a.y / len, a.z / len };
}

static inline Vec3 vec3_maxv(Vec3 a, Vec3 b)
{
    return (Vec3){ max(a.x, b.x), max(a.y, b.y), max(a.z, b.z) };
}

static inline Vec3 vec3_minv(Vec3 a, Vec3 b)
{
    return (Vec3){ min(a.x, b.x), min(a.y, b.y), min(a.z, b.z) };
}

static inline bool vec3_eqv(Vec3 a, Vec3 b)
{
    return eq(a.x, b.x) && eq(a.y, b.y) && eq(a.z, b.z);
}
