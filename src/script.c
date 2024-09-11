#include "script.h"

#include <dirent.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdio.h>

#include "editor.h"
#include "logging.h"

static void luaError(lua_State *L)
{
    LogError("[script] %s", lua_tostring(L, -1));
}

static void luaWarningFunc(void *user, const char *msg, int tocont)
{
    LogWarning("[Script] %s", msg);
}

static void collectLogData(lua_State *L, char *buffer, size_t bufferSize)
{
    int numArgs = lua_gettop(L);
    size_t offset = 0;
    for(int i = 1; i <= numArgs; ++i)
    {
        if(lua_isstring(L, i))
        {
            const char *str = lua_tostring(L, i);
            int ret = snprintf(buffer + offset, bufferSize - offset, "%s ", str);
            if(ret > 0) offset += ret;
        }
        else if(lua_isboolean(L, i))
        {
            int b = lua_toboolean(L, i);
            int ret = snprintf(buffer + offset, bufferSize - offset, "%s ", b ? "true" : "false");
            if(ret > 0) offset += ret;
        }
        else
        {
            lua_pushstring(L, "incorrect argument");
            lua_error(L);
        }
    }
}

static int logInfoFunc(lua_State *L)
{
    char buffer[2048] = { 0 };
    collectLogData(L, buffer, sizeof buffer);
    LogInfo("[script] %s", buffer);
    return 0;
}

static int logWarnFunc(lua_State *L)
{
    char buffer[2048] = { 0 };
    collectLogData(L, buffer, sizeof buffer);
    LogWarning("[script] %s", buffer);
    return 0;
}

static void registerBuiltins(lua_State *L, EdState *state)
{
    //lua_pushlightuserdata(L, state);
    lua_pushcclosure(L, logInfoFunc, 0);
    lua_setglobal(L, "LogInfo");

    lua_pushcclosure(L, logInfoFunc, 0);
    lua_setglobal(L, "print");

    lua_pushcclosure(L, logWarnFunc, 0);
    lua_setglobal(L, "LogWarn");
}

bool ScriptInit(Script *script, EdState *state)
{
    script->state = luaL_newstate();
    if(!script->state) return false;

    lua_setwarnf(script->state, luaWarningFunc, NULL);

    luaopen_base(script->state);
    luaopen_table(script->state);
    //luaopen_io(script->state);
    luaopen_string(script->state);
    luaopen_math(script->state);

    registerBuiltins(script->state, state);

    lua_pushstring(script->state, "./plugins/?.lua");
    lua_setglobal(script->state, "LUA_PATH");

    return true;
}

void ScriptLoadScripts(Script *script)
{

}

void ScriptDestroy(Script *script)
{
    lua_close(script->state);
}

void ScriptRunString(Script *script, const char *str)
{
    if(luaL_dostring(script->state, str) != LUA_OK)
        luaError(script->state);
}
