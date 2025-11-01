#pragma once

#include "../map.h"
#include "../vecmath.h"

MapSector* MakeMapSector(Map *map, MapLine *startLine, SectorData data);
bool InsertLinesIntoMap(Map *map, size_t numVerts, Vec2 vertices[static numVerts], bool isLoop);
