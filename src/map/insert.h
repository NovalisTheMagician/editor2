#pragma once

#include "../map.h"
#include "../vecmath.h"

MapSector* MakeMapSector(Map *map, MapLine *startLine, bool reversed, SectorData data);
bool InsertLinesIntoMap(Map *map, size_t numVerts, FVec2 vertices[static numVerts], bool isLoop);
