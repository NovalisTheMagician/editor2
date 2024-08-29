#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <cglm/struct.h>

#include "map.h"

#define PI 3.14159265359
#define PI2 (PI * 2.0)
#define PIHALF (PI / 2.0)

#define SMALL_NUM 0.00000001

#define rad2deg(x) ({__typeof__(x) _x = (x); _x * 180.0 / PI;})
#define deg2rad(x) ({__typeof__(x) _x = (x); _x * PI / 180.0;})

#define between(p, a, b) ({ __typeof__(p) p_ = (p); __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); (p_ >= a_ && p_ <= b_) || (p_ <= a_ && p_ >= b_); })
#define sign(x) ({ __typeof__(x) x_ = (x); (x_ > 0) - (x_ < 0); })

typedef float angle_t;

struct line_t
{
    vec2s a, b;
};

struct intersection_res_t
{
    float t0, t1;
    vec2s p0, p1;
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

static inline bool LineEq(struct line_t a, struct line_t b)
{
    return (glms_vec2_eqv(a.a, b.a) && glms_vec2_eqv(a.b, b.b)) || (glms_vec2_eqv(a.a, b.b) && glms_vec2_eqv(a.b, b.a));
}

bool PointInSector(struct MapSector *sector, vec2s point);
bool PointInPolygon(size_t numVertices, vec2s vertices[static numVertices], vec2s point);
float MinDistToLine(vec2s a, vec2s b, vec2s point);

bool LineIsCollinear(struct line_t a, struct line_t b);
bool LineIsParallel(struct line_t a, struct line_t b);

// this assumes that both lines are collinear
vec2s LineGetCommonPoint(struct line_t major, struct line_t support);
float LineGetPointFactor(struct line_t line, vec2s point);

bool LineOverlap(struct line_t a, struct line_t b, struct intersection_res_t *res);
bool LineIntersection(struct line_t a, struct line_t b, struct intersection_res_t *res);
enum orientation_t LineLoopOrientation(size_t numVertices, vec2s vertices[static numVertices]);

struct BoundingBox BoundingBoxFromVertices(size_t numVertices, vec2s vertices[static numVertices]);
bool BoundingBoxIntersect(struct BoundingBox a, struct BoundingBox b);

int SideOfMapLine(struct MapLine *line, vec2s point);
int SideOfLine(vec2s a, vec2s b, vec2s point);

angle_t NormalizeAngle(angle_t angle);
angle_t AngleDifference(angle_t a, angle_t b);
angle_t AngleLine(struct MapLine *line);
angle_t AngleOfLines(struct line_t a, struct line_t b);
angle_t AngleOfMapLines(struct MapLine *a, struct MapLine *b);
angle_t AngleOf(vec2s a, vec2s b, vec2s c);
