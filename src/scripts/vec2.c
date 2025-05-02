#include "scripts.h"

#include <tgmath.h>

#include "lua.h"
#include "lauxlib.h"
#include "../editor.h"

static int sub_(lua_State *L)
{
    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2");
    luaL_argexpected(L, lua_istable(L, 2), 2, "Vec2");

    lua_pushstring(L, "x");
    lua_gettable(L, 1);
    float x1 = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 1);
    float y1 = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "x");
    lua_gettable(L, 2);
    float x2 = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 2);
    float y2 = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "Vec2");
    lua_pushstring(L, "new");
    lua_gettable(L, -2);
    lua_pushnumber(L, x1 - x2);
    lua_pushnumber(L, y1 - y2);
    lua_pcall(L, 2, 1, 0);

    lua_remove(L, -2);

    return 1;
}

static int add_(lua_State *L)
{
    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2");
    luaL_argexpected(L, lua_istable(L, 2), 2, "Vec2");

    lua_pushstring(L, "x");
    lua_gettable(L, 1);
    float x1 = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 1);
    float y1 = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "x");
    lua_gettable(L, 2);
    float x2 = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 2);
    float y2 = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "Vec2");
    lua_pushstring(L, "new");
    lua_gettable(L, -2);
    lua_pushnumber(L, x1 + x2);
    lua_pushnumber(L, y1 + y2);
    lua_pcall(L, 2, 1, 0);

    lua_remove(L, -2);

    return 1;
}

static int mul_(lua_State *L)
{
    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2");
    float scalar = luaL_checknumber(L, 2);

    lua_pushstring(L, "x");
    lua_gettable(L, 1);
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 1);
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "Vec2");
    lua_pushstring(L, "new");
    lua_gettable(L, -2);
    lua_pushnumber(L, x * scalar);
    lua_pushnumber(L, y * scalar);
    lua_pcall(L, 2, 1, 0);

    lua_remove(L, -2);

    return 1;
}

static int div_(lua_State *L)
{
    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2");
    float scalar = luaL_checknumber(L, 2);

    lua_pushstring(L, "x");
    lua_gettable(L, 1);
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 1);
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "Vec2");
    lua_pushstring(L, "new");
    lua_gettable(L, -2);
    lua_pushnumber(L, x / scalar);
    lua_pushnumber(L, y / scalar);
    lua_pcall(L, 2, 1, 0);

    lua_remove(L, -2);

    return 1;
}

static int len_(lua_State *L)
{
    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2");

    lua_pushstring(L, "x");
    lua_gettable(L, 1);
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 1);
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushnumber(L, sqrt(x*x + y*y));

    return 1;
}

static int len2_(lua_State *L)
{
    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2");

    lua_pushstring(L, "x");
    lua_gettable(L, 1);
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 1);
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushnumber(L, x*x + y*y);

    return 1;
}

static int dot_(lua_State *L)
{
    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2");
    luaL_argexpected(L, lua_istable(L, 2), 2, "Vec2");

    lua_pushstring(L, "x");
    lua_gettable(L, 1);
    float x1 = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 1);
    float y1 = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "x");
    lua_gettable(L, 2);
    float x2 = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 2);
    float y2 = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushnumber(L, x1*x2 + y1*y2);
    lua_pcall(L, 2, 1, 0);

    lua_remove(L, -2);

    return 1;
}

static int tostring_(lua_State *L)
{
    luaL_argexpected(L, lua_istable(L, 1), 1, "Vec2");

    lua_pushstring(L, "x");
    lua_gettable(L, 1);
    float x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_pushstring(L, "y");
    lua_gettable(L, 1);
    float y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushfstring(L, "Vec2(%f | %f)", x, y);

    return 1;
}

static int new_(lua_State *L)
{
    float x = luaL_optnumber(L, 1, 0);
    float y = luaL_optnumber(L, 2, 0);

    lua_newtable(L); // table to return (Vec2)
    luaL_setmetatable(L, "Vec2");

    lua_pushstring(L, "x");
    lua_pushnumber(L, x);
    lua_settable(L, -3);

    lua_pushstring(L, "y");
    lua_pushnumber(L, y);
    lua_settable(L, -3);

    lua_pushstring(L, "length");
    lua_pushcfunction(L, len_);
    lua_settable(L, -3);

    lua_pushstring(L, "length2");
    lua_pushcfunction(L, len2_);
    lua_settable(L, -3);

    lua_pushstring(L, "dot");
    lua_pushcfunction(L, dot_);
    lua_settable(L, -3);

    return 1;
}

void ScriptRegisterVecMath(lua_State *L, EdState *)
{
    luaL_newmetatable(L, "Vec2");
    lua_pushstring(L, "__add");
    lua_pushcfunction(L, add_);
    lua_settable(L, -3);

    lua_pushstring(L, "__sub");
    lua_pushcfunction(L, sub_);
    lua_settable(L, -3);

    lua_pushstring(L, "__mul");
    lua_pushcfunction(L, mul_);
    lua_settable(L, -3);

    lua_pushstring(L, "__div");
    lua_pushcfunction(L, div_);
    lua_settable(L, -3);

    lua_pushstring(L, "__len");
    lua_pushcfunction(L, len_);
    lua_settable(L, -3);

    lua_pushstring(L, "__tostring");
    lua_pushcfunction(L, tostring_);
    lua_settable(L, -3);

    lua_pop(L, 1);

    lua_newtable(L);
    lua_pushstring(L, "new");
    lua_pushcfunction(L, new_);
    lua_settable(L, -3);
    lua_setglobal(L, "Vec2");
}

void ScriptCreateVec2(lua_State *L, float x, float y)
{
    lua_getglobal(L, "Vec2");
    lua_pushstring(L, "new");
    lua_gettable(L, -2);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pcall(L, 2, 1, 0);

    lua_remove(L, -2);
}
