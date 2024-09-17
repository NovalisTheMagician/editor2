#include "script.h"

#include <dirent.h>
#include <string.h>
#include <assert.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "editor.h"
#include "logging.h"
#include "memory.h"

#include "scripts/scripts.h"

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
        if(offset >= bufferSize) break;
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
        else if(lua_isnil(L, i))
        {
            int ret = snprintf(buffer + offset, bufferSize - offset, "%s ", "nil");
            if(ret > 0) offset += ret;
        }
        else // try to convert the value into a string
        {
            lua_getglobal(L, "tostring");
            lua_pushvalue(L, i);
            if(lua_pcall(L, 1, 1, 0) != LUA_OK)
                luaError(L);

            if(lua_isstring(L, -1))
            {
                const char *str = lua_tostring(L, -1);
                int ret = snprintf(buffer + offset, bufferSize - offset, "%s ", str);
                if(ret > 0) offset += ret;

                lua_pop(L, 1);
            }
            else
            {
                lua_pushfstring(L, "type \"%s\" is not convertible to string", luaL_typename(L, i));
                lua_error(L);
            }
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

static size_t addPlugin(Script *script, size_t len, const char name[static len])
{
    for(size_t i = 0; i < script->numPlugins; ++i)
    {
        if(strncmp(name, script->plugins[i].name, len) == 0)
        {
            return i;
        }
    }

    script->numPlugins++;
    script->plugins = realloc(script->plugins, sizeof *script->plugins * script->numPlugins);
    script->plugins[script->numPlugins-1] = (Plugin){ .name = string_cstr_size(len, name), .flags = 0 };
    return script->numPlugins-1;
}

// void Editor.RegisterPlugin(name: string, execute: function, check?: function, gui?: function)
static int registerPluginFunc(lua_State *L)
{
    size_t nameLen;
    const char *name = luaL_checklstring(L, 1, &nameLen);
    luaL_argexpected(L, lua_isfunction(L, 2), 2, "function");

    if(!lua_isnoneornil(L, 3)&& !lua_isfunction(L, 3))
    {
        luaL_typeerror(L, 3, "function");
    }
    else if(!lua_isnoneornil(L, 4) && !lua_isfunction(L, 4))
    {
        luaL_typeerror(L, 4, "function");
    }

    EdState *state = lua_touserdata(L, lua_upvalueindex(1));

    lua_getglobal(L, "_Plugins");
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    if(lua_istable(L, -1))
    {
        lua_pushfstring(L, "Plugin \"%s\" already registered", name);
        lua_error(L);
    }
    lua_pop(L, 1);

    Script *script = &state->script;
    size_t idx = addPlugin(script, nameLen, name);

    lua_pushstring(L, name);

    lua_newtable(L);
    lua_pushstring(L, "execute");
    lua_pushvalue(L, 2);
    lua_settable(L, -3);

    if(lua_isfunction(L, 3)) // check function
    {
        lua_pushstring(L, "check");
        lua_pushvalue(L, 3);
        lua_settable(L, -3);
        script->plugins[idx].flags |= Plugin_HasPrerequisite;
    }

    if(lua_isfunction(L, 4)) // gui function
    {
        lua_pushstring(L, "gui");
        lua_pushvalue(L, 4);
        lua_settable(L, -3);
        script->plugins[idx].flags |= Plugin_HasGui;
    }

    lua_settable(L, -3);

    lua_pop(L, 1);

    return 0;
}

static void registerBuiltins(lua_State *L, EdState *state)
{
    lua_pushcclosure(L, logInfoFunc, 0);
    lua_setglobal(L, "LogInfo");

    lua_pushcclosure(L, logInfoFunc, 0);
    lua_setglobal(L, "print");

    lua_pushcclosure(L, logWarnFunc, 0);
    lua_setglobal(L, "LogWarn");

    luaL_Reg funcs[] =
    {
        { .name = "RegisterPlugin", .func = registerPluginFunc },
        { NULL, NULL }
    };
    luaL_newlibtable(L, funcs);
    lua_pushlightuserdata(L, state);
    luaL_setfuncs(L, funcs, 1);
    lua_setglobal(L, "Editor");

    ScriptRegisterVecMath(L, state);
    ScriptRegisterEditor(L, state);

    lua_newtable(L);
    lua_setglobal(L, "_Plugins");
}

bool ScriptInit(Script *script, EdState *state)
{
    script->state = luaL_newstate();
    if(!script->state) return false;

    luaL_checkversion(script->state);
    LogInfo("Using Lua Version %d", (int)lua_version(script->state));

    lua_setwarnf(script->state, luaWarningFunc, NULL);

    luaL_requiref(script->state, "_G", luaopen_base, 1);
    lua_pop(script->state, 1);
    luaL_requiref(script->state, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(script->state, 1);
    luaL_requiref(script->state, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(script->state, 1);
    luaL_requiref(script->state, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(script->state, 1);
    luaL_requiref(script->state, LUA_LOADLIBNAME, luaopen_package, 1);
    lua_pop(script->state, 1);

    registerBuiltins(script->state, state);

    lua_pushstring(script->state, "./plugins/dep/?.lua");
    lua_setglobal(script->state, "LUA_PATH");

    ScriptLoadScripts(script);

    return true;
}

void ScriptLoadScripts(Script *script)
{
    DIR *dir = opendir("./plugins");
    while(dir)
    {
        struct dirent *dent = readdir(dir);
        if(!dent) break;

        pstring filename = string_cstr(dent->d_name);
        ssize_t idx = string_last_index_of(filename, 0, ".");
        if(idx != -1 && (strcmp(filename + idx, ".lua") == 0))
        {
            char filepath[512] = { 0 };
            snprintf(filepath, sizeof filepath, "%s/%s", "./plugins", filename);

            size_t cacheSize = script->numPlugins;
            if(luaL_dofile(script->state, filepath) != LUA_OK)
            {
                luaError(script->state);
            }
            else
            {
                if(cacheSize != script->numPlugins) // plugin has been added
                {
                    script->plugins[script->numPlugins-1].file = string_cstr(filepath);
                }
            }
        }
        string_free(filename);
    }
    closedir(dir);
}

void ScriptDestroy(Script *script)
{
    lua_close(script->state);
    for(size_t i = 0; i < script->numPlugins; ++i)
    {
        string_free(script->plugins[i].name);
        string_free(script->plugins[i].file);
    }
    free(script->plugins);
}

void ScriptReloadPlugin(Script *script, size_t idx)
{
    assert(idx < script->numPlugins);

    lua_State *L = script->state;
    lua_getglobal(L, "_Plugins");
    lua_pushstring(L, script->plugins[idx].name);
    lua_pushnil(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    if(luaL_dofile(L, script->plugins[idx].file) != LUA_OK)
        luaError(L);
}

void ScriptReloadAll(Script *script)
{
    for(size_t i = 0; i < script->numPlugins; ++i)
    {
        string_free(script->plugins[i].name);
        string_free(script->plugins[i].file);
    }
    free(script->plugins);
    script->numPlugins = 0;
    script->plugins = NULL;
    lua_newtable(script->state);
    lua_setglobal(script->state, "_Plugins");
    ScriptLoadScripts(script);
}

void ScriptPluginExec(Script *script, size_t idx)
{
    assert(idx < script->numPlugins);

    lua_State *L = script->state;

    lua_getglobal(L, "_Plugins");
    lua_pushstring(L, script->plugins[idx].name);
    lua_gettable(L, -2);
    if(!lua_istable(L, -1))
    {
        LogError("Failed to execute plugin: \"%s\" not found", script->plugins[idx].name);
        lua_pop(L, 2);
    }

    lua_pushstring(L, "execute");
    lua_gettable(L, -2);
    if(lua_pcall(L, 0, 0, 0) != LUA_OK)
        luaError(L);

    lua_pop(L, 2);
}

bool ScriptPluginCheck(Script *script, size_t idx)
{
    assert(idx < script->numPlugins);

    lua_State *L = script->state;
    lua_getglobal(L, "_Plugins");
    lua_pushstring(L, script->plugins[idx].name);
    lua_gettable(L, -2);
    if(!lua_istable(L, -1))
    {
        LogError("Failed to execute plugin: \"%s\" not found", script->plugins[idx].name);
        lua_pop(L, 2);
    }

    lua_pushstring(L, "check");
    lua_gettable(L, -2);
    if(!lua_isfunction(L, -1))
    {
        lua_pop(L, 3);
        return true;
    }

    if(lua_pcall(L, 0, 1, 0) != LUA_OK)
        luaError(L);

    bool ret = true;
    if(lua_isboolean(L, -1))
        ret = lua_toboolean(L, -1);

    lua_pop(L, 3);

    return ret;
}

void ScriptRunString(Script *script, const char *str)
{
    if(luaL_dostring(script->state, str) != LUA_OK)
        luaError(script->state);
}
