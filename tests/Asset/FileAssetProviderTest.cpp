// Tests for FileAssetProvider, which opens "<directory>/<name>.ab" files.
#include <Bibim/Config.h>
#include <Bibim/FileAssetProvider.h>
#include <Bibim/GameAsset.h>
#include <Bibim/GameAssetStorage.h>
#include <Bibim/GameModuleTree.h>
#include <Bibim/Stream.h>
#include <doctest.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace Bibim;

namespace
{
    class AssetDirectory
    {
        public:
            AssetDirectory()
            {
                char pattern[] = "/tmp/BibimTestsXXXXXX";
                REQUIRE(mkdtemp(pattern) != nullptr);
                path = pattern;
                REQUIRE(mkdir((path + "/Sprite").c_str(), 0755) == 0);
                FILE* file = std::fopen((path + "/Sprite/Player.ab").c_str(), "wb");
                REQUIRE(file != nullptr);
                std::fputs("asset", file);
                std::fclose(file);
            }

            ~AssetDirectory()
            {
                const std::string command = "rm -rf '" + path + "'";
                std::system(command.c_str());
            }

            std::string path;
    };

    std::string ReadAll(Stream* stream)
    {
        char buffer[16] = { 0 };
        const int read = stream->Read(buffer, sizeof(buffer));
        return std::string(buffer, read);
    }
}

TEST_CASE("FileAssetProvider opens an asset file by name")
{
    AssetDirectory directory;
    GameModuleTree modules;
    GameAssetStorage storage(&modules);
    FileAssetProvider provider(&storage, directory.path.c_str());

    StreamPtr stream = provider.Open("Sprite/Player");
    REQUIRE(stream != nullptr);
    CHECK(ReadAll(stream) == "asset");
}

TEST_CASE("FileAssetProvider accepts Windows-style asset names")
{
    AssetDirectory directory;
    GameModuleTree modules;
    GameAssetStorage storage(&modules);
    FileAssetProvider provider(&storage, directory.path.c_str());

    StreamPtr stream = provider.Open("Sprite\\Player");
    REQUIRE(stream != nullptr);
    CHECK(ReadAll(stream) == "asset");
}

TEST_CASE("FileAssetProvider returns no stream for a missing asset")
{
    AssetDirectory directory;
    GameModuleTree modules;
    GameAssetStorage storage(&modules);
    FileAssetProvider provider(&storage, directory.path.c_str());

    CHECK(provider.Open("Sprite/Missing") == nullptr);
}

TEST_CASE("GameAssetStorage opens assets through its file provider")
{
    AssetDirectory directory;
    GameModuleTree modules;
    GameAssetStorage storage(&modules);
    FileAssetProvider provider(&storage, directory.path.c_str());

    StreamPtr stream = storage.Open("Sprite/Player");
    REQUIRE(stream != nullptr);
    CHECK(ReadAll(stream) == "asset");
}
