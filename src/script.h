#pragma once

#include <lua.h>

#include "memory.h"

typedef struct EdState EdState;

typedef enum PluginFlags
{
    Plugin_None = 0,
    Plugin_HasGui = (1<<0),
    Plugin_HasPrerequisite = (1<<1),
} PluginFlags;

typedef struct Plugin
{
    char *name;
    char *file;
    PluginFlags flags;
} Plugin;

typedef struct Script
{
    lua_State *state;
    Plugin *plugins;
    size_t numPlugins;
} Script;

bool ScriptInit(Script *script, EdState *state);
void ScriptLoadScripts(Script *script);
void ScriptDestroy(Script *script);

void ScriptReloadPlugin(Script *script, size_t idx);
void ScriptReloadAll(Script *script);

void ScriptPluginExec(Script *script, size_t idx);
bool ScriptPluginCheck(Script *script, size_t idx);

void ScriptRunString(Script *script, const char *str);
