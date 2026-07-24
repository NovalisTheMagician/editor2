#pragma once

#include <tgmath.h>
#include <stdint.h>
#include <float.h>
#include "utils.h"

typedef float real_t;
#define REAL_MIN -FLT_MAX
#define REAL_MAX FLT_MAX

typedef struct Vec2
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
    return a.x * b.y - a.y * b.x;
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

///////////////////////////////////////////////////////////////////////

typedef int32_t fixed_t;
typedef int64_t fixedw_t;
#define FIXED_SHIFT 16
#define FIXED_ONE ((fixed_t)1 << FIXED_SHIFT)
#define FIXED_MAX INT32_MAX
#define FIXED_MIN INT32_MIN
#define FIXEDW_MAX INT64_MAX
#define FIXEDW_MIN INT64_MIN

static inline fixed_t fixed_from_real(real_t f) { return (fixed_t)(f * (real_t)FIXED_ONE); }
static inline fixed_t fixed_from_int(int i) { return (fixed_t)i << FIXED_SHIFT; }
static inline real_t fixed_to_real(fixed_t f) { return (real_t)f / (real_t)FIXED_ONE; }
static inline int fixed_to_int(fixed_t f) { return (int)(f >> FIXED_SHIFT); }

static inline fixed_t fixed_mul(fixed_t a, fixed_t b)
{
    return (fixed_t)(((fixedw_t)a * (fixedw_t)b) >> FIXED_SHIFT);
}

static inline fixed_t fixed_div(fixed_t a, fixed_t b)
{
    fixedw_t wide_a = (fixedw_t)a << FIXED_SHIFT;
    return (fixed_t)(wide_a / (fixedw_t)b);
}

static inline fixed_t round_div(fixed_t a, fixed_t b)
{
    fixedw_t wide_a = (fixedw_t)a << FIXED_SHIFT;
    fixedw_t half_b = (fixedw_t)b / 2;

    if((wide_a < 0) != (b < 0))
        return (fixed_t)((wide_a - half_b) / (fixedw_t)b);
    else
        return (fixed_t)((wide_a + half_b) / (fixedw_t)b);
}

typedef struct
{
    fixed_t x, y;
} FVec2;

static inline FVec2 fvec2_make(real_t x, real_t y)
{
    return (FVec2){ fixed_from_real(x), fixed_from_real(y) };
}

static inline FVec2 fvec2_from_vec2(Vec2 v)
{
    return fvec2_make(v.x, v.y);
}

static inline Vec2 vec2_from_fvec2(FVec2 v)
{
    return (Vec2){ fixed_to_real(v.x), fixed_to_real(v.y) };
}

static inline FVec2 fvec2_add(FVec2 a, FVec2 b)
{
    return (FVec2){ a.x + b.x, a.y + b.y };
}

static inline FVec2 fvec2_sub(FVec2 a, FVec2 b)
{
    return (FVec2){ a.x - b.x, a.y - b.y };
}

static inline FVec2 fvec2_mul(FVec2 a, FVec2 b)
{
    return (FVec2){ fixed_mul(a.x, b.x), fixed_mul(a.y, b.y) };
}

static inline FVec2 fvec2_scale(FVec2 a, fixed_t b)
{
    return (FVec2){ fixed_mul(a.x, b), fixed_mul(a.y, b) };
}

static inline FVec2 fvec2_scalef(FVec2 a, real_t b)
{
    return fvec2_scale(a, fixed_from_real(b));
}

static inline fixed_t fvec2_dot(FVec2 a, FVec2 b)
{
    return fixed_mul(a.x, b.x) + fixed_mul(a.y, b.y);
}

static inline fixed_t fvec2_cross(FVec2 a, FVec2 b)
{
    return fixed_mul(a.x, b.y) - fixed_mul(a.y, b.x);
}

static inline fixedw_t fvec2_dotw(FVec2 a, FVec2 b)
{
    return (fixedw_t)a.x * b.x + (fixedw_t)a.y * b.y;
}

static inline fixedw_t fvec2_crossw(FVec2 a, FVec2 b)
{
    return (fixedw_t)a.x * b.y - (fixedw_t)a.y * b.x;
}

static inline fixedw_t fvec2_len2(FVec2 a)
{
    return (fixedw_t)a.x * a.x + (fixedw_t)a.y * a.y;
}

static inline real_t fvec2_len_real(FVec2 a)
{
    real_t x = fixed_to_real(a.x);
    real_t y = fixed_to_real(a.y);
    return sqrt(x*x+y*y);
}

static inline fixed_t fvec2_len(FVec2 a)
{
    return fixed_from_real(fvec2_len_real(a));
}

static inline fixedw_t fvec2_distance2(FVec2 a, FVec2 b)
{
    FVec2 l = fvec2_sub(b, a);
    return fvec2_len2(l);
}

static inline fixed_t fvec2_distance(FVec2 a, FVec2 b)
{
    FVec2 l = fvec2_sub(b, a);
    return fvec2_len(l);
}

static inline int fvec2_within(FVec2 a, FVec2 b, fixed_t radius)
{
    FVec2 d = fvec2_sub(a, b);
    fixedw_t distSq = (fixedw_t)d.x * d.x + (fixedw_t)d.y * d.y;
    fixedw_t radiusSq = (fixedw_t)radius * radius;
    return distSq <= radiusSq;
}

static inline FVec2 fvec2_normalize(FVec2 v)
{
    real_t x = fixed_to_real(v.x);
    real_t y = fixed_to_real(v.y);
    real_t len = sqrt(x*x + y*y);

    if(len == 0.0f)
        return (FVec2){ 0, 0 };

    return (FVec2) { fixed_from_real(x/len), fixed_from_real(y/len) };
}

static inline bool fvec2_eq(FVec2 a, FVec2 b)
{
    return a.x == b.x && a.y == b.y;
}

static inline FVec2 fvec2_min(FVec2 a, FVec2 b)
{
    return (FVec2){ min(a.x, b.x), min(a.y, b.y) };
}

static inline FVec2 fvec2_max(FVec2 a, FVec2 b)
{
    return (FVec2){ max(a.x, b.x), max(a.y, b.y) };
}

typedef struct
{
    fixedw_t num;
    fixedw_t den;
} Frac;

static inline Frac frac_make(fixedw_t num, fixedw_t den)
{
    if(den < 0) 
    { 
        num = -num; 
        den = -den; 
    }
    return (Frac){ num, den };
}

static inline bool frac_is_zero(Frac f) { return f.num == 0; }
static inline bool frac_is_one(Frac f) { return f.num == f.den; }
static inline bool frac_lte_zero(Frac f) { return f.num <= 0; }
static inline bool frac_gte_one(Frac f) { return f.num >= f.den; }
static inline bool frac_lt_zero(Frac f) { return f.num < 0; }
static inline bool frac_gt_one(Frac f) { return f.num > f.den; }

