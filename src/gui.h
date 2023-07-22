#pragma once

#include "common.h"
#include "editor.h"
#include "map.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

void SetStyle(enum Theme theme);
bool DoGui(struct EdState *state, bool quitRequest);
