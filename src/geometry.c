#include "geometry.h"

#include "common.h"

#include <assert.h>
#include <tgmath.h>
#include <limits.h>

#include <cglm/ivec2.h>

bool PointInSector(struct MapSector sector[static 1], vec2s point)
{
    return PointInPolygon(sector->numOuterLines, sector->vertices, point);
}

bool PointInPolygon(size_t numVertices, vec2s vertices[static numVertices], vec2s point)
{
    bool inside = false;
    for(size_t i = 0; i < numVertices; ++i)
    {
        vec2s A = vertices[i];
        vec2s B = vertices[(i+1) % numVertices];

        if ((point.x == A.x && point.y == A.y) || (point.x == B.x && point.y == B.y)) break;
        if (A.y == B.y && point.y == A.y && between(point.x, A.x, B.x)) break;

        if (between(point.y, A.y, B.y))
        { // if P inside the vertical range
            // filter out "ray pass vertex" problem by treating the line a little lower
            if ((point.y == A.y && B.y >= A.y) || (point.y == B.y && A.y >= B.y)) continue;
            // calc cross product `PA X PB`, P lays on left side of AB if c > 0
            float c = (A.x - point.x) * (B.y - point.y) - (B.x - point.x) * (A.y - point.y);
            if (c == 0) break;
            if ((A.y < B.y) == (c > 0)) inside = !inside;
        }
    }
    return inside;
}

float MinDistToLine(vec2s a, vec2s b, vec2s point)
{
    float l2 = glms_vec2_distance2(a, b);
    if(l2 == 0) return glms_vec2_distance2(point, a);
    float t = ((point.x - a.x) * (b.x - a.x) + (point.y - a.y) * (b.y - a.y)) / l2;
    t = max(0, min(1, t));
    vec2s tmp = { .x = a.x + t * (b.x - a.x), .y = a.y + t * (b.y - a.y) };
    return sqrt(glms_vec2_distance2(point, tmp));
}

int SideOfMapLine(struct MapLine line[static 1], vec2s point)
{
    return SideOfLine(line->a->pos, line->b->pos, point);
}

int SideOfLine(vec2s a, vec2s b, vec2s point)
{
    return (point.y - a.y) * (b.x - a.x) - (point.x - a.x) * (b.y - a.y);
}

struct BoundingBox BoundingBoxFromVertices(size_t numVertices, vec2s vertices[static numVertices])
{
    vec2s min = { .x = FLT_MAX, .y = FLT_MAX }, max = { .x = FLT_MIN, .y = FLT_MIN };
    for(size_t i = 0; i < numVertices; ++i)
    {
        vec2s vert = vertices[i];
        max = glms_vec2_maxv(vert, max);
        min = glms_vec2_minv(vert, min);
    }
    return (struct BoundingBox){ .min = min, .max = max };
}

struct BoundingBox BoundingBoxFromMapLines(size_t numLines, struct MapLine *lines[static numLines])
{
    vec2s min = { .x = FLT_MAX, .y = FLT_MAX }, max = { .x = FLT_MIN, .y = FLT_MIN };
    for(size_t i = 0; i < numLines; ++i)
    {
        vec2s vert = lines[i]->a->pos;
        max = glms_vec2_maxv(vert, max);
        min = glms_vec2_minv(vert, min);
        vert = lines[i]->b->pos;
        max = glms_vec2_maxv(vert, max);
        min = glms_vec2_minv(vert, min);
    }
    return (struct BoundingBox){ .min = min, .max = max };
}

bool BoundingBoxIntersect(struct BoundingBox a, struct BoundingBox b)
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

angle_t AngleLine(struct MapLine line[static 1])
{
    return 0;
}

angle_t AngleOfMapLines(struct MapLine a[static 1], struct MapLine b[static 1])
{
    struct MapVertex *aa = a->a;
    //struct MapVertex *ab = a->b;
    struct MapVertex *ba = b->a;
    struct MapVertex *bb = b->b;

    struct MapVertex *common = aa == ba ? ba : aa == bb ? bb : NULL;
    assert(common);

    vec2s va = aa == common ? ba->pos : aa->pos, vb = ba == common ? bb->pos : ba->pos, vc = common->pos;
    return AngleOf(va, vc, vb);
}

angle_t AngleOfLines(struct line_t a, struct line_t b)
{
    // assume a.a and b.a are equal
    return AngleOf(a.b, a.a, b.b);
}

angle_t AngleOf(vec2s a, vec2s b, vec2s c)
{
    vec2s ab = {{b.x - a.x, b.y - a.y}};
    vec2s cb = {{b.x - c.x, b.y - c.y}};

    // dot product
    float dot = (ab.x * cb.x + ab.y * cb.y);

    // length square of both vectors
    float abSqr = ab.x * ab.x + ab.y * ab.y;
    float cbSqr = cb.x * cb.x + cb.y * cb.y;

    // square of cosine of the needed angle
    float cosSqr = dot * dot / abSqr / cbSqr;

    // this is a known trigonometric equality:
    // cos(alpha * 2) = [ cos(alpha) ]^2 * 2 - 1
    float cos2 = 2.0f * cosSqr - 1.0f;

    // Here's the only invocation of the heavy function.
    // It's a good idea to check explicitly if cos2 is within [-1 .. 1] range
    float alpha2 =
        (cos2 <= -1) ? PI :
        (cos2 >= 1) ? 0.0 :
        acos(cos2);

    float rs = alpha2 * 0.5;

    // Now revolve the ambiguities.
    // 1. If dot product of two vectors is negative - the angle is definitely
    // above 90 degrees. Still we have no information regarding the sign of the angle.

    // NOTE: This ambiguity is the consequence of our method: calculating the cosine
    // of the double angle. This allows us to get rid of calling sqrt.
    if(dot < 0) rs = PI - rs;

    // 2. Determine the sign. For this we'll use the Determinant of two vectors.
    float det = (ab.x * cb.y - ab.y * cb.x);
    if(det < 0) rs = (2.0 * PI) - rs;

    return rs;
}

bool inSegment(vec2s p, struct line_t s)
{
    if(s.a.x != s.b.x)
    {
        if(s.a.x <= p.x && p.x <= s.b.x)
            return true;
        if(s.a.x >= p.x && p.x >= s.b.x)
            return true;
    }
    else
    {
        if(s.a.y <= p.y && p.y <= s.b.y)
            return true;
        if(s.a.y >= p.y && p.y >= s.b.y)
            return true;
    }
    return false;
}

bool LineIsCollinear(struct line_t la, struct line_t lb)
{
    vec2s u = glms_vec2_sub(la.b, la.a);
    vec2s v = glms_vec2_sub(lb.b, lb.a);
    vec2s w = glms_vec2_sub(la.a, lb.a);
    float D = glms_vec2_cross(u, v);

    if(fabs(D) > SMALL_NUM)
        return false;

    if(fabs(glms_vec2_cross(u, w)) < SMALL_NUM && fabs(glms_vec2_cross(v, w)) < SMALL_NUM)
        return true;

    return false;
}

bool LineIsParallel(struct line_t a, struct line_t b)
{
    vec2s u = glms_vec2_sub(a.b, a.a);
    vec2s v = glms_vec2_sub(b.b, b.a);
    float D = glms_vec2_cross(u, v);
    return fabs(D) < SMALL_NUM;
}

vec2s LineGetCommonPoint(struct line_t major, struct line_t support)
{
    if(glms_vec2_eqv_eps(major.a, support.a)) return major.a;
    if(glms_vec2_eqv_eps(major.b, support.a)) return major.b;
    if(glms_vec2_eqv_eps(major.a, support.b)) return major.a;
    //if(glms_vec2_eqv_eps(major.b, support.b)) return major.b;
    return major.b;
}

float LineGetPointFactor(struct line_t line, vec2s point)
{
    //vec2s u = glms_vec2_sub(line.b, line.a);
    float len = glms_vec2_distance(line.b, line.a);
    return glms_vec2_distance(line.a, point) / len;
}

bool LineOverlap(struct line_t la, struct line_t lb, struct intersection_res_t *res)
{
    vec2s u = glms_vec2_sub(la.b, la.a);
    vec2s v = glms_vec2_sub(lb.b, lb.a);
    vec2s w = glms_vec2_sub(la.a, lb.a);
    float D = glms_vec2_cross(u, v);

    if(fabs(D) > SMALL_NUM)
        return false;

    if(fabs(glms_vec2_cross(u, w)) > SMALL_NUM || fabs(glms_vec2_cross(v, w)) > SMALL_NUM)
        return false;

    /*
    float du = glms_vec2_dot(u, u);
    float dv = glms_vec2_dot(v, v);
    if(du == 0 && dv == 0)
    {
        if(!glms_vec2_eqv(la.a, lb.a)) return false;
        if(res) res->p0 = la.a;
        return true;
    }
    if(du == 0)
    {
        if(!inSegment(la.a, lb)) return false;
        if(res) res->p0 = la.a;
        return true;
    }
    if(dv == 0)
    {
        if(!inSegment(lb.a, la)) return false;
        if(res) res->p0 = lb.a;
        return true;
    }
    */

    float t0, t1;
    vec2s w2 = glms_vec2_sub(la.b, lb.a);
    if(v.x != 0)
    {
        t0 = w.x / v.x;
        t1 = w2.x / v.x;
    }
    else
    {
        t0 = w.y / v.y;
        t1 = w2.y / v.y;
    }

    if(t0 > t1)
    {
        float t = t0; t0 = t1; t0 = t;
    }

    if(t0 >= 1 || t1 <= 0)
    {
        return false;
    }

    t0 = max(t0, 0);
    t1 = min(t1, 1);
    if(t0 == t1)
    {
        if(res)
        {
            res->p0 = glms_vec2_add(lb.a, glms_vec2_scale(v, t0));
            res->t0 = t0;
        }
        return true;
    }

    if(res)
    {
        res->p0 = glms_vec2_add(lb.a, glms_vec2_scale(v, t0));
        res->p1 = glms_vec2_add(lb.a, glms_vec2_scale(v, t1));
        res->t0 = t0;
        res->t1 = t1;
    }

    return true;
}

bool LineIntersection(struct line_t la, struct line_t lb, struct intersection_res_t *res)
{
    vec2s u = glms_vec2_sub(la.b, la.a);
    vec2s v = glms_vec2_sub(lb.b, lb.a);
    vec2s w = glms_vec2_sub(la.a, lb.a);
    float D = glms_vec2_cross(u, v);

    if(fabs(D) < SMALL_NUM)
    {
        return false;
    }

    float sI = glms_vec2_cross(v, w) / D;
    if (sI <= 0 || sI >= 1)
        return false;

    float tI = glms_vec2_cross(u, w) / D;
    if(tI <= 0 || tI >= 1)
        return false;

    if(res)
    {
        res->p0 = glms_vec2_add(la.a, glms_vec2_scale(u, sI));
        res->t0 = sI;
    }

    return true;
}

enum orientation_t LineLoopOrientation(size_t numVertices, vec2s vertices[static numVertices])
{
    float res = 0;
    for(size_t i = 0; i < numVertices; ++i)
    {
        vec2s a = vertices[i];
        vec2s b = vertices[(i+1)%numVertices];

        res += (b.x - a.x) * (b.y + a.y);
    }
    return res >= 0 ? CCW_ORIENT : CW_ORIENT;
}
