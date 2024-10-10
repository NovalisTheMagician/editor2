#pragma once

#include <stdint.h>

#include "map.h"

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

typedef enum intersection_type_t
{
    NO_INTERSECTION,
    INTERSECTION,
    OVERLAP
} intersection_type_t;

typedef enum orientation_t
{
    CW_ORIENT,
    CCW_ORIENT
} orientation_t;

static inline bool LineEq(line_t a, line_t b)
{
    return (glms_vec2_eqv(a.a, b.a) && glms_vec2_eqv(a.b, b.b)) || (glms_vec2_eqv(a.a, b.b) && glms_vec2_eqv(a.b, b.a));
}

bool PointInSector(MapSector *sector, vec2s point);
bool PointInPolygon(size_t numVertices, vec2s vertices[static numVertices], vec2s point);
float MinDistToLine(vec2s a, vec2s b, vec2s point);

bool LineIsCollinear(line_t a, line_t b);
bool LineIsParallel(line_t a, line_t b);

// this assumes that both lines are collinear
vec2s LineGetCommonPoint(line_t major, line_t support);
float LineGetPointFactor(line_t line, vec2s point);

bool LineOverlap(line_t a, line_t b, intersection_res_t *res);
bool LineIntersection(line_t a, line_t b, intersection_res_t *res);
enum orientation_t LineLoopOrientation(size_t numVertices, vec2s vertices[static numVertices]);

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
