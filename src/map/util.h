#pragma once

#include <triangulate.h>
#include <stdbool.h>

#include "../map.h"
#include "../geometry.h"
#include "../vertex_types.h"
#include "../vecmath.h"
#include "arena.h"

typedef struct SplitResult
{
    MapLine *left, *right, *middle;
} SplitResult;

struct Polygon* MakePolygon(Arena *arena, size_t numVertices, FVec2 vertices[static numVertices]);

SplitResult SplitMapLine(Map *map, MapLine *line, MapVertex *vertex);
SplitResult SplitMapLine2(Map *map, MapLine *line, MapVertex *vertexA, MapVertex *vertexB);
polygon_t* PolygonFromMapLines(size_t numLines, MapLine *lines[static numLines]);
polygon_t* PolygonFromMapLinesArena(Arena *arena, size_t numLines, MapLine *lines[static numLines]);
polygon_t* PolygonFromVertices(size_t numVertices, EditorVertexType vertices[static numVertices]);
polygon_t* PolygonFromVectors(size_t numVectors, FVec2 vectors[static numVectors]);
bool IsLineFront(MapVertex *v1, MapLine *line);
MapLine* GetMapLine(Map *map, line_t line);
void GetLoopStart(MapLine *l0, MapLine *l1, MapVertex **outVertex, MapVertex **outNext);

