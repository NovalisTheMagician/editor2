#pragma once

#include <triangulate.h>
#include <stdbool.h>

#include "../map.h"
#include "../geometry.h"
#include "../vertex_types.h"

typedef struct SplitResult
{
    MapLine *left, *right, *middle;
} SplitResult;

SplitResult SplitMapLine(Map *map, MapLine *line, MapVertex *vertex);
SplitResult SplitMapLine2(Map *map, MapLine *line, MapVertex *vertexA, MapVertex *vertexB);
struct Polygon* PolygonFromMapLines(Map *map, size_t numLines, MapLine *lines[static numLines]);
struct Polygon* PolygonFromVertices(size_t numVertices, EditorVertexType vertices[static numVertices]);
struct Polygon* PolygonFromVectors(size_t numVectors, vec2s vectors[static numVectors]);
bool IsLineFront(MapVertex *v1, MapLine *line);
MapLine* GetMapLine(Map *map, line_t line);
