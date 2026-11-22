#include <doctest.h>

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

// luai_hashnum adds the two 32-bit halves of a non-integer key's double,
// which overflows an int for many of these keys; only UBSan catches it.
TEST_CASE("Lua table keeps values stored under non-integer number keys")
{
    lua_State* L = luaL_newstate();
    REQUIRE(L != nullptr);
    luaL_openlibs(L);

    const char* script =
        "local t = {}\n"
        "for i = 1, 1000 do t[i / 7] = i end\n"
        "for i = 1, 1000 do\n"
        "  if t[i / 7] ~= i then return i end\n"
        "end\n"
        "return 0\n";
    REQUIRE(luaL_dostring(L, script) == LUA_OK);
    CHECK(lua_tointeger(L, -1) == 0);

    lua_close(L);
}
