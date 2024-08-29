#pragma once

#include <triangulate.h>
#include <stdbool.h>

#include "../editor.h"
#include "../map.h"
#include "../geometry.h"

struct SplitResult
{
    struct MapLine *left, *right, *middle;
};

struct SplitResult SplitMapLine(struct EdState *state, struct MapLine *line, struct MapVertex *vertex);
struct SplitResult SplitMapLine2(struct EdState *state, struct MapLine *line, struct MapVertex *vertexA, struct MapVertex *vertexB);
struct Polygon* PolygonFromMapLines(size_t numLines, struct MapLine *lines[static numLines], bool lineFronts[static numLines]);
bool IsLineFront(struct MapVertex *v1, struct MapLine *line);
struct MapLine* GetMapLine(struct Map *map, struct line_t line);
