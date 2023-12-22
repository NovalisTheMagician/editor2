#include "geometry.h"

#include <assert.h>
#include <tgmath.h>
#include <limits.h>

#include <cglm/ivec2.h>

bool PointInSector(struct MapSector sector[static 1], ivec2s point)
{
    return PointInPolygon(sector->numOuterLines, sector->vertices, point);
}

bool PointInPolygon(size_t numVertices, ivec2s vertices[static numVertices], ivec2s point)
{
    bool inside = false;
    for(size_t i = 0; i < numVertices; ++i)
    {
        ivec2s A = vertices[i];
        ivec2s B = vertices[(i+1) % numVertices];

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
    if(inside)
    {
        return true;
    }
    return false;
}

float MinDistToLine(ivec2s a, ivec2s b, ivec2s point)
{
    float l2 = dist2(a, b);
    if(l2 == 0) return dist2(point, a);
    float t = ((point.x - a.x) * (b.x - a.x) + (point.y - a.y) * (b.y - a.y)) / l2;
    t = max(0, min(1, t));
    ivec2s tmp = { .x = a.x + t * (b.x - a.x), .y = a.y + t * (b.y - a.y) };
    return sqrt(dist2(point, tmp));
}

int SideOfMapLine(struct MapLine line[static 1], ivec2s point)
{
    return SideOfLine(line->a->pos, line->b->pos, point);
}

int SideOfLine(ivec2s a, ivec2s b, ivec2s point)
{
    return (point.y - a.y) * (b.x - a.x) - (point.x - a.x) * (b.y - a.y);
}

struct BoundingBox BoundingBoxFromVertices(size_t numVertices, ivec2s vertices[static numVertices])
{
    ivec2s min = { .x = INT32_MAX, .y = INT32_MAX }, max = { .x = INT32_MIN, .y = INT32_MIN };
    for(size_t i = 0; i < numVertices; ++i)
    {
        ivec2s vert = vertices[i];
        glm_ivec2_maxv(vert.raw, max.raw, max.raw);
        glm_ivec2_minv(vert.raw, min.raw, min.raw);
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

angle_t AngleOfLines(struct MapLine a[static 1], struct MapLine b[static 1])
{
    struct MapVertex *aa = a->a;
    //struct MapVertex *ab = a->b;
    struct MapVertex *ba = b->a;
    struct MapVertex *bb = b->b;
    
    struct MapVertex *common = aa == ba ? ba : aa == bb ? bb : NULL;
    assert(common);

    ivec2s va = aa == common ? ba->pos : aa->pos, vb = ba == common ? bb->pos : ba->pos, vc = common->pos;
    return AngleOf(va, vc, vb);
}

angle_t AngleOf(ivec2s a, ivec2s b, ivec2s c)
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

static bool inBetween(int a)
{
    return false;
}

bool LineIntersection(struct line_t a, struct line_t b, ivec2s *ia, ivec2s *ib)
{

    return false;
}
