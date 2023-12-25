#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <cglm/struct.h>

#include "map.h"

#define PI 3.14159265359
#define PI2 (PI * 2.0)
#define PIHALF (PI / 2.0)

#define SMALL_NUM 0.00000001

#define deg2rad(x) ({__typeof__(x) _x = (x); _x * 180.0 / PI;})
#define rad2deg(x) ({__typeof__(x) _x = (x); _x * PI / 180.0;})

#define dot(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_.x * b_.x + a_.y * b_.y; })
#define perp(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_.x * b_.y - a_.y * b_.x; })
#define dist2(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); float dx = a_.x - b_.x; float dy = a_.y - b_.y; dx*dx + dy*dy; })
#define vec2eq(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); a_.x == b_.x && a_.y == b_.y; })
#define tovec(a) ((vec2s){ .x = (float)a.x, .y = (float)a.y })
#define toivec(a) ((ivec2s){ .x = (int32_t)roundf(a.x), .y = (int32_t)roundf(a.y) })

#define between(p, a, b) ({ __typeof__(p) p_ = (p); __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); (p_ >= a_ && p_ <= b_) || (p_ <= a_ && p_ >= b_); })
#define sign(x) ({ __typeof__(x) x_ = (x); (x_ > 0) - (x_ < 0); })

typedef float angle_t;

struct line_t
{
    ivec2s a, b;
};

struct intersection_res_t
{
    float t0, t1;
    ivec2s p0, p1;
};

enum intersection_type_t
{
    NO_INTERSECTION,
    INTERSECTION,
    OVERLAP
};

enum orientation_t
{
    CW_ORIENT,
    CCW_ORIENT
};

bool PointInSector(struct MapSector sector[static 1], ivec2s point);
bool PointInPolygon(size_t numVertices, ivec2s vertices[static numVertices], ivec2s point);
float MinDistToLine(ivec2s a, ivec2s b, ivec2s point);

enum intersection_type_t LineIntersection(struct line_t a, struct line_t b, struct intersection_res_t *res);
enum orientation_t LineLoopOrientation(size_t numVertices, ivec2s vertices[static numVertices]);

struct BoundingBox BoundingBoxFromVertices(size_t numVertices, ivec2s vertices[static numVertices]);
bool BoundingBoxIntersect(struct BoundingBox a, struct BoundingBox b);

int SideOfMapLine(struct MapLine line[static 1], ivec2s point);
int SideOfLine(ivec2s a, ivec2s b, ivec2s point);

angle_t NormalizeAngle(angle_t angle);
angle_t AngleDifference(angle_t a, angle_t b);
angle_t AngleLine(struct MapLine line[static 1]);
angle_t AngleOfLines(struct line_t a, struct line_t b);
angle_t AngleOfMapLines(struct MapLine a[static 1], struct MapLine b[static 1]);
angle_t AngleOf(ivec2s a, ivec2s b, ivec2s c);
