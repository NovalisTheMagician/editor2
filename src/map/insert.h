#pragma once

#include "../map.h"

MapSector* MakeMapSector(Map *map, MapLine *startLine, SectorData data);
bool InsertLinesIntoMap(Map *map, size_t numVerts, vec2s vertices[static numVerts], bool isLoop);
