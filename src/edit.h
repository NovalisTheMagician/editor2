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

void EditAddVertex(struct EdState *state, struct Vertex pos);
void EditRemoveVertex(struct EdState *state, size_t index);
bool EditGetVertex(struct EdState *state, struct Vertex pos, size_t *ind);

void EditAddLine(struct EdState *state, size_t v0, size_t v1);
void EditRemoveLine(struct EdState *state, size_t index);
