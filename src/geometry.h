#pragma once

#include <stdint.h>

#include "map.h"
#include "triangulate.h"
#include "vecmath.h"

#define PI 3.14159265359
#define PI2 (PI * 2.0)
#define PIHALF (PI / 2.0)

//#define SMALL_NUM 0.000001
#define SMALL_NUM 0.001

#define rad2deg(x) ({typeof(x) _x = (x); _x * 180.0 / PI;})
#define deg2rad(x) ({typeof(x) _x = (x); _x * PI / 180.0;})

#define between(p, a, b) ({ typeof(p) p_ = (p); typeof(a) a_ = (a); typeof(b) b_ = (b); (p_ >= a_ && p_ <= b_) || (p_ <= a_ && p_ >= b_); })
#define sign(x) ({ typeof(x) x_ = (x); (x_ > 0) - (x_ < 0); })

typedef float angle_t;

typedef struct line_t
{
    Vec2 a, b;
} line_t;

typedef struct intersection_res_t
{
    double u, v;
    Vec2 p0, p1;
} intersection_res_t;

typedef enum orientation_t
{
    CW_ORIENT,
    CCW_ORIENT
} orientation_t;

static inline bool LineEq(line_t a, line_t b)
{
    return (vec2_eqv(a.a, b.a) && vec2_eqv(a.b, b.b)) || (vec2_eqv(a.a, b.b) && vec2_eqv(a.b, b.a));
}

static inline bool LineShare(line_t a, line_t b)
{
    return (vec2_eqv(a.a, b.a) || vec2_eqv(a.b, b.a) || vec2_eqv(a.b, b.b) || vec2_eqv(a.a, b.b));
}

bool PointInSector(MapSector *sector, Vec2 point);
bool PointInSector2(MapSector *sector, Vec2 point);
bool PointInPolygonVector(size_t numVertices, Vec2 vertices[static numVertices], Vec2 point);
bool PointInPolygon(struct Polygon *polygon, Vec2 point);
real_t MinDistToLine(Vec2 a, Vec2 b, Vec2 point);

bool LineIsCollinear(line_t a, line_t b);
bool LineIsParallel(line_t a, line_t b);

Vec2 LineGetClosestPoint(line_t line, Vec2 pos);

// this assumes that both lines are collinear
Vec2 LineGetCommonPoint(line_t major, line_t support);
real_t LineGetPointFactor(line_t line, Vec2 point);

bool LineOverlap(line_t a, line_t b, intersection_res_t *res);
bool LineIntersection(line_t a, line_t b, intersection_res_t *res);
orientation_t LineLoopOrientation(size_t numVertices, Vec2 vertices[static numVertices]);

BoundingBox BoundingBoxFromVertices(size_t numVertices, Vec2 vertices[static numVertices]);
bool BoundingBoxIntersect(BoundingBox a, BoundingBox b);

int SideOfMapLine(MapLine *line, Vec2 point);
int SideOfLine(Vec2 a, Vec2 b, Vec2 point);

angle_t NormalizeAngle(angle_t angle);
angle_t AngleDifference(angle_t a, angle_t b);
angle_t AngleLine(MapLine *line);
angle_t AngleOfLines(line_t a, line_t b);
angle_t AngleOfMapLines(MapLine *a, MapLine *b);
angle_t AngleOf(Vec2 a, Vec2 b, Vec2 c);
