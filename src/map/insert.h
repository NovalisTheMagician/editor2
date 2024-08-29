#pragma once

#include "../editor.h"
#include "../map.h"

struct MapSector* MakeMapSector(struct EdState *state, struct MapLine *startLine, bool front, struct SectorData data);
bool InsertLinesIntoMap(struct EdState *state, size_t numVerts, vec2s vertices[static numVerts], bool isLoop);
