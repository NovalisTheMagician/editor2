#pragma once

#include "common.h"

#include "editor.h"
#include "map.h"
#include "editor.h"

void EditCopy(struct EdState *state, struct Map *map);
void EditPaste(struct EdState *state, struct Map *map);
void EditCut(struct EdState *state, struct Map *map);

void EditAddVertex(struct EdState *state, struct Map *map, struct Vertex pos);
void EditRemoveVertex(struct EdState *state, struct Map *map, size_t index);
bool EditGetVertex(struct EdState *state, struct Map *map, struct Vertex pos, size_t *ind);

void EditAddLine(struct EdState *state, struct Map *map, size_t v0, size_t v1);
void EditRemoveLine(struct EdState *state, struct Map *map, size_t index);
