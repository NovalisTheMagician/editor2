#pragma once

#include "editor.h"
#include "vecmath.h"

void ScreenToEditorSpace(const EdState *state, real_t *x, real_t *y);
void EditorToScreenSpace(const EdState *state, real_t *x, real_t *y);
void ScreenToEditorSpaceGrid(const EdState *state, int gridsize, real_t *x, real_t *y);

void EditCopy(EdState *state);
void EditPaste(EdState *state);
void EditCut(EdState *state);

MapVertex* EditAddVertex(Map *map, FVec2 pos);
void EditRemoveVertices(Map *map, size_t num, MapVertex *vertices[static num]);
MapVertex* EditGetVertex(Map *map, FVec2 pos);
MapVertex* EditGetClosestVertex(Map *map, FVec2 pos, fixed_t maxDist);

MapLine* EditAddLine(Map *map, MapVertex *v0, MapVertex *v1, LineData data);
void EditRemoveLines(Map *map, size_t num, MapLine *lines[static num]);
// MapLine* EditGetLine(Map *map, Vertex pos);
MapLine* EditGetClosestLine(Map *map, FVec2 pos, fixed_t maxDist);

MapSector* EditAddSector(Map *map, size_t numLines, MapLine *lines[static numLines], size_t numInnerLines, size_t numInnerLinesNum[static numInnerLines], MapLine ***innerLines, bool innerOtherSide[static numInnerLines], SectorData data);
void EditRemoveSectors(Map *map, size_t num, MapSector *sectors[static num]);
MapSector* EditGetSector(Map *map, FVec2 pos);

bool EditApplyLines(EdState *state, size_t num, FVec2 points[static num]);
bool EditApplySector(EdState *state, size_t num, FVec2 points[static num]);
