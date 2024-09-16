#pragma once

#include "../script.h"

void ScriptCreateVec2(lua_State *L, float x, float y);

void ScriptRegisterVecMath(lua_State *L, EdState *state);
void ScriptRegisterEditor(lua_State *L, EdState *state);
