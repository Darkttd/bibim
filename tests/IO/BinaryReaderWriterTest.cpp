// Tests for BinaryReader / BinaryWriter over MemoryStream.
//
// Asset files are written by the Windows tools (Bibim.Toolkit), so the layout
// must not depend on the platform: little-endian, int = 4 bytes, longint = 8
// bytes, strings as a 4-byte length followed by the characters.
#include <Bibim/Config.h>
#include <Bibim/BinaryReader.h>
#include <Bibim/BinaryWriter.h>
#include <Bibim/MemoryStream.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    // Reader over a fixed byte sequence; the stream does not take ownership of it.
    MemoryStreamPtr ReadableStream(const byte* bytes, int length)
    {
        return MemoryStream::NewReadableStream(bytes, length);
    }
}

TEST_CASE("BinaryWriter writes fixed-size, platform-independent values")
{
    MemoryStreamPtr stream = MemoryStream::NewWritableStream(64, true);
    BinaryWriter writer(stream);

    writer.Write(static_cast<short>(1));
    CHECK(stream->GetLength() == 2);
    writer.Write(static_cast<int>(1));
    CHECK(stream->GetLength() == 2 + 4);
    writer.Write(static_cast<longint>(1));
    CHECK(stream->GetLength() == 2 + 4 + 8);
    writer.Write(1.0f);
    CHECK(stream->GetLength() == 2 + 4 + 8 + 4);
    writer.Write(1.0);
    CHECK(stream->GetLength() == 2 + 4 + 8 + 4 + 8);
}

TEST_CASE("Values written by BinaryWriter read back unchanged")
{
    MemoryStreamPtr stream = MemoryStream::NewWritableStream(256, true);
    BinaryWriter writer(stream);
    writer.Write(true);
    writer.Write(static_cast<byte>(200));
    writer.Write(static_cast<short>(-12345));
    writer.Write(-123456789);
    writer.Write(static_cast<longint>(1) << 40);
    writer.Write(3.5f);
    writer.Write(-2.25);
    writer.Write(String("bibim"));
    writer.Write(String::Empty);

    MemoryStreamPtr input = ReadableStream(stream->GetBuffer(), stream->GetLength());
    BinaryReader reader(input);
    CHECK(reader.ReadBool() == true);
    CHECK(reader.ReadByte() == 200);
    CHECK(reader.ReadShortInt() == -12345);
    CHECK(reader.ReadInt() == -123456789);
    CHECK(reader.ReadLongInt() == (static_cast<longint>(1) << 40));
    CHECK(reader.ReadFloat() == doctest::Approx(3.5f));
    CHECK(reader.ReadDouble() == doctest::Approx(-2.25));
    CHECK(reader.ReadString() == "bibim");
    CHECK(reader.ReadString() == String::Empty);
}

TEST_CASE("BinaryReader reads integers as little-endian 32-bit values")
{
    // Header of a script asset (.ab) built by the Windows tools:
    // total size 334, bytecode size 246, text size 80.
    const byte header[] = { 0x4E, 0x01, 0x00, 0x00, 0xF6, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00 };
    MemoryStreamPtr input = ReadableStream(header, sizeof(header));
    BinaryReader reader(input);

    CHECK(reader.ReadInt() == 334);
    CHECK(reader.ReadInt() == 246);
    CHECK(reader.ReadInt() == 80);
}

TEST_CASE("BinaryReader reads strings as a 32-bit length followed by the characters")
{
    const byte bytes[] = { 0x05, 0x00, 0x00, 0x00, 'b', 'i', 'b', 'i', 'm', 0x00, 0x00, 0x00, 0x00 };
    MemoryStreamPtr input = ReadableStream(bytes, sizeof(bytes));
    BinaryReader reader(input);

    CHECK(reader.ReadString() == "bibim");
    CHECK(reader.ReadString() == String::Empty);
}
