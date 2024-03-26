#pragma once

#include "../editor.h"
#include "../map.h"

struct MapSector* MakeMapSector(struct EdState state[static 1], struct MapLine startLine[static 1], bool front, struct SectorData data);
bool InsertLinesIntoMap(struct EdState state[static 1], size_t numVerts, vec2s vertices[static numVerts], bool isLoop);
