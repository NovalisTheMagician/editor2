#pragma once

#include "../editor.h"
#include "../map.h"

MapSector* MakeMapSector(EdState *state, MapLine *startLine, bool front, SectorData data);
bool InsertLinesIntoMap(EdState *state, size_t numVerts, vec2s vertices[static numVerts], bool isLoop);
