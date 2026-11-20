// Tests for FileAssetProvider, which opens "<directory>/<name>.ab" files.
#include <Bibim/Config.h>
#include <Bibim/FileAssetProvider.h>
#include <Bibim/FileStream.h>
#include <Bibim/GameAsset.h>
#include <Bibim/GameAssetStorage.h>
#include <Bibim/GameModuleTree.h>
#include <Bibim/Stream.h>
#include <doctest.h>
#include <string>
#include "Common/TemporaryDirectory.h"

using namespace Bibim;

namespace
{
    class AssetDirectory : public TemporaryDirectory
    {
        public:
            AssetDirectory()
                : path(GetPath())
            {
                FileStreamPtr stream = new FileStream(Get("Sprite/Player.ab"), FileStream::WriteOnly);
                REQUIRE(stream->Write("asset", 5) == 5);
            }

            String path;
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
    FileAssetProvider provider(&storage, directory.path);

    StreamPtr stream = provider.Open("Sprite/Player");
    REQUIRE(stream != nullptr);
    CHECK(ReadAll(stream) == "asset");
}

TEST_CASE("FileAssetProvider accepts Windows-style asset names")
{
    AssetDirectory directory;
    GameModuleTree modules;
    GameAssetStorage storage(&modules);
    FileAssetProvider provider(&storage, directory.path);

    StreamPtr stream = provider.Open("Sprite\\Player");
    REQUIRE(stream != nullptr);
    CHECK(ReadAll(stream) == "asset");
}

TEST_CASE("FileAssetProvider returns no stream for a missing asset")
{
    AssetDirectory directory;
    GameModuleTree modules;
    GameAssetStorage storage(&modules);
    FileAssetProvider provider(&storage, directory.path);

    CHECK(provider.Open("Sprite/Missing") == nullptr);
}

TEST_CASE("GameAssetStorage opens assets through its file provider")
{
    AssetDirectory directory;
    GameModuleTree modules;
    GameAssetStorage storage(&modules);
    FileAssetProvider provider(&storage, directory.path);

    StreamPtr stream = storage.Open("Sprite/Player");
    REQUIRE(stream != nullptr);
    CHECK(ReadAll(stream) == "asset");
}
