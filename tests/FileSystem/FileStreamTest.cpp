// Tests for FileStream on Linux. Each case works in its own temporary directory.
#include <Bibim/Config.h>
#include <Bibim/FileStream.h>
#include <doctest.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

using namespace Bibim;

namespace
{
    class TemporaryDirectory
    {
        public:
            TemporaryDirectory()
            {
                char pattern[] = "/tmp/BibimTestsXXXXXX";
                REQUIRE(mkdtemp(pattern) != nullptr);
                path = pattern;
            }

            ~TemporaryDirectory()
            {
                const std::string command = "rm -rf '" + path + "'";
                std::system(command.c_str());
            }

            String Get(const char* name) const
            {
                return String((path + "/" + name).c_str());
            }

        private:
            std::string path;
    };

    void WriteFile(const String& path, const char* text)
    {
        FileStreamPtr stream = new FileStream(path, FileStream::WriteOnly);
        REQUIRE(stream->CanWrite());
        const int length = static_cast<int>(std::strlen(text));
        REQUIRE(stream->Write(text, length) == length);
    }

    std::string ReadFile(const String& path)
    {
        FileStreamPtr stream = new FileStream(path, FileStream::ReadOnly);
        REQUIRE(stream->CanRead());
        char buffer[64] = { 0 };
        const int read = stream->Read(buffer, sizeof(buffer));
        return std::string(buffer, read);
    }
}

TEST_CASE("FileStream reads back what it wrote")
{
    TemporaryDirectory directory;
    const String path = directory.Get("a.bin");
    {
        FileStreamPtr stream = new FileStream(path, FileStream::WriteOnly);
        CHECK(stream->CanWrite());
        CHECK_FALSE(stream->CanRead());
        CHECK(stream->Write("hello", 5) == 5);
        CHECK(stream->GetLength() == 5);
    }

    FileStreamPtr stream = new FileStream(path, FileStream::ReadOnly);
    CHECK(stream->CanRead());
    CHECK_FALSE(stream->CanWrite());
    CHECK(stream->GetLength() == 5);
    char buffer[8] = { 0 };
    CHECK(stream->Read(buffer, sizeof(buffer)) == 5);
    CHECK(std::string(buffer) == "hello");
    CHECK(stream->Read(buffer, sizeof(buffer)) == 0);
}

TEST_CASE("FileStream cannot read a missing file")
{
    TemporaryDirectory directory;
    FileStreamPtr stream = new FileStream(directory.Get("missing.bin"), FileStream::ReadOnly);
    CHECK_FALSE(stream->CanRead());
    char buffer[4];
    CHECK(stream->Read(buffer, sizeof(buffer)) == 0);
    CHECK(stream->GetLength() == 0);
}

TEST_CASE("FileStream Seek returns the new position")
{
    TemporaryDirectory directory;
    const String path = directory.Get("seek.bin");
    WriteFile(path, "0123456789");

    FileStreamPtr stream = new FileStream(path, FileStream::ReadOnly);
    CHECK(stream->Seek(3, Stream::FromBegin) == 3);
    CHECK(stream->Seek(2, Stream::FromCurrent) == 5);
    CHECK(stream->GetPosition() == 5);
    char c = 0;
    CHECK(stream->Read(&c, 1) == 1);
    CHECK(c == '5');
    CHECK(stream->Seek(-1, Stream::FromEnd) == 9);
}

TEST_CASE("FileStream WriteOnly replaces an existing file")
{
    TemporaryDirectory directory;
    const String path = directory.Get("replace.txt");
    WriteFile(path, "long contents");
    WriteFile(path, "short");
    CHECK(ReadFile(path) == "short");
}

TEST_CASE("FileStream WriteOnly creates a missing parent directory")
{
    TemporaryDirectory directory;
    const String path = directory.Get("sub/file.txt");
    WriteFile(path, "nested");
    CHECK(ReadFile(path) == "nested");
}

TEST_CASE("FileStream treats backslashes as directory separators")
{
    TemporaryDirectory directory;
    WriteFile(directory.Get("sub\\file.txt"), "nested");
    CHECK(ReadFile(directory.Get("sub/file.txt")) == "nested");
}
