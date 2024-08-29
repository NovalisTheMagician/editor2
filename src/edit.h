#pragma once

#include "editor.h"

void ScreenToEditorSpace(const EdState *state, int32_t *x, int32_t *y);
void EditorToScreenSpace(const EdState *state, int32_t *x, int32_t *y);
void ScreenToEditorSpaceGrid(const EdState *state, int32_t *x, int32_t *y);

void ScreenToEditorSpacef(const EdState *state, float *x, float *y);

void EditCopy(EdState *state);
void EditPaste(EdState *state);
void EditCut(EdState *state);

MapVertex* EditAddVertex(EdState *state, vec2s pos);
void EditRemoveVertices(EdState *state, size_t num, MapVertex *vertices[static num]);
MapVertex* EditGetVertex(EdState *state, vec2s pos);
MapVertex* EditGetClosestVertex(EdState *state, vec2s pos, float maxDist);

MapLine* EditAddLine(EdState *state, MapVertex *v0, MapVertex *v1, LineData data);
void EditRemoveLines(EdState *state, size_t num, MapLine *lines[static num]);
// MapLine* EditGetLine(EdState *state, Vertex pos);
MapLine* EditGetClosestLine(EdState *state, vec2s pos, float maxDist);

MapSector* EditAddSector(EdState *state, size_t numLines, MapLine *lines[static numLines], bool lineFront[static numLines], SectorData data);
void EditRemoveSectors(EdState *state, size_t num, MapSector *sectors[static num]);
MapSector* EditGetSector(EdState *state, vec2s pos);

MapLine* EditApplyLines(EdState *state, size_t num, vec2s points[static num]);
MapSector* EditApplySector(EdState *state, size_t num, vec2s points[static num]);
