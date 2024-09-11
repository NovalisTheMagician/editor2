#pragma once

#include <lua.h>

typedef struct EdState EdState;

typedef struct Extension
{
    char *name;
    char *func;
} Extension;

typedef struct Script
{
    lua_State *state;
} Script;

bool ScriptInit(Script *script, EdState *state);
void ScriptLoadScripts(Script *script);
void ScriptDestroy(Script *script);

void ScriptRunString(Script *script, const char *str);
