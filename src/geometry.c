#include "geometry.h"

#include <assert.h>
#include <stdlib.h>
#include <float.h>
#include <tgmath.h>

#include "arena.h"

#include "logging.h"
#include "map/util.h"
#include "utils.h"
#include "vecmath.h"

static Arena tmpArena = { 0 };

bool PointInSector(MapSector *sector, FVec2 point)
{
    polygon_t *poly = PolygonFromMapLinesArena(&tmpArena, sector->numOuterLines, sector->outerLines);
    arena_reset(&tmpArena);
    return PointInPolygon(poly, point);
}

bool PointInSector2(MapSector *sector, FVec2 point)
{
    bool inside = PointInSector(sector, point);
    for(size_t i = 0; i < sector->numInnerLines; ++i)
    {
        polygon_t *poly = PolygonFromMapLinesArena(&tmpArena, sector->numInnerLinesNum[i], sector->innerLines[i]);
        inside &= !PointInPolygon(poly, point);
    }
    arena_reset(&tmpArena);
    return inside;
}

bool PointInPolygon(polygon_t *polygon, FVec2 point)
{
    return PointInPolygonVector(polygon->length, (FVec2*)polygon->vertices, point);
}

bool PointInPolygonVector(size_t numVertices, FVec2 vertices[static numVertices], FVec2 point)
{
    bool inside = false;
    for(size_t i = 0; i < numVertices; ++i)
    {
        FVec2 A = vertices[i];
        FVec2 B = vertices[(i+1) % numVertices];

        if ((point.x == A.x && point.y == A.y) || (point.x == B.x && point.y == B.y)) break;
        if (A.y == B.y && point.y == A.y && between(point.x, A.x, B.x)) break;

        if (between(point.y, A.y, B.y))
        { // if P inside the vertical range
            // filter out "ray pass vertex" problem by treating the line a little lower
            if ((point.y == A.y && B.y >= A.y) || (point.y == B.y && A.y >= B.y)) continue;
            // calc cross product `PA X PB`, P lays on left side of AB if c > 0
            fixedw_t c = (fixedw_t)(A.x - point.x) * (fixedw_t)(B.y - point.y) - (fixedw_t)(B.x - point.x) * (fixedw_t)(A.y - point.y);
            if (c == 0) break;
            if ((A.y < B.y) == (c > 0)) inside = !inside;
        }
    }
    return inside;
}

// this is only used for selection so use normal floating point math
real_t MinDistToLine(FVec2 fa, FVec2 fb, FVec2 fpoint)
{
    Vec2 a = vec2_from_fvec2(fa), b = vec2_from_fvec2(fb), point = vec2_from_fvec2(fpoint);

    real_t l2 = vec2_distance2(a, b);
    if(eq(l2, 0)) return vec2_distance2(point, a);
    real_t t = ((point.x - a.x) * (b.x - a.x) + (point.y - a.y) * (b.y - a.y)) / l2;
    t = max(0, min(1, t));
    Vec2 tmp = { .x = a.x + t * (b.x - a.x), .y = a.y + t * (b.y - a.y) };
    return vec2_distance(point, tmp);
}

int SideOfMapLine(MapLine *line, FVec2 point)
{
    return SideOfLine(line->a->pos, line->b->pos, point);
}

int SideOfLine(FVec2 a, FVec2 b, FVec2 point)
{
    fixedw_t c = (fixedw_t)(point.y - a.y) * (fixedw_t)(b.x - a.x) - (fixedw_t)(point.x - a.x) * (fixedw_t)(b.y - a.y);
    return (c > 0) - (c < 0);
}

BoundingBox BoundingBoxFromVertices(size_t numVertices, FVec2 vertices[static numVertices])
{
    Vec2 min = { .x = REAL_MAX, .y = REAL_MAX }, max = { .x = REAL_MIN, .y = REAL_MIN };
    for(size_t i = 0; i < numVertices; ++i)
    {
        Vec2 vert = vec2_from_fvec2(vertices[i]);
        max = vec2_maxv(vert, max);
        min = vec2_minv(vert, min);
    }
    return (BoundingBox){ .min = min, .max = max };
}

BoundingBox BoundingBoxFromVerticesReal(size_t numVertices, Vec2 vertices[static numVertices])
{
    Vec2 min = { .x = REAL_MAX, .y = REAL_MAX }, max = { .x = REAL_MIN, .y = REAL_MIN };
    for(size_t i = 0; i < numVertices; ++i)
    {
        Vec2 vert = vertices[i];
        max = vec2_maxv(vert, max);
        min = vec2_minv(vert, min);
    }
    return (BoundingBox){ .min = min, .max = max };
}

BoundingBox BoundingBoxFromMapLines(size_t numLines, MapLine *lines[static numLines])
{
    Vec2 min = { .x = REAL_MAX, .y = REAL_MAX }, max = { .x = REAL_MIN, .y = REAL_MIN };
    for(size_t i = 0; i < numLines; ++i)
    {
        Vec2 vert = vec2_from_fvec2(lines[i]->a->pos);
        max = vec2_maxv(vert, max);
        min = vec2_minv(vert, min);
        vert = vec2_from_fvec2(lines[i]->b->pos);
        max = vec2_maxv(vert, max);
        min = vec2_minv(vert, min);
    }
    return (BoundingBox){ .min = min, .max = max };
}

bool BoundingBoxIntersect(BoundingBox a, BoundingBox b)
{
    return a.min.x > b.min.x && a.min.x < b.max.x && a.max.x < b.max.x && a.min.y > b.min.y && a.min.y < b.max.y && a.max.y < b.max.y;
}

angle_t NormalizeAngle(angle_t angle)
{
    while(angle < 0.0f) angle += PI2;
    while(angle >= PI2) angle -= PI2;
    return angle;
}

angle_t AngleDifference(angle_t a, angle_t b)
{
    angle_t d = NormalizeAngle(a) - NormalizeAngle(b);

    // Make corrections for zero barrier
    if(d < 0.0f) d += PI2;
    if(d > PI) d = PI2 - d;

    return d;
}

angle_t AngleLine(MapLine *line)
{
    return 0;
}

angle_t AngleOfMapLines(MapLine *a, MapLine *b)
{
    MapVertex *aa = a->a;
    //MapVertex *ab = a->b;
    MapVertex *ba = b->a;
    MapVertex *bb = b->b;

    MapVertex *common = aa == ba ? ba : aa == bb ? bb : NULL;
    assert(common);

    FVec2 va = aa == common ? ba->pos : aa->pos, vb = ba == common ? bb->pos : ba->pos, vc = common->pos;
    return AngleOf(vec2_from_fvec2(va), vec2_from_fvec2(vc), vec2_from_fvec2(vb));
}

angle_t AngleOfLines(line_t a, line_t b)
{
    // assume a.a and b.a are equal
    return AngleOf(vec2_from_fvec2(a.b), vec2_from_fvec2(a.a), vec2_from_fvec2(b.b));
}

angle_t AngleOf(Vec2 a, Vec2 b, Vec2 c)
{
    real_t angleBA = atan2(a.y - b.y, a.x - b.x);
    real_t angleBC = atan2(c.y - b.y, c.x - b.x);

    real_t diff = angleBC - angleBA;
    while(diff < 0) diff += PI2;
    while(diff >= PI2) diff -= PI2;
    return diff;
#if 0
    Vec2 ab = {b.x - a.x, b.y - a.y};
    Vec2 cb = {b.x - c.x, b.y - c.y};

    // dot product
    real_t dot = (ab.x * cb.x + ab.y * cb.y);

    // length square of both vectors
    real_t abSqr = ab.x * ab.x + ab.y * ab.y;
    real_t cbSqr = cb.x * cb.x + cb.y * cb.y;

    // square of cosine of the needed angle
    real_t cosSqr = dot * dot / abSqr / cbSqr;

    // this is a known trigonometric equality:
    // cos(alpha * 2) = [ cos(alpha) ]^2 * 2 - 1
    real_t cos2 = 2.0f * cosSqr - 1.0f;

    // Here's the only invocation of the heavy function.
    // It's a good idea to check explicitly if cos2 is within [-1 .. 1] range
    real_t alpha2 =
        (cos2 <= -1) ? PI :
        (cos2 >= 1) ? 0.0 :
        acos(cos2);

    real_t rs = alpha2 * 0.5;

    // Now revolve the ambiguities.
    // 1. If dot product of two vectors is negative - the angle is definitely
    // above 90 degrees. Still we have no information regarding the sign of the angle.

    // NOTE: This ambiguity is the consequence of our method: calculating the cosine
    // of the double angle. This allows us to get rid of calling sqrt.
    if(dot < 0) rs = PI - rs;

    // 2. Determine the sign. For this we'll use the Determinant of two vectors.
    real_t det = (ab.x * cb.y - ab.y * cb.x);
    if(det < 0) rs = (2.0 * PI) - rs;

    return rs;
#endif
}

FVec2 LineGetClosestPoint(line_t line, FVec2 pos)
{
#if 0
    Vec2 dir = vec2_normalize(vec2_sub(line.b, line.a));
    real_t len = vec2_distance(line.a, line.b);
    Vec2 lhs = vec2_sub(pos, line.a);
    real_t dotP = vec2_dot(lhs, dir);
    dotP = clamp(0.0f, len, dotP);
    return vec2_add(line.a, vec2_scale(dir, dotP));
#endif
    return (FVec2){ 0 };
}

static inline int64_t floordiv(fixed_t a, fixed_t b)
{
    int64_t q = a / b;
    int64_t r = a % b;
    if (r != 0 && ((r < 0) != (b < 0)))
        q -= 1;
    return q;
}

static inline fixed_t snapToGrid(fixed_t value, fixed_t gridSize, fixed_t halfGrid)
{
    int64_t cells = floordiv(value + halfGrid, gridSize);
    return (fixed_t)(cells * gridSize);
}

static inline fixed_t snapToGridPow2(fixed_t value, int gridSize)
{ 
    assert((gridSize & (gridSize - 1)) == 0 && "gridSize must be a power of 2");

    fixed_t gridFixed = fixed_from_int(gridSize);
    fixed_t halfGrid = gridFixed / 2;
    fixed_t mask = gridFixed - 1;

    fixed_t shifted = value + halfGrid;
    return shifted & ~mask;
}

FVec2 LineGetClosestPointGrid(line_t line, FVec2 pos, int gridSize)
{
    FVec2 dir = fvec2_sub(line.b, line.a);
    fixed_t dx = dir.x;
    fixed_t dy = dir.y;

    fixed_t minX = min(line.a.x, line.b.x);
    fixed_t maxX = max(line.a.x, line.b.x);
    fixed_t minY = min(line.a.y, line.b.y);
    fixed_t maxY = max(line.a.y, line.b.y);

    if (dx == 0)
    {
        // Vertical line: x is fixed, snap y to nearest grid line, clamped to the segment
        fixed_t y = snapToGridPow2(pos.y, gridSize);
        y = clamp(y, minY, maxY);
        return (FVec2){ .x = line.a.x, .y = y };
    }

    if (dy == 0)
    {
        // Horizontal line: y is fixed, snap x to nearest grid line, clamped to the segment
        fixed_t x = snapToGridPow2(pos.x, gridSize);
        x = clamp(x, minX, maxX);
        return (FVec2){ .x = x, .y = line.a.y };
    }

    fixed_t m = fixed_div(dy, dx);

    // Candidate A: snap x to nearest vertical grid line, solve y along the line
    fixed_t candXx = snapToGridPow2(pos.x, gridSize);
    candXx = clamp(candXx, minX, maxX);
    fixed_t candXy = line.a.y + fixed_mul(m, candXx - line.a.x);
    candXy = clamp(candXy, minY, maxY);
    FVec2 candX = { .x = candXx, .y = candXy };

    // Candidate B: snap y to nearest horizontal grid line, solve x along the line
    fixed_t candYy = snapToGridPow2(pos.y, gridSize);
    candYy = clamp(candYy, minY, maxY);
    fixed_t candYx = line.a.x + fixed_div(candYy - line.a.y, m);
    candYx = clamp(candYx, minX, maxX);
    FVec2 candY = { .x = candYx, .y = candYy };

    fixedw_t distXsq = fvec2_len2(fvec2_sub(candX, pos));
    fixedw_t distYsq = fvec2_len2(fvec2_sub(candY, pos));

    return (distXsq <= distYsq) ? candX : candY;
}

bool LineOverlap(line_t la, line_t lb, intersection_res_t *res)
{
    FVec2 u = fvec2_sub(la.b, la.a);
    FVec2 v = fvec2_sub(lb.b, lb.a);
    FVec2 w = fvec2_sub(lb.a, la.a);

    if(u.x == 0 && u.y == 0)
        return false;

    if(fvec2_crossw(u, v) != 0)
        return false;

    if(fvec2_crossw(u, w) != 0)
        return false;

    FVec2 w2 = fvec2_sub(lb.b, la.a);
    fixed_t denom; fixedw_t num0, num1;
    if(llabs((int64_t)u.x) >= llabs((int64_t)u.y))
    {
        denom = u.x;
        num0 = w.x;
        num1 = w2.x;
    }
    else
    {
        denom = u.y;
        num0 = w.y;
        num1 = w2.y;
    }

    Frac t0 = frac_make(num0, denom);
    Frac t1 = frac_make(num1, denom);

    if((frac_lte_zero(t0) && frac_lte_zero(t1)) ||
       (frac_gte_one(t0)  && frac_gte_one(t1)))
       return false;

    if(res)
    {
        res->p0 = lb.a;
        res->p1 = lb.b;
        res->u = t0;
        res->v = t1;
        res->exact = true;
    }

    return true;
}

static fixed_t roundDivWide(fixedw_t num, fixedw_t den)
{
    fixedw_t half = den / 2;
    fixedw_t adj = (num < 0) ? (num - half) : (num + half);
    return (fixed_t)(adj / den);
}

#define USE_128_INT_TYPE
#ifdef USE_128_INT_TYPE
typedef __int128 fixedww_t;
static fixed_t roundDivWide128(fixedww_t num, fixedww_t den)
{
    fixedww_t half = den / 2;
    fixedww_t adj = (num < 0) ? (num - half) : (num + half);
    return (fixed_t)(adj / den);
}
#else
static void umul64(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo)
{
    uint64_t a_lo = (uint32_t)a, a_hi = a >> 32;
    uint64_t b_lo = (uint32_t)b, b_hi = b >> 32;

    uint64_t t0 = a_lo * b_lo;
    uint64_t t1 = a_hi * b_lo + (t0 >> 32);
    uint64_t t2 = a_lo * b_hi + (uint32_t)t1;

    *hi = a_hi * b_hi + (t1 >> 32) + (t2 >> 32);
    *lo = (t2 << 32) | (uint32_t)t0;
}

static uint64_t udiv128(uint64_t hi, uint64_t lo, uint64_t divisor)
{
    uint64_t quotient = 0, remainder = 0;
    for(int i = 127; i >= 0; --i)
    {
        remainder = (remainder << 1) | ((i >= 64 ? (hi >> (i - 64)) : (lo >> i)) & 1);
        quotient <<= 1;
        if(remainder >= divisor)
        {
            remainder -= divisor;
            quotient |= 1;
        }
    }
    return quotient;
}

static fixed_t muldiv_round(fixed_t a, fixedw_t b, fixedw_t c)
{
    bool negative = a < 0;
    uint64_t ua = negative ? (uint64_t)(-(int64_t)a) : (uint64_t)a;
    uint64_t ub = (uint64_t)b;
    uint64_t uc = (uint64_t)c;

    uint64_t hi, lo;
    umul64(ua, ub, &hi, &lo);

    uint64_t half = uc / 2;
    uint64_t newLo = lo + half;
    if(newLo < lo) hi++;
    lo = newLo;

    uint64_t result = udiv128(hi, lo, uc);
    return negative ? -(fixed_t)result : (fixed_t)result;
}
#endif

FVec2 Materialize(FVec2 origin, FVec2 dir, Frac t)
{
#ifdef USE_128_INT_TYPE
    fixedww_t dx = (fixedww_t)dir.x * (fixedww_t)t.num;
    fixedww_t dy = (fixedww_t)dir.y * (fixedww_t)t.num;
    return (FVec2)
    {
        .x = origin.x + roundDivWide128(dx, t.den),
        .y = origin.y + roundDivWide128(dy, t.den)
    };
#else
    return (FVec2)
    {
        .x = origin.x + muldiv_round(dir.x, t.num, t.den),
        .y = origin.y + muldiv_round(dir.y, t.num, t.den)
    };
#endif
}

bool LineIntersection(line_t la, line_t lb, intersection_res_t *res)
{
    FVec2 u = fvec2_sub(la.b, la.a);
    FVec2 v = fvec2_sub(lb.b, lb.a);
    FVec2 w = fvec2_sub(la.a, lb.a);

    fixedw_t D = fvec2_crossw(u, v);
    if(D == 0)
        return false;

    Frac sI = frac_make(fvec2_crossw(v, w), D);
    Frac tI = frac_make(fvec2_crossw(u, w), D);

    if(frac_lt_zero(sI) || frac_gt_one(sI))
        return false;
    if(frac_lt_zero(tI) || frac_gt_one(tI))
        return false;

    if(res)
    {
        bool sIs0 = frac_is_zero(sI);
        bool sIs1 = frac_is_one(sI);
        bool tIs0 = frac_is_zero(tI);
        bool tIs1 = frac_is_one(tI);

        if(sIs0)        res->p0 = la.a;
        else if(sIs1)   res->p0 = la.b;
        else if(tIs0)   res->p0 = lb.a;
        else if(tIs1)   res->p0 = lb.b;
        else            res->p0 = Materialize(la.a, u, sI);

        res->u = sI;
        res->v = tI;
        res->exact = sIs0 || sIs1 || tIs0 || tIs1;
    }

    return true;
}

enum orientation_t LineLoopOrientation(size_t numVertices, FVec2 vertices[static numVertices])
{
    fixedw_t res = 0;
    for(size_t i = 0; i < numVertices; ++i)
    {
        FVec2 a = vertices[i];
        FVec2 b = vertices[(i+1)%numVertices];

        fixedw_t dx = (fixedw_t)b.x - (fixedw_t)a.x;
        fixedw_t sy = (fixedw_t)b.y + (fixedw_t)a.y;

        res += dx * sy;

        //res += fixed_mul((b.x - a.x), (b.y + a.y));
    }
    return res >= 0 ? CCW_ORIENT : CW_ORIENT;
}

enum orientation_t LineLoopOrientationReal(size_t numVertices, Vec2 vertices[static numVertices])
{
    real_t res = 0;
    for(size_t i = 0; i < numVertices; ++i)
    {
        Vec2 a = vertices[i];
        Vec2 b = vertices[(i+1)%numVertices];

        res += (b.x - a.x) * (b.y + a.y);
    }
    return res >= 0 ? CCW_ORIENT : CW_ORIENT;
}

