#pragma once

#include "../map.h"

MapVertex* GetVertex(Map *map, size_t idx);
MapLine* GetLine(Map *map, size_t idx);
MapSector* GetSector(Map *map, size_t idx);

int angleSortOuter(const void *a, const void *b);
int angleSortInner(const void *a, const void *b);

size_t FindLineLoop(MapLine *startLine, bool front, MapLine **loop, bool *loopFront, size_t maxLoopLength, int(*cmpFunc)(const void*, const void*));
#define FindOuterLineLoop(startLine, front, loop, loopFront, maxLoopLength) FindLineLoop(startLine, front, loop, loopFront, maxLoopLength, angleSortOuter)
#define FindInnerLineLoop(startLine, front, loop, loopFront, maxLoopLength) FindLineLoop(startLine, front, loop, loopFront, maxLoopLength, angleSortInner)
