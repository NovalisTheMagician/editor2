#pragma once

#include "../map.h"

MapVertex* GetVertex(Map *map, size_t idx);
MapLine* GetLine(Map *map, size_t idx);
MapSector* GetSector(Map *map, size_t idx);
MapSector* FindEquivalentSector(Map *map, size_t numLines, MapLine *lines[static numLines]);

MapVertex* FindClosestVertex(const Map *map, FVec2 position, fixed_t radius);

int angleSortOuter(const void *a, const void *b);
int angleSortInner(const void *a, const void *b);

size_t FindLineLoop(MapLine *startLine, MapLine **loop, size_t maxLoopLength, bool reversed, bool(*linePredicateFunc)(const MapLine*, const MapLine*), int(*cmpFunc)(const void*, const void*));
#define FindOuterLineLoop(startLine, loop, maxLoopLength, reversed) FindLineLoop(startLine, loop, maxLoopLength, reversed, NULL, angleSortOuter)
#define FindInnerLineLoop(startLine, loop, maxLoopLength, reversed) FindLineLoop(startLine, loop, maxLoopLength, reversed, NULL, angleSortInner)
#define FindOuterLineLoopPredicate(startLine, loop, maxLoopLength, reversed, predicate) FindLineLoop(startLine, loop, maxLoopLength, reversed, predicate, angleSortOuter)
