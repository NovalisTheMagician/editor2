#pragma once

#include "common.h"

#include "editor.h"
#include "map.h"
#include "editor.h"

void ScreenToEditorSpace(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1]);
void EditorToScreenSpace(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1]);
void ScreenToEditorSpaceGrid(const struct EdState state[static 1], int32_t x[static 1], int32_t y[static 1]);

void ScreenToEditorSpacef(const struct EdState state[static 1], float x[static 1], float y[static 1]);

void EditCopy(struct EdState state[static 1]);
void EditPaste(struct EdState state[static 1]);
void EditCut(struct EdState state[static 1]);

struct MapVertex* EditAddVertex(struct EdState state[static 1], vec2s pos);
void EditRemoveVertices(struct EdState state[static 1], size_t num, struct MapVertex *vertices[static num]);
struct MapVertex* EditGetVertex(struct EdState state[static 1], vec2s pos);
struct MapVertex* EditGetClosestVertex(struct EdState state[static 1], vec2s pos, float maxDist);

struct MapLine* EditAddLine(struct EdState state[static 1], struct MapVertex v0[static 1], struct MapVertex v1[static 1]);
void EditRemoveLines(struct EdState state[static 1], size_t num, struct MapLine *lines[static num]);
// struct MapLine* EditGetLine(struct EdState *state, struct Vertex pos);
struct MapLine* EditGetClosestLine(struct EdState state[static 1], vec2s pos, float maxDist);

// struct MapSector* EditAddSector(struct EdState *state, size_t *lineIndices, size_t numLines);
void EditRemoveSectors(struct EdState state[static 1], size_t num, struct MapSector *sectors[static num]);
struct MapSector* EditGetSector(struct EdState state[static 1], vec2s pos);

struct MapLine* EditApplyLines(struct EdState state[static 1], size_t num, vec2s points[static num]);
struct MapSector* EditApplySector(struct EdState state[static 1], size_t num, vec2s points[static num]);
