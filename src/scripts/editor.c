#include "scripts.h"

#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "../editor.h"

static const char* selectionModeToString(int selectionMode)
{
    switch(selectionMode)
    {
    case MODE_VERTEX: return "vertex";
    case MODE_LINE: return "line";
    case MODE_SECTOR: return "sector";
    }
    return "ERROR";
}

static void fillMapVertices(lua_State *L, MapVertex **vertices, size_t numVertices)
{
    for(int i = 0; i < numVertices; ++i)
    {
        ScriptCreateVec2(L, vertices[i]->pos.x, vertices[i]->pos.y);
        lua_rawseti(L, -2, i+1);
    }
}

static void fillMapLines(lua_State *L, MapLine **lines, size_t numLines)
{
    for(int i = 0; i < numLines; ++i)
    {
        lua_newtable(L);
        lua_pushstring(L, "a");
        ScriptCreateVec2(L, lines[i]->a->pos.x, lines[i]->a->pos.y);
        lua_settable(L, -3);

        lua_pushstring(L, "b");
        ScriptCreateVec2(L, lines[i]->a->pos.x, lines[i]->a->pos.y);
        lua_settable(L, -3);

        lua_rawseti(L, -2, i+1);
    }
}

static void fillMapSectors(lua_State *L, MapSector **sectors, size_t numSectors)
{
    for(int i = 0; i < numSectors; ++i)
    {
        lua_pushlightuserdata(L, sectors[i]);

        lua_rawseti(L, -2, i+1);
    }
}

static int getselection_(lua_State *L)
{
    EdState *state = lua_touserdata(L, lua_upvalueindex(1));

    lua_newtable(L);
    lua_pushstring(L, "mode");
    lua_pushstring(L, selectionModeToString(state->data.selectionMode));
    lua_settable(L, -3);

    lua_pushstring(L, "items");
    lua_createtable(L, state->data.numSelectedElements, 0);
    if(state->data.selectionMode == MODE_VERTEX)
        fillMapVertices(L, (MapVertex**)state->data.selectedElements, state->data.numSelectedElements);
    else if(state->data.selectionMode == MODE_LINE)
        fillMapLines(L, (MapLine**)state->data.selectedElements, state->data.numSelectedElements);
    else if(state->data.selectionMode == MODE_SECTOR)
        fillMapSectors(L, (MapSector**)state->data.selectedElements, state->data.numSelectedElements);

    lua_settable(L, -3);

    return 1;
}

static int checkselection_(lua_State *L)
{
    EdState *state = lua_touserdata(L, lua_upvalueindex(1));

    const char *mode = luaL_checkstring(L, 1);
    int numSelected = luaL_checkinteger(L, 2);
    bool exact = false;
    if(!lua_isnoneornil(L, 3))
        exact = lua_toboolean(L, 3);

    bool match = strcmp(mode, selectionModeToString(state->data.selectionMode)) == 0 && (exact ? numSelected == state->data.numSelectedElements : state->data.numSelectedElements >= numSelected);
    lua_pushboolean(L, match);

    return 1;
}

void ScriptRegisterEditor(lua_State *L, EdState *state)
{
    lua_getglobal(L, "Editor");
    luaL_Reg funcs[] = {
        { .name = "GetSelection", .func = getselection_ },
        { .name = "CheckSelection", .func = checkselection_ },
        { NULL, NULL }
    };
    lua_pushlightuserdata(L, state);
    luaL_setfuncs(L, funcs, 1);
    lua_pop(L, 1);
}
