#include "logging.h"
#include "map/query.h"
#include "scripts.h"

#include "cglm/types-struct.h"
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "../editor.h"
#include "edit.h"

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
    for(size_t i = 0; i < numVertices; ++i)
    {
        ScriptCreateVec2(L, vertices[i]->pos.x, vertices[i]->pos.y);
        lua_rawseti(L, -2, i+1);
    }
}

static void fillSideData(lua_State *L, Side side)
{
    lua_pushstring(L, "upper");
    if(side.upperTex)
        lua_pushstring(L, side.upperTex);
    else
        lua_pushnil(L);
    lua_settable(L, -3);
    lua_pushstring(L, "middle");
    if(side.middleTex)
        lua_pushstring(L, side.middleTex);
    else
        lua_pushnil(L);
    lua_settable(L, -3);
    lua_pushstring(L, "lower");
    if(side.lowerTex)
        lua_pushstring(L, side.lowerTex);
    else
        lua_pushnil(L);
    lua_settable(L, -3);
}

static void fillMapLines(lua_State *L, Map *map, MapLine **lines, size_t numLines)
{
    for(size_t i = 0; i < numLines; ++i)
    {
        MapLine *line = lines[i];
        MapVertex *a = GetVertex(map, line->a);
        lua_newtable(L);
        lua_pushstring(L, "a");
        ScriptCreateVec2(L, a->pos.x, a->pos.y);
        lua_settable(L, -3);

        lua_pushstring(L, "b");
        ScriptCreateVec2(L, a->pos.x, a->pos.y);
        lua_settable(L, -3);

        lua_pushstring(L, "type");
        lua_pushnumber(L, line->data.type);
        lua_settable(L, -3);

        lua_pushstring(L, "front");
        lua_newtable(L);
        fillSideData(L, line->data.front);
        lua_settable(L, -3);

        lua_pushstring(L, "back");
        lua_newtable(L);
        fillSideData(L, line->data.back);
        lua_settable(L, -3);

        lua_rawseti(L, -2, i+1);
    }
}

static void fillMapSectors(lua_State *L, MapSector **sectors, size_t numSectors)
{
    for(size_t i = 0; i < numSectors; ++i)
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
        fillMapLines(L, &state->map, (MapLine**)state->data.selectedElements, state->data.numSelectedElements);
    else if(state->data.selectionMode == MODE_SECTOR)
        fillMapSectors(L, (MapSector**)state->data.selectedElements, state->data.numSelectedElements);

    lua_settable(L, -3);

    return 1;
}

static int checkselection_(lua_State *L)
{
    EdState *state = lua_touserdata(L, lua_upvalueindex(1));

    const char *mode = luaL_checkstring(L, 1);
    size_t numSelected = (size_t)luaL_checkinteger(L, 2);
    bool exact = false;
    if(!lua_isnoneornil(L, 3))
        exact = lua_toboolean(L, 3);

    bool match = strcmp(mode, selectionModeToString(state->data.selectionMode)) == 0 && (exact ? numSelected == state->data.numSelectedElements : state->data.numSelectedElements >= numSelected);
    lua_pushboolean(L, match);

    return 1;
}

static int insertlines_(lua_State *L)
{
    EdState *state = lua_touserdata(L, lua_upvalueindex(1));

    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2[]");
    bool isLoop = false;
    if(!lua_isnoneornil(L, 2))
        isLoop = lua_toboolean(L, 2);

    int numVertices = luaL_len(L, 1);
    vec2s vertices[numVertices];
    for(int i = 0; i < numVertices; ++i)
    {
        lua_rawgeti(L, 1, i+1);
        lua_pushstring(L, "x");
        lua_gettable(L, -2);
        float x = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_pushstring(L, "y");
        lua_gettable(L, -2);
        float y = lua_tonumber(L, -1);
        lua_pop(L, 1);

        vertices[i] = (vec2s){ .x = x, .y = y };

        lua_pop(L, 1);
    }

    bool res;
    if(isLoop)
        res = EditApplySector(state, numVertices, vertices);
    else
        res = EditApplyLines(state, numVertices, vertices);

    if(!res)
        LogWarning("Failed too insert all lines, queue full. Added too many intersecting lines?");

    return 0;
}

void ScriptRegisterEditor(lua_State *L, EdState *state)
{
    lua_getglobal(L, "Editor");
    luaL_Reg funcs[] =
    {
        { .name = "GetSelection", .func = getselection_ },
        { .name = "CheckSelection", .func = checkselection_ },
        { .name = "InsertLines", .func = insertlines_ },
        { NULL, NULL }
    };
    lua_pushlightuserdata(L, state);
    luaL_setfuncs(L, funcs, 1);
    lua_pop(L, 1);
}
