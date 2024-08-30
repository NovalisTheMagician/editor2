#pragma once

#include <triangulate.h>
#include <stdbool.h>

#include "../editor.h"
#include "../map.h"
#include "../geometry.h"

typedef struct SplitResult
{
    MapLine *left, *right, *middle;
} SplitResult;

SplitResult SplitMapLine(EdState *state, MapLine *line, MapVertex *vertex);
SplitResult SplitMapLine2(EdState *state, MapLine *line, MapVertex *vertexA, MapVertex *vertexB);
struct Polygon* PolygonFromMapLines(size_t numLines, MapLine *lines[static numLines], bool lineFronts[static numLines]);
bool IsLineFront(MapVertex *v1, MapLine *line);
MapLine* GetMapLine(Map *map, line_t line);
