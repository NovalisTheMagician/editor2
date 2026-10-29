#pragma once

#include <stdint.h>

#include "map.h"
#include "triangulate.h"
#include "vecmath.h"
#include "arena.h"

typedef real_t angle_t;

typedef struct polygon_t
{
    size_t length;
    Vec2 vertices[];
} polygon_t;

typedef struct line_t
{
    Vec2 a, b;
} line_t;

typedef struct intersection_res_t
{
    real_t u, v;
    Vec2 p0, p1;
    bool exact;
} intersection_res_t;

typedef enum orientation_t
{
    CW_ORIENT,
    CCW_ORIENT,
    DEGENERATE_ORIENT
} orientation_t;

static inline bool LineEq(line_t a, line_t b)
{
    //return (fvec2_eq(a.a, b.a) && fvec2_eq(a.b, b.b)) || (fvec2_eq(a.a, b.b) && fvec2_eq(a.b, b.a));
    return (vec2_eq(a.a, b.a) && vec2_eq(a.b, b.b)) || (vec2_eq(a.a, b.b) && vec2_eq(a.b, b.a));
}

static inline bool LineShare(line_t a, line_t b)
{
    //return (fvec2_eq(a.a, b.a) || fvec2_eq(a.b, b.a) || fvec2_eq(a.b, b.b) || fvec2_eq(a.a, b.b));
    return (vec2_eq(a.a, b.a) || vec2_eq(a.b, b.a) || vec2_eq(a.b, b.b) || vec2_eq(a.a, b.b));
}

bool PointInSector(MapSector *sector, Vec2 point);
bool PointInSector2(MapSector *sector, Vec2 point);
bool PointInPolygonVector(size_t numVertices, Vec2 vertices[static numVertices], Vec2 point);
bool PointInPolygon(polygon_t *polygon, Vec2 point);
real_t MinDistToLine(Vec2 a, Vec2 b, Vec2 point);

Vec2 LineGetClosestPoint(line_t line, Vec2 pos);
Vec2 LineGetClosestPointGrid(line_t line, Vec2 pos, int gridSize);

Vec2 Materialize(Vec2 origin, Vec2 dir, real_t t);
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
