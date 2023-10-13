#pragma once

#include "common.h"

#include "editor.h"
#include "map.h"
#include "editor.h"

void ScreenToEditorSpace(const struct EdState *state, int32_t *x, int32_t *y);
void EditorToScreenSpace(const struct EdState *state, int32_t *x, int32_t *y);
void ScreenToEditorSpaceGrid(const struct EdState *state, int32_t *x, int32_t *y);

void ScreenToEditorSpacef(const struct EdState *state, float *x, float *y);

void EditCopy(struct EdState *state);
void EditPaste(struct EdState *state);
void EditCut(struct EdState *state);

struct MapVertex* EditAddVertex(struct EdState *state, struct Vertex pos);
void EditRemoveVertex(struct EdState *state, struct MapVertex *vertex);
struct MapVertex* EditGetVertex(struct EdState *state, struct Vertex pos);
struct MapVertex* EditGetClosestVertex(struct EdState *state, struct Vertex pos, float maxDist);

struct MapLine* EditAddLine(struct EdState *state, struct MapVertex *v0, struct MapVertex *v1);
void EditRemoveLine(struct EdState *state, struct MapLine *index);
// struct MapLine* EditGetLine(struct EdState *state, struct Vertex pos);
struct MapLine* EditGetClosestLine(struct EdState *state, struct Vertex pos, float maxDist);

// struct MapSector* EditAddSector(struct EdState *state, size_t *lineIndices, size_t numLines);
void EditRemoveSector(struct EdState *state, struct MapSector *sector);
struct MapSector* EditGetSector(struct EdState *state, struct Vertex pos);

struct MapLine* EditApplyLines(struct EdState *state, struct Vertex *points, size_t num);
struct MapSector* EditApplySector(struct EdState *state, struct Vertex *points, size_t num);
