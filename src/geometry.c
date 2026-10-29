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

bool PointInSector(MapSector *sector, Vec2 point)
{
    polygon_t *poly = PolygonFromMapLinesArena(&tmpArena, sector->numOuterLines, sector->outerLines);
    arena_reset(&tmpArena);
    return PointInPolygon(poly, point);
}

bool PointInSector2(MapSector *sector, Vec2 point)
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

bool PointInPolygon(polygon_t *polygon, Vec2 point)
{
    return PointInPolygonVector(polygon->length, (Vec2*)polygon->vertices, point);
}

bool PointInPolygonVector(size_t numVertices, Vec2 vertices[static numVertices], Vec2 point)
{
    bool inside = false;
    for(size_t i = 0; i < numVertices; ++i)
    {
        Vec2 A = vertices[i];
        Vec2 B = vertices[(i+1) % numVertices];

        if ((point.x == A.x && point.y == A.y) || (point.x == B.x && point.y == B.y)) break;
        if (A.y == B.y && point.y == A.y && between(point.x, A.x, B.x)) break;

        if (between(point.y, A.y, B.y))
        { // if P inside the vertical range
            // filter out "ray pass vertex" problem by treating the line a little lower
            if ((point.y == A.y && B.y >= A.y) || (point.y == B.y && A.y >= B.y)) continue;
            // calc cross product `PA X PB`, P lays on left side of AB if c > 0
            real_t c = (A.x - point.x) * (B.y - point.y) - (B.x - point.x) * (A.y - point.y);
            if (c == 0) break;
            if ((A.y < B.y) == (c > 0)) inside = !inside;
        }
    }
    return inside;
}

real_t MinDistToLine(Vec2 a, Vec2 b, Vec2 point)
{
    real_t l2 = vec2_distance2(a, b);
    if(eq(l2, 0)) return vec2_distance2(point, a);
    real_t t = ((point.x - a.x) * (b.x - a.x) + (point.y - a.y) * (b.y - a.y)) / l2;
    t = max(0, min(1, t));
    Vec2 tmp = { .x = a.x + t * (b.x - a.x), .y = a.y + t * (b.y - a.y) };
    return vec2_distance(point, tmp);
}

int SideOfMapLine(MapLine *line, Vec2 point)
{
    return SideOfLine(line->a->pos, line->b->pos, point);
}

int SideOfLine(Vec2 a, Vec2 b, Vec2 point)
{
    real_t c = (point.y - a.y) * (b.x - a.x) - (point.x - a.x) * (b.y - a.y);
    return (c > 0) - (c < 0);
}

BoundingBox BoundingBoxFromVertices(size_t numVertices, Vec2 vertices[static numVertices])
{
    Vec2 min = { .x = REAL_MAX, .y = REAL_MAX }, max = { .x = REAL_MIN, .y = REAL_MIN };
    for(size_t i = 0; i < numVertices; ++i)
    {
        Vec2 vert = vertices[i];
        max = vec2_max(vert, max);
        min = vec2_min(vert, min);
    }
    return (BoundingBox){ .min = min, .max = max };
}

BoundingBox BoundingBoxFromMapLines(size_t numLines, MapLine *lines[static numLines])
{
    Vec2 min = { .x = REAL_MAX, .y = REAL_MAX }, max = { .x = REAL_MIN, .y = REAL_MIN };
    for(size_t i = 0; i < numLines; ++i)
    {
        Vec2 vert = lines[i]->a->pos;
        max = vec2_max(vert, max);
        min = vec2_min(vert, min);
        vert = lines[i]->b->pos;
        max = vec2_max(vert, max);
        min = vec2_min(vert, min);
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
    (void)line;
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

    Vec2 va = aa == common ? ba->pos : aa->pos, vb = ba == common ? bb->pos : ba->pos, vc = common->pos;
    return AngleOf(va, vc, vb);
}

angle_t AngleOfLines(line_t a, line_t b)
{
    // assume a.a and b.a are equal
    return AngleOf(a.b, a.a, b.b);
}

angle_t AngleOf(Vec2 a, Vec2 b, Vec2 c)
{
    real_t angleBA = atan2(a.y - b.y, a.x - b.x);
    real_t angleBC = atan2(c.y - b.y, c.x - b.x);

    real_t diff = angleBC - angleBA;
    while(diff < 0) diff += PI2;
    while(diff >= PI2) diff -= PI2;
    return diff;
}

Vec2 LineGetClosestPoint(line_t line, Vec2 pos)
{
    Vec2 dir = vec2_normalize(vec2_sub(line.b, line.a));
    real_t len = vec2_distance(line.a, line.b);
    Vec2 lhs = vec2_sub(pos, line.a);
    real_t dotP = vec2_dot(lhs, dir);
    dotP = clamp(0.0f, len, dotP);
    return vec2_add(line.a, vec2_scale(dir, dotP));
}

static inline real_t snapToGrid(real_t value, int gridSize)
{
    real_t halfGrid = (real_t)gridSize / 2;
    int cells = floor((value + halfGrid) / gridSize);
    return (cells * gridSize);
}

Vec2 LineGetClosestPointGrid(line_t line, Vec2 pos, int gridSize)
{
    Vec2 dir = vec2_sub(line.b, line.a);
    real_t dx = dir.x;
    real_t dy = dir.y;

    real_t minX = min(line.a.x, line.b.x);
    real_t maxX = max(line.a.x, line.b.x);
    real_t minY = min(line.a.y, line.b.y);
    real_t maxY = max(line.a.y, line.b.y);

    if (eq(dx, 0))
    {
        // Vertical line: x is fixed, snap y to nearest grid line, clamped to the segment
        //fixed_t y = snapToGridPow2(pos.y, gridSize);
        real_t y = snapToGrid(pos.y, gridSize);
        y = clamp(y, minY, maxY);
        return (Vec2){ .x = line.a.x, .y = y };
    }

    if (eq(dy, 0))
    {
        // Horizontal line: y is fixed, snap x to nearest grid line, clamped to the segment
        real_t x = snapToGrid(pos.x, gridSize);
        x = clamp(x, minX, maxX);
        return (Vec2){ .x = x, .y = line.a.y };
    }

    real_t m = dy / dx;

    // Candidate A: snap x to nearest vertical grid line, solve y along the line
    real_t candXx = snapToGrid(pos.x, gridSize);
    candXx = clamp(candXx, minX, maxX);
    real_t candXy = line.a.y + (m * (candXx - line.a.x));
    candXy = clamp(candXy, minY, maxY);
    Vec2 candX = { .x = candXx, .y = candXy };

    // Candidate B: snap y to nearest horizontal grid line, solve x along the line
    real_t candYy = snapToGrid(pos.y, gridSize);
    candYy = clamp(candYy, minY, maxY);
    real_t candYx = line.a.x + ((candYy - line.a.y) / m);
    candYx = clamp(candYx, minX, maxX);
    Vec2 candY = { .x = candYx, .y = candYy };

    real_t distXsq = vec2_len2(vec2_sub(candX, pos));
    real_t distYsq = vec2_len2(vec2_sub(candY, pos));

    return (distXsq <= distYsq) ? candX : candY;
}

bool LineOverlap(line_t la, line_t lb, intersection_res_t *res)
{
    Vec2 u = vec2_sub(la.b, la.a);
    Vec2 v = vec2_sub(lb.b, lb.a);
    Vec2 w = vec2_sub(lb.a, la.a);

    if(u.x == 0 && u.y == 0)
        return false;

    if(vec2_cross(u, v) != 0)
        return false;

    if(vec2_cross(u, w) != 0)
        return false;

    Vec2 w2 = vec2_sub(lb.b, la.a);
    real_t denom; real_t num0, num1;
    if(fabs(u.x) >= fabs(u.y))
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

    real_t t0 = num0 / denom;
    real_t t1 = num1 / denom;

   if((t0 <= 0 && t1 <= 0) || (t0 >= 1 && t1 >= 1))
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

Vec2 Materialize(Vec2 origin, Vec2 dir, real_t t)
{
    return (Vec2)
    {
        .x = origin.x + (dir.x * t),
        .y = origin.y + (dir.y * t)
    };
}

bool LineIntersection(line_t la, line_t lb, intersection_res_t *res)
{
    Vec2 u = vec2_sub(la.b, la.a);
    Vec2 v = vec2_sub(lb.b, lb.a);
    Vec2 w = vec2_sub(la.a, lb.a);

    real_t D = vec2_cross(u, v);
    if(D == 0)
        return false;

    real_t sI = vec2_cross(v, w) / D;
    real_t tI = vec2_cross(u, w) / D;

    if(sI < 0 || sI > 1)
        return false;
    if(tI < 0 || tI > 1)
        return false;

    if(res)
    {
        bool sIs0 = eq(sI, 0); //frac_is_zero(sI);
        bool sIs1 = eq(sI, 1); //frac_is_one(sI);
        bool tIs0 = eq(tI, 0); //frac_is_zero(tI);
        bool tIs1 = eq(tI, 1); //frac_is_one(tI);

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

enum orientation_t LineLoopOrientation(size_t numVertices, Vec2 vertices[static numVertices])
{
    real_t res = 0;
    for(size_t i = 0; i < numVertices; ++i)
    {
        Vec2 a = vertices[i];
        Vec2 b = vertices[(i+1)%numVertices];

        res += (b.x - a.x) * (b.y + a.y);
    }
    if(res == 0)
        return DEGENERATE_ORIENT;
    return res >= 0 ? CCW_ORIENT : CW_ORIENT;
}

