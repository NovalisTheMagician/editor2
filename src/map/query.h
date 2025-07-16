#pragma once

#include "../map.h"

MapVertex* GetVertex(Map *map, size_t idx);
MapLine* GetLine(Map *map, size_t idx);
MapSector* GetSector(Map *map, size_t idx);
MapSector* FindEquivalentSector(Map *map, size_t numLines, MapLine *lines[static numLines]);

int angleSortOuter(const void *a, const void *b);
int angleSortInner(const void *a, const void *b);

size_t FindLineLoop(MapLine *startLine, MapLine **loop, size_t maxLoopLength, int(*cmpFunc)(const void*, const void*));
#define FindOuterLineLoop(startLine, loop, maxLoopLength) FindLineLoop(startLine, loop, maxLoopLength, angleSortOuter)
#define FindInnerLineLoop(startLine, loop, maxLoopLength) FindLineLoop(startLine, loop, maxLoopLength, angleSortInner)
