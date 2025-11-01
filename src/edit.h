#pragma once

#include "editor.h"
#include "vecmath.h"

void ScreenToEditorSpace(const EdState *state, float *x, float *y);
void EditorToScreenSpace(const EdState *state, float *x, float *y);
void ScreenToEditorSpaceGrid(const EdState *state, int gridsize, float *x, float *y);

void EditCopy(EdState *state);
void EditPaste(EdState *state);
void EditCut(EdState *state);

MapVertex* EditAddVertex(Map *map, Vec2 pos);
void EditRemoveVertices(Map *map, size_t num, MapVertex *vertices[static num]);
MapVertex* EditGetVertex(Map *map, Vec2 pos);
MapVertex* EditGetClosestVertex(Map *map, Vec2 pos, float maxDist);

MapLine* EditAddLine(Map *map, MapVertex *v0, MapVertex *v1, LineData data);
void EditRemoveLines(Map *map, size_t num, MapLine *lines[static num]);
// MapLine* EditGetLine(Map *map, Vertex pos);
MapLine* EditGetClosestLine(Map *map, Vec2 pos, float maxDist);

MapSector* EditAddSector(Map *map, size_t numLines, MapLine *lines[static numLines], size_t numInnerLines, size_t numInnerLinesNum[static numInnerLines], MapLine ***innerLines, SectorData data);
void EditRemoveSectors(Map *map, size_t num, MapSector *sectors[static num]);
MapSector* EditGetSector(Map *map, Vec2 pos);

bool EditApplyLines(EdState *state, size_t num, Vec2 points[static num]);
bool EditApplySector(EdState *state, size_t num, Vec2 points[static num]);
