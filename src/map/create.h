#pragma once

#include <stdbool.h>

#include "../map.h"

typedef struct CreateResult
{
    void *mapElement;
    bool created;
} CreateResult;

CreateResult CreateVertex(Map *map, vec2s pos);
CreateResult CreateLine(Map *map, MapVertex *v0, MapVertex *v1, LineData data);
CreateResult CreateSector(Map *map, size_t numLines, MapLine *lines[static numLines], bool lineFronts[static numLines], SectorData data);
