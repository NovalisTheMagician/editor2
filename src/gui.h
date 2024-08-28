#pragma once

#include "editor.h"

void SetStyle(enum Theme theme);

void InitGui(void);
void FreeGui(void);
bool DoGui(struct EdState *state, bool quitRequest);
