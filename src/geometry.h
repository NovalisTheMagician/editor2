#pragma once

#include <stdint.h>

#include "map.h"
#include "triangulate.h"

#define PI 3.14159265359
#define PI2 (PI * 2.0)
#define PIHALF (PI / 2.0)

#define SMALL_NUM 0.00000001

#define rad2deg(x) ({typeof(x) _x = (x); _x * 180.0 / PI;})
#define deg2rad(x) ({typeof(x) _x = (x); _x * PI / 180.0;})

#define between(p, a, b) ({ typeof(p) p_ = (p); typeof(a) a_ = (a); typeof(b) b_ = (b); (p_ >= a_ && p_ <= b_) || (p_ <= a_ && p_ >= b_); })
#define sign(x) ({ typeof(x) x_ = (x); (x_ > 0) - (x_ < 0); })

typedef float angle_t;

typedef struct line_t
{
    vec2s a, b;
} line_t;

typedef struct intersection_res_t
{
    float t0, t1;
    vec2s p0, p1;
} intersection_res_t;

typedef struct classify_res_t
{
    enum
    {
        NO_RELATION,
        SAME_LINES,
        INTERSECTION,
        TOUCH,
        TOUCH_REVERSE,
        INNER_CONTAINMENT,
        OUTER_CONTAINMENT,
        SIMPLE_OVERLAP_INNER,
        SIMPLE_OVERLAP_OUTER,
        OVERLAP
    } type;
    union
    {
        struct
        {
            vec2s splitPoint;
            line_t splitLine1, splitLine2;
        } intersection;
        struct
        {
            vec2s split1, split2;
        } innerContainment;
        struct
        {
            line_t line1, line2;
        } outerContainment;
        struct
        {
            vec2s splitPoint;
            line_t line;
        } overlap;
        struct
        {
            vec2s splitPoint;
        } touch;
    };
} classify_res_t;

typedef enum orientation_t
{
    CW_ORIENT,
    CCW_ORIENT
} orientation_t;

static inline bool LineEq(line_t a, line_t b)
{
    return (glms_vec2_eqv_eps(a.a, b.a) && glms_vec2_eqv_eps(a.b, b.b)) || (glms_vec2_eqv_eps(a.a, b.b) && glms_vec2_eqv_eps(a.b, b.a));
}

static inline bool LineShare(line_t a, line_t b)
{
    return (glms_vec2_eqv_eps(a.a, b.a) || glms_vec2_eqv_eps(a.b, b.a) || glms_vec2_eqv_eps(a.b, b.b) || glms_vec2_eqv_eps(a.a, b.b));
}

bool PointInSector(MapSector *sector, vec2s point);
bool PointInSector2(MapSector *sector, vec2s point);
bool PointInPolygonVector(size_t numVertices, vec2s vertices[static numVertices], vec2s point);
bool PointInPolygon(struct Polygon *polygon, vec2s point);
float MinDistToLine(vec2s a, vec2s b, vec2s point);

bool LineIsCollinear(line_t a, line_t b);
bool LineIsParallel(line_t a, line_t b);

vec2s LineGetClosestPoint(line_t line, vec2s pos);

// this assumes that both lines are collinear
vec2s LineGetCommonPoint(line_t major, line_t support);
float LineGetPointFactor(line_t line, vec2s point);

classify_res_t ClassifyLines(line_t a, line_t b);
bool LineOverlap(line_t a, line_t b, intersection_res_t *res);
bool LineIntersection(line_t a, line_t b, intersection_res_t *res);
orientation_t LineLoopOrientation(size_t numVertices, vec2s vertices[static numVertices]);

BoundingBox BoundingBoxFromVertices(size_t numVertices, vec2s vertices[static numVertices]);
bool BoundingBoxIntersect(BoundingBox a, BoundingBox b);

int SideOfMapLine(MapLine *line, vec2s point);
int SideOfLine(vec2s a, vec2s b, vec2s point);

angle_t NormalizeAngle(angle_t angle);
angle_t AngleDifference(angle_t a, angle_t b);
angle_t AngleLine(MapLine *line);
angle_t AngleOfLines(line_t a, line_t b);
angle_t AngleOfMapLines(MapLine *a, MapLine *b);
angle_t AngleOf(vec2s a, vec2s b, vec2s c);
