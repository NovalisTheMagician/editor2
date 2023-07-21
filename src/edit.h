#pragma once

#include "common.h"

#include "map.h"
#include "editor.h"

void EditCopy(struct EdState *state, struct Map *map);
void EditPaste(struct EdState *state, struct Map *map);
void EditCut(struct EdState *state, struct Map *map);
