#pragma once

#include "editor.h"

void SetStyle(enum Theme theme);

void InitGui(void);
void FreeGui(void);
bool DoGui(EdState *state, bool quitRequest);
