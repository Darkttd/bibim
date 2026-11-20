// Tests for MPQ, MPQStream and MPQAssetProvider on archives that each case
// creates with StormLib.
#include <Bibim/Config.h>
#include <Bibim/MPQ.h>
#include <Bibim/MPQAssetProvider.h>
#include <Bibim/MPQStream.h>
#include <Bibim/String.h>
#include <StormLib.h>
#include <doctest.h>
#include <cstring>
#include "Common/TemporaryDirectory.h"

using namespace Bibim;

namespace
{
    const char Contents[] = "Hello, MPQ archive!";
    const int ContentsLength = sizeof(Contents) - 1;

    class TemporaryArchive
    {
        public:
            TemporaryArchive()
                : path(directory.Get("Test.mpq"))
            {
                HANDLE archive = nullptr;
                REQUIRE(SFileCreateArchive(path.CStr(), MPQ_CREATE_ARCHIVE_V1, 16, &archive));
                AddFile(archive, "Data\\Hello.ab");
                AddFile(archive, "Readme.txt");
                REQUIRE(SFileCloseArchive(archive));
            }

            TemporaryDirectory directory;
            String path;

        private:
            static void AddFile(HANDLE archive, const char* name)
            {
                HANDLE file = nullptr;
                REQUIRE(SFileCreateFile(archive, name, 0, ContentsLength, 0, MPQ_FILE_COMPRESS, &file));
                REQUIRE(SFileWriteFile(file, Contents, ContentsLength, MPQ_COMPRESSION_ZLIB));
                REQUIRE(SFileFinishFile(file));
            }
    };

    String ReadAll(Stream* stream)
    {
        char buffer[64] = { 0 };
        const int read = stream->Read(buffer, sizeof(buffer) - 1);
        return String(buffer, 0, read);
    }
}

TEST_CASE("MPQ opens an archive and finds its files")
{
    TemporaryArchive archive;
    MPQPtr mpq = new MPQ(archive.path);
    REQUIRE(mpq->GetHandle() != nullptr);

    CHECK(mpq->Has("Readme.txt"));
    CHECK(mpq->Has("Data\\Hello.ab"));
    CHECK(mpq->Has("data/hello.ab"));
    CHECK_FALSE(mpq->Has("Missing.txt"));

    mpq->Close();
    CHECK(mpq->GetHandle() == nullptr);
    CHECK_FALSE(mpq->Has("Readme.txt"));
}

TEST_CASE("MPQ has no handle when the archive does not exist")
{
    TemporaryArchive archive;
    MPQPtr mpq = new MPQ(archive.directory.Get("Missing.mpq"));
    CHECK(mpq->GetHandle() == nullptr);
    CHECK_FALSE(mpq->Has("Readme.txt"));
}

TEST_CASE("MPQStream reads and seeks a file in the archive")
{
    TemporaryArchive archive;
    MPQPtr mpq = new MPQ(archive.path);
    MPQStream stream(mpq, "Readme.txt");

    CHECK(stream.CanRead());
    CHECK_FALSE(stream.CanWrite());
    CHECK(stream.GetLength() == ContentsLength);

    char buffer[8] = { 0 };
    CHECK(stream.Read(buffer, 5) == 5);
    CHECK(std::memcmp(buffer, "Hello", 5) == 0);
    CHECK(stream.GetPosition() == 5);

    CHECK(stream.Seek(2, Stream::FromCurrent) == 7);
    CHECK(stream.Read(buffer, 3) == 3);
    CHECK(std::memcmp(buffer, "MPQ", 3) == 0);

    CHECK(stream.Seek(-8, Stream::FromEnd) == ContentsLength - 8);
    CHECK(stream.Read(buffer, 8) == 8);
    CHECK(std::memcmp(buffer, "archive!", 8) == 0);
    CHECK(stream.Read(buffer, 8) == 0);

    CHECK(stream.Seek(0, Stream::FromBegin) == 0);
    CHECK(ReadAll(&stream) == Contents);
}

TEST_CASE("MPQStream opens a file named with slashes")
{
    TemporaryArchive archive;
    MPQPtr mpq = new MPQ(archive.path);
    MPQStream stream(mpq, "Data/Hello.ab");
    CHECK(ReadAll(&stream) == Contents);
}

TEST_CASE("MPQStream reads nothing from a file that is not in the archive")
{
    TemporaryArchive archive;
    MPQPtr mpq = new MPQ(archive.path);
    MPQStream stream(mpq, "Missing.txt");

    char buffer[8] = { 0 };
    CHECK(stream.Read(buffer, sizeof(buffer)) == 0);
    CHECK(stream.GetLength() == 0);
}

TEST_CASE("MPQAssetProvider opens <name>.ab from its archive")
{
    TemporaryArchive archive;
    MPQAssetProvider provider;
    provider.SetMPQ(new MPQ(archive.path));

    StreamPtr stream = provider.Open("Data\\Hello");
    REQUIRE(stream != nullptr);
    CHECK(ReadAll(stream) == Contents);

    StreamPtr slashed = provider.Open("Data/Hello");
    CHECK(slashed != nullptr);
    StreamPtr missing = provider.Open("Data\\Missing");
    CHECK(missing == nullptr);
}
