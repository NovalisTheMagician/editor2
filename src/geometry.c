#include "geometry.h"

#include <assert.h>
#include <tgmath.h>
#include <limits.h>

#include "map/query.h"
#include "map/util.h"
#include "utils.h"

bool PointInSector(MapSector *sector, vec2s point)
{
    return PointInPolygonVector(sector->numOuterLines, sector->edData.vertices, point);
}

bool PointInSector2(Map *map, MapSector *sector, vec2s point)
{
    bool inside = PointInSector(sector, point);
    for(size_t i = 0; i < sector->numInnerLines; ++i)
    {
        size_t count = sector->numInnerLinesNum[i];
        MapLine *lines[count];
        for(size_t j = 0; j < count; ++j)
            lines[j] = GetLine(map, sector->innerLines[i][j]);
        struct Polygon *poly = PolygonFromMapLines(map, count, lines);
        inside &= !PointInPolygon(poly, point);
        free(poly);
    }
    return inside;
}

bool PointInPolygon(struct Polygon *polygon, vec2s point)
{
    return PointInPolygonVector(polygon->length, (vec2s*)polygon->vertices, point);
}

bool PointInPolygonVector(size_t numVertices, vec2s vertices[static numVertices], vec2s point)
{
    bool inside = false;
    for(size_t i = 0; i < numVertices; ++i)
    {
        vec2s A = vertices[i];
        vec2s B = vertices[(i+1) % numVertices];

        if ((eqv(point.x, A.x) && eqv(point.y, A.y)) || (eqv(point.x, B.x) && eqv(point.y, B.y))) break;
        if (eqv(A.y, B.y) && eqv(point.y, A.y) && between(point.x, A.x, B.x)) break;

        if (between(point.y, A.y, B.y))
        { // if P inside the vertical range
            // filter out "ray pass vertex" problem by treating the line a little lower
            if ((eqv(point.y, A.y) && B.y >= A.y) || (eqv(point.y, B.y) && A.y >= B.y)) continue;
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
    if(eqv(l2, 0)) return glms_vec2_distance2(point, a);
    float t = ((point.x - a.x) * (b.x - a.x) + (point.y - a.y) * (b.y - a.y)) / l2;
    t = max(0, min(1, t));
    vec2s tmp = { .x = a.x + t * (b.x - a.x), .y = a.y + t * (b.y - a.y) };
    return sqrt(glms_vec2_distance2(point, tmp));
}

int SideOfMapLine(Map *map, MapLine *line, vec2s point)
{
    MapVertex *a = GetVertex(map, line->a);
    MapVertex *b = GetVertex(map, line->a);
    return SideOfLine(a->pos, b->pos, point);
}

int SideOfLine(vec2s a, vec2s b, vec2s point)
{
    return (point.y - a.y) * (b.x - a.x) - (point.x - a.x) * (b.y - a.y);
}

BoundingBox BoundingBoxFromVertices(size_t numVertices, vec2s vertices[static numVertices])
{
    vec2s min = { .x = FLT_MAX, .y = FLT_MAX }, max = { .x = FLT_MIN, .y = FLT_MIN };
    for(size_t i = 0; i < numVertices; ++i)
    {
        vec2s vert = vertices[i];
        max = glms_vec2_maxv(vert, max);
        min = glms_vec2_minv(vert, min);
    }
    return (BoundingBox){ .min = min, .max = max };
}

BoundingBox BoundingBoxFromMapLines(Map *map, size_t numLines, MapLine *lines[static numLines])
{
    vec2s min = { .x = FLT_MAX, .y = FLT_MAX }, max = { .x = FLT_MIN, .y = FLT_MIN };
    for(size_t i = 0; i < numLines; ++i)
    {
        MapVertex *a = GetVertex(map, lines[i]->a);
        MapVertex *b = GetVertex(map, lines[i]->b);
        vec2s vert = a->pos;
        max = glms_vec2_maxv(vert, max);
        min = glms_vec2_minv(vert, min);
        vert = b->pos;
        max = glms_vec2_maxv(vert, max);
        min = glms_vec2_minv(vert, min);
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

angle_t AngleOfMapLines(Map *map, MapLine *a, MapLine *b)
{
    MapVertex *va_ = GetVertex(map, a->a);
    MapVertex *vb_ = GetVertex(map, a->b);

    MapVertex *aa = va_;
    //MapVertex *ab = a->b;
    MapVertex *ba = va_;
    MapVertex *bb = vb_;

    MapVertex *common = aa == ba ? ba : aa == bb ? bb : NULL;
    assert(common);

    vec2s va = aa == common ? ba->pos : aa->pos, vb = ba == common ? bb->pos : ba->pos, vc = common->pos;
    return AngleOf(va, vc, vb);
}

angle_t AngleOfLines(line_t a, line_t b)
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

bool LineIsCollinear(line_t la, line_t lb)
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

bool LineIsParallel(line_t a, line_t b)
{
    vec2s u = glms_vec2_sub(a.b, a.a);
    vec2s v = glms_vec2_sub(b.b, b.a);
    float D = glms_vec2_cross(u, v);
    return fabs(D) < SMALL_NUM;
}

vec2s LineGetCommonPoint(line_t major, line_t support)
{
    if(glms_vec2_eqv_eps(major.a, support.a)) return major.a;
    if(glms_vec2_eqv_eps(major.b, support.a)) return major.b;
    if(glms_vec2_eqv_eps(major.a, support.b)) return major.a;
    //if(glms_vec2_eqv_eps(major.b, support.b)) return major.b;
    return major.b;
}

float LineGetPointFactor(line_t line, vec2s point)
{
    //vec2s u = glms_vec2_sub(line.b, line.a);
    float len = glms_vec2_distance(line.b, line.a);
    return glms_vec2_distance(line.a, point) / len;
}

static bool isBetween(vec2s p, line_t l)
{
    //return eqv(glms_vec2_distance(l.a, p) + glms_vec2_distance(p, l.b), glms_vec2_distance(l.a, l.b));
    float crossproduct = (p.y - l.a.y) * (l.b.x - l.a.x) - (p.x - l.a.x) * (l.b.y - l.a.y);

    // compare versus epsilon for floating point values, or != 0 if using integers
    if(!eqv(crossproduct, 0))
        return false;

    float dotproduct = (p.x - l.a.x) * (l.b.x - l.a.x) + (p.y - l.a.y)*(l.b.y - l.a.y);
    if(dotproduct < 0)
        return false;

    float squaredlengthba = (l.b.x - l.a.x)*(l.b.x - l.a.x) + (l.b.y - l.a.y)*(l.b.y - l.a.y);
    if(dotproduct > squaredlengthba)
        return false;

    return true;
}

classify_res_t ClassifyLines(line_t a, line_t b)
{
    if(LineEq(a, b))
        return (classify_res_t){ .type = SAME_LINES };

    if(LineShare(a, b))
    {
        if(LineIsParallel(a, b))
        {
            vec2s commonPoint = LineGetCommonPoint(a, b);
            line_t major = { .a = commonPoint, .b = glms_vec2_eqv_eps(commonPoint, a.a) ? a.b : a.a };
            line_t minor = { .a = commonPoint, .b = glms_vec2_eqv_eps(commonPoint, b.a) ? b.b : b.a };

            vec2s u = glms_vec2_sub(major.b, major.a);
            vec2s v = glms_vec2_sub(minor.b, minor.a);
            float dot = glms_vec2_dot(u, v);
            if(dot > 0) // they do overlap
            {
                float t = LineGetPointFactor(major, minor.b);
                if(t > 1)
                    return (classify_res_t){ .type = SIMPLE_OVERLAP_OUTER, .overlap = { .line = { .a = major.b, .b = minor.b } } };
                else
                    return (classify_res_t){ .type = SIMPLE_OVERLAP_INNER, .overlap = { .splitPoint = minor.b } };
            }
            else
                return (classify_res_t){ .type = NO_RELATION };
        }
        else
            return (classify_res_t){ .type = NO_RELATION };
    }

    if(isBetween(b.a, a))
    {
        vec2s splitPoint = glms_vec2_add(a.a, glms_vec2_scale(glms_vec2_sub(a.b, a.a), (glms_vec2_distance(a.a, b.a) / glms_vec2_distance(a.a, a.b))));
        return (classify_res_t){ .type = TOUCH, .touch = { .splitPoint = splitPoint } };
    }

    if(isBetween(b.b, a))
    {
        vec2s splitPoint = glms_vec2_add(a.a, glms_vec2_scale(glms_vec2_sub(a.b, a.a), (glms_vec2_distance(a.a, b.b) / glms_vec2_distance(a.a, a.b))));
        return (classify_res_t){ .type = TOUCH, .touch = { .splitPoint = splitPoint } };
    }

    if(isBetween(a.a, b))
    {
        vec2s splitPoint = glms_vec2_add(b.a, glms_vec2_scale(glms_vec2_sub(b.b, b.a), (glms_vec2_distance(b.a, a.a) / glms_vec2_distance(b.a, b.b))));
        return (classify_res_t){ .type = TOUCH_REVERSE, .touch = { .splitPoint = splitPoint } };
    }

    if(isBetween(a.b, b))
    {
        vec2s splitPoint = glms_vec2_add(b.a, glms_vec2_scale(glms_vec2_sub(b.b, b.a), (glms_vec2_distance(b.a, a.b) / glms_vec2_distance(b.a, b.b))));
        return (classify_res_t){ .type = TOUCH_REVERSE, .touch = { .splitPoint = splitPoint } };
    }

    intersection_res_t intersection = { 0 };
    if(LineIntersection(a, b, &intersection))
    {
        line_t line1 = { .a = b.a, .b = intersection.p0 };
        line_t line2 = { .a = intersection.p0, .b = b.b };
        return (classify_res_t){ .type = INTERSECTION, .intersection = { .splitPoint = intersection.p0, .splitLine1 = line1, .splitLine2 = line2 } };
    }

    float aOrient = (a.b.x - a.a.x) * (a.b.y + a.a.y);
    float bOrient = (b.b.x - b.a.x) * (b.b.y + b.a.y);
    line_t lineCorrected = b;
    if(signbit(aOrient) != signbit(bOrient))
    {
        lineCorrected.a = b.b;
        lineCorrected.b = b.a;
    }
    if(LineOverlap(a, lineCorrected, &intersection))
    {
        if(eqv(intersection.t0, 0) && eqv(intersection.t1, 1))
        {
            return (classify_res_t){ .type = INNER_CONTAINMENT, .innerContainment = { .split1 = intersection.p0, .split2 = intersection.p1 } };
        }
        else if(eqv(intersection.t0, 0))
        {
            return (classify_res_t){ .type = OVERLAP, .overlap = { .splitPoint = intersection.p0, .line = { a.b, lineCorrected.b } } };
        }
        else if(eqv(intersection.t1, 1))
        {
            return (classify_res_t){ .type = OVERLAP, .overlap = { .splitPoint = intersection.p1, .line = { a.a, lineCorrected.a } } };
        }
        else
        {
            line_t line1 = { a.b, lineCorrected.b };
            line_t line2 = { lineCorrected.a, a.a };
            return (classify_res_t){ .type = OUTER_CONTAINMENT, .outerContainment = { .line1 = line1, .line2 = line2 } };
        }
    }

    return (classify_res_t){ .type = NO_RELATION };
}

bool LineOverlap(line_t la, line_t lb, intersection_res_t *res)
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
    if(!eqv(v.x, 0))
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
    if(eqv(t0, t1))
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

bool LineIntersection(line_t la, line_t lb, intersection_res_t *res)
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
