#pragma once
#include <lua.h>
#include <iostream>

#undef max

#define lua_normalisestack(L, MaxSize) { if (lua_gettop(L) > MaxSize) lua_settop(L, MaxSize); }

#define lua_isinstance(L, idx) (lua_userdatatag(L, idx) == 1)


inline float lua_tofloat(lua_State* L, int idx) {
    double value = lua_tonumber(L, idx);
    if (value == -std::numeric_limits<double>::infinity())
        return -std::numeric_limits<float>::infinity();

    if (value == std::numeric_limits<double>::infinity())
        return std::numeric_limits<float>::infinity();

    if (not ((value < 0.0) || (value >= 0.0)))
        return (float)value;

    if (value < (double)-std::numeric_limits<float>::max())
        return -std::numeric_limits<float>::max();

    if (value > (double)std::numeric_limits<float>::max())
        return std::numeric_limits<float>::max();

    return (float)value;
}

inline static void PrintStack(lua_State* L) {
    int top = lua_gettop(L);

    for (int i = 1; i <= top; i++) {
        int type = lua_type(L, i);

        std::cout << i << " = ";

        switch (type) {
        case LUA_TNONE:
            std::cout << "none";
            break;
        case LUA_TNIL:
            std::cout << "nil";
            break;
        case LUA_TBOOLEAN:
            std::cout << "boolean: " << (lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            std::cout << "number: " << lua_tonumber(L, i);
            break;
        case LUA_TSTRING:
            std::cout << "string: \"" << lua_tostring(L, i) << "\"";
            break;
        case LUA_TFUNCTION:
            std::cout << "function";
            break;
        case LUA_TTABLE:
            std::cout << "table";
            break;
        case LUA_TUSERDATA:
            std::cout << "userdata: " << lua_topointer(L, i);
            break;
        case LUA_TLIGHTUSERDATA:
            std::cout << "lightuserdata: " << lua_topointer(L, i);
            break;
        case LUA_TTHREAD:
            std::cout << "thread";
            break;
        default:
            std::cout << "unknown";
            break;
        }

        std::cout << "\n";
    }


};