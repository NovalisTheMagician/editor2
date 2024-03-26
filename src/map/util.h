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

struct SplitResult SplitMapLine(struct EdState state[static 1], struct MapLine line[static 1], struct MapVertex vertex[static 1]);
struct SplitResult SplitMapLine2(struct EdState state[static 1], struct MapLine line[static 1], struct MapVertex vertexA[static 1], struct MapVertex vertexB[static 1]);
struct Polygon* PolygonFromMapLines(size_t numLines, struct MapLine *lines[static numLines], bool lineFronts[static numLines]);
bool IsLineFront(struct MapVertex *v1, struct MapLine *line);
struct MapLine* GetMapLine(struct Map map[static 1], struct line_t line);
