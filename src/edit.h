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

ssize_t EditAddVertex(struct EdState *state, struct Vertex pos);
void EditRemoveVertex(struct EdState *state, size_t index);
bool EditGetVertex(struct EdState *state, struct Vertex pos, size_t *ind);

ssize_t EditAddLine(struct EdState *state, size_t v0, size_t v1);
void EditRemoveLine(struct EdState *state, size_t index);
bool EditGetLine(struct EdState *state, struct Vertex pos, size_t *ind);

ssize_t EditAddSector(struct EdState *state, size_t *lineIndices, size_t numLines);
void EditRemoveSector(struct EdState *state, size_t index);
bool EditGetSector(struct EdState *state, struct Vertex pos, size_t *ind);

ssize_t EditApplyLines(struct EdState *state, struct Vertex *points, size_t num);
ssize_t EditApplySector(struct EdState *state, struct Vertex *points, size_t num);
