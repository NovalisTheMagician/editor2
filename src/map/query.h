#pragma once

#include "../map.h"

MapVertex* GetVertex(const Map *map, size_t idx);
MapLine* GetLine(const Map *map, size_t idx);
MapSector* GetSector(const Map *map, size_t idx);
MapSector* FindEquvivalentSector(const Map *map, size_t numLines, MapLine *lines[static numLines]);

MapVertex* FindClosestVertex(const Map *map, vec2s position, float radius);

int angleSortOuter(const void *a, const void *b);
int angleSortInner(const void *a, const void *b);

size_t FindLineLoop(Map *map, MapLine *startLine, MapLine **loop, size_t maxLoopLength, int(*cmpFunc)(const void*, const void*));
#define FindOuterLineLoop(map, startLine, loop, maxLoopLength) FindLineLoop(map, startLine, loop, maxLoopLength, angleSortOuter)
#define FindInnerLineLoop(map, startLine, loop, maxLoopLength) FindLineLoop(map, startLine, loop, maxLoopLength, angleSortInner)
