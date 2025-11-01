#include "geometry.h"

#include <assert.h>
#include <stdlib.h>
#include <float.h>

#include "map/util.h"
#include "utils.h"

bool PointInSector(MapSector *sector, Vec2 point)
{
    return PointInPolygonVector(sector->numOuterLines, sector->edData.vertices, point);
}

bool PointInSector2(MapSector *sector, Vec2 point)
{
    bool inside = PointInSector(sector, point);
    for(size_t i = 0; i < sector->numInnerLines; ++i)
    {
        struct Polygon *poly = PolygonFromMapLines(sector->numInnerLinesNum[i], sector->innerLines[i]);
        inside &= !PointInPolygon(poly, point);
        free(poly);
    }
    return inside;
}

bool PointInPolygon(struct Polygon *polygon, Vec2 point)
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

        if ((eq(point.x, A.x) && eq(point.y, A.y)) || (eq(point.x, B.x) && eq(point.y, B.y))) break;
        if (eq(A.y, B.y) && eq(point.y, A.y) && between(point.x, A.x, B.x)) break;

        if (between(point.y, A.y, B.y))
        { // if P inside the vertical range
            // filter out "ray pass vertex" problem by treating the line a little lower
            if ((eq(point.y, A.y) && B.y >= A.y) || (eq(point.y, B.y) && A.y >= B.y)) continue;
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
    return (point.y - a.y) * (b.x - a.x) - (point.x - a.x) * (b.y - a.y);
}

BoundingBox BoundingBoxFromVertices(size_t numVertices, Vec2 vertices[static numVertices])
{
    Vec2 min = { .x = DBL_MAX, .y = DBL_MAX }, max = { .x = DBL_MIN, .y = DBL_MIN };
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
    Vec2 min = { .x = DBL_MAX, .y = DBL_MAX }, max = { .x = DBL_MIN, .y = DBL_MIN };
    for(size_t i = 0; i < numLines; ++i)
    {
        Vec2 vert = lines[i]->a->pos;
        max = vec2_maxv(vert, max);
        min = vec2_minv(vert, min);
        vert = lines[i]->b->pos;
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
}

bool LineIsCollinear(line_t la, line_t lb)
{
    Vec2 u = vec2_sub(la.b, la.a);
    Vec2 v = vec2_sub(lb.b, lb.a);
    Vec2 w = vec2_sub(la.a, lb.a);
    real_t D = vec2_cross(u, v);

    if(fabs(D) > SMALL_NUM)
        return false;

    if(fabs(vec2_cross(u, w)) < SMALL_NUM && fabs(vec2_cross(v, w)) < SMALL_NUM)
        return true;

    return false;
}

bool LineIsParallel(line_t a, line_t b)
{
    Vec2 u = vec2_sub(a.b, a.a);
    Vec2 v = vec2_sub(b.b, b.a);
    real_t D = vec2_cross(u, v);
    return fabs(D) < SMALL_NUM;
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

Vec2 LineGetCommonPoint(line_t major, line_t support)
{
    if(vec2_eqv(major.a, support.a)) return major.a;
    if(vec2_eqv(major.b, support.a)) return major.b;
    if(vec2_eqv(major.a, support.b)) return major.a;
    //if(vec2_eqv_eps(major.b, support.b)) return major.b;
    return major.b;
}

real_t LineGetPointFactor(line_t line, Vec2 point)
{
    //Vec2 u = vec2_sub(line.b, line.a);
    real_t len = vec2_distance(line.b, line.a);
    return vec2_distance(line.a, point) / len;
}

bool LineOverlap(line_t la, line_t lb, intersection_res_t *res)
{
    Vec2 u = vec2_sub(la.b, la.a);
    Vec2 v = vec2_sub(lb.b, lb.a);
    Vec2 w = vec2_sub(lb.a, la.a);
    real_t D = vec2_cross(u, v);

    if(eq(mag2(u), 0))
        return false;

    if(fabs(D) > SMALL_NUM)
        return false;

    real_t D2 = vec2_cross(u, w), D3 = vec2_cross(v, w);
    if(fabs(D2) > SMALL_NUM || fabs(D3) > SMALL_NUM)
        return false;

    real_t t0, t1;
    Vec2 w2 = vec2_sub(lb.b, la.a);
    if(!eq(u.x, 0))
    {
        t0 = w.x / u.x;
        t1 = w2.x / u.x;
    }
    else
    {
        t0 = w.y / u.y;
        t1 = w2.y / u.y;
    }

    //if(((t0 - SMALL_NUM) <= 0 && (t1 - SMALL_NUM) <= 0) || ((t0 + SMALL_NUM) >= 1 && (t1 + SMALL_NUM) >= 1))
    if((lte(t0, 0) && lte(t1, 0)) || (gte(t0, 1) && gte(t1, 1)))
        return false;

    if(res)
    {
        res->p0 = vec2_add(la.a, vec2_scale(u, t0));
        res->p1 = vec2_add(la.a, vec2_scale(u, t1));
        res->u = t0;
        res->v = t1;
    }

    return true;
}

#undef SMALL_NUM
#define SMALL_NUM 0.00001
bool LineIntersection(line_t la, line_t lb, intersection_res_t *res)
{
    Vec2 u = vec2_sub(la.b, la.a);
    Vec2 v = vec2_sub(lb.b, lb.a);
    Vec2 w = vec2_sub(la.a, lb.a);
    real_t D = vec2_cross(u, v);

    if(fabs(D) < SMALL_NUM)
        return false;

    real_t sI = vec2_cross(v, w) / D;
    real_t tI = vec2_cross(u, w) / D;

    if((sI + SMALL_NUM) < 0 || (sI - SMALL_NUM) > 1)
        return false;

    if((tI + SMALL_NUM) < 0 || (tI - SMALL_NUM) > 1)
        return false;

    if(res)
    {
        res->p0 = vec2_add(la.a, vec2_scale(u, sI));
        res->u = sI;
        res->v = tI;
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
    return res >= 0 ? CCW_ORIENT : CW_ORIENT;
}
