// StandardGame::Start builds the whole engine (UI, assets, Lua) and runs
// Startup::All(), so these cases run in their own CTest entry. Assets are read
// relative to the working directory, so each case works in a temporary one.
#include <Bibim/Config.h>
#include <Bibim/BinaryWriter.h>
#include <Bibim/FileStream.h>
#include <Bibim/StandardGame.h>
#include <Bibim/String.h>
#include <doctest.h>
#include <lua.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include "Platform/OffscreenVideo.h"

using namespace Bibim;

namespace
{
    class TestGame : public StandardGame
    {
        public:
            TestGame()
                : StandardGame(Point2(64, 48))
            {
                Start("BibimTests", "0", new LuaBase(this), false);
            }

        protected:
            virtual void ReloadScripts()
            {
            }
    };

    class TemporaryWorkingDirectory
    {
        public:
            TemporaryWorkingDirectory()
            {
                char pattern[] = "/tmp/BibimTestsXXXXXX";
                REQUIRE(mkdtemp(pattern) != nullptr);
                path = pattern;
                char current[4096];
                REQUIRE(getcwd(current, sizeof(current)) != nullptr);
                previous = current;
                REQUIRE(chdir(path.CStr()) == 0);
                REQUIRE(mkdir("Asset", 0755) == 0);
                REQUIRE(mkdir("Asset/Script", 0755) == 0);
            }

            ~TemporaryWorkingDirectory()
            {
                chdir(previous.CStr());
                std::system(String::CFormat("rm -rf '%s'", path.CStr()).CStr());
            }

        private:
            String path;
            String previous;
    };

    // A script asset as the asset tools write it: total size, 32-bit Lua
    // bytecode size, text size, the bytecode, then the source text XORed
    // with 0xA8.
    void WriteScriptAsset(const char* name, const char* byteCode, const char* text)
    {
        const int byteCodeSize = static_cast<int>(std::strlen(byteCode));
        const int textSize = static_cast<int>(std::strlen(text));

        FileStreamPtr stream = new FileStream(name, FileStream::WriteOnly);
        REQUIRE(stream->CanWrite());
        BinaryWriter writer(stream);
        writer.Write(static_cast<int>(sizeof(int) * 2 + byteCodeSize + textSize));
        writer.Write(byteCodeSize);
        writer.Write(textSize);
        stream->Write(byteCode, byteCodeSize);
        for (int i = 0; i < textSize; i++)
        {
            const char c = static_cast<char>(text[i] ^ 0xA8);
            stream->Write(&c, 1);
        }
    }

    lua_Integer GetGlobalInteger(lua_State* L, const char* name)
    {
        lua_getglobal(L, name);
        const lua_Integer value = lua_isnumber(L, -1) ? lua_tointeger(L, -1) : -1;
        lua_pop(L, 1);
        return value;
    }

    String GetGlobalString(lua_State* L, const char* name)
    {
        lua_getglobal(L, name);
        const String value = lua_isstring(L, -1) ? String(lua_tostring(L, -1)) : String("(not a string)");
        lua_pop(L, 1);
        return value;
    }
}

TEST_CASE("StandardGame Lua runs the text source of a script asset on 64-bit builds")
{
    REQUIRE(sizeof(void*) == 8);
    OffscreenVideo video;
    TemporaryWorkingDirectory directory;
    WriteScriptAsset("Asset/Script/LoadTest.ab", "\x1bLuaR not real bytecode", "LoadTestValue = 6 * 7");

    TestGame game;
    game.GetLua()->Load("Asset/Script/LoadTest");
    CHECK(GetGlobalInteger(game.GetLua()->GetState(), "LoadTestValue") == 42);
}

TEST_CASE("StandardGame Lua sees Linux as PLATFORM")
{
    OffscreenVideo video;
    TemporaryWorkingDirectory directory;

    TestGame game;
    CHECK(GetGlobalString(game.GetLua()->GetState(), "PLATFORM") == "Linux");
}
