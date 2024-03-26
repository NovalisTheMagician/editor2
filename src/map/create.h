#pragma once

#include <stdbool.h>

#include "../map.h"

struct CreateResult
{
    void *mapElement;
    bool created;
};

struct CreateResult CreateVertex(struct Map map[static 1], vec2s pos);
struct CreateResult CreateLine(struct Map map[static 1], struct MapVertex v0[static 1], struct MapVertex v1[static 1], struct LineData data);
struct CreateResult CreateSector(struct Map map[static 1], size_t numLines, struct MapLine *lines[static numLines], bool lineFronts[static numLines], struct SectorData data);
