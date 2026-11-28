// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
#include <Bibim/Config.h>
#include <Bibim/MemoryStream.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("MemoryStream::NewReadableStream reads the given bytes")
{
    const byte data[] = { 1, 2, 3, 4, 5 };
    MemoryStreamPtr stream = MemoryStream::NewReadableStream(data, 5);

    CHECK(stream->CanRead());
    CHECK_FALSE(stream->CanWrite());
    CHECK(stream->CanSeek());
    CHECK(stream->GetLength() == 5);
    CHECK(stream->GetCapacity() == 5);
    CHECK(stream->GetPosition() == 0);
    CHECK(stream->GetBuffer() == data);

    byte buffer[5] = {};
    CHECK(stream->Read(buffer, 2) == 2);
    CHECK(buffer[0] == 1);
    CHECK(buffer[1] == 2);
    CHECK(stream->GetPosition() == 2);

    CHECK(stream->Read(buffer, 3) == 3);
    CHECK(buffer[0] == 3);
    CHECK(buffer[2] == 5);
    CHECK(stream->GetPosition() == 5);
}

TEST_CASE("MemoryStream::Read stops at the end of the stream")
{
    const byte data[] = { 1, 2, 3 };
    MemoryStreamPtr stream = MemoryStream::NewReadableStream(data, 3);

    byte buffer[8] = {};
    CHECK(stream->Read(buffer, 8) == 3);
    CHECK(buffer[2] == 3);
    CHECK(stream->GetPosition() == 3);

    CHECK(stream->Read(buffer, 8) == 0);
    CHECK(stream->GetPosition() == 3);
}

TEST_CASE("MemoryStream::NewReadableStream with deleteOnClose reads the buffer it takes")
{
    byte* data = new byte[4];
    data[0] = 7;
    MemoryStreamPtr stream = MemoryStream::NewReadableStream(data, 4, true);

    byte value = 0;
    CHECK(stream->Read(&value, 1) == 1);
    CHECK(value == 7);
}

TEST_CASE("MemoryStream::NewWritableStream starts empty with the given capacity")
{
    MemoryStreamPtr stream = MemoryStream::NewWritableStream(16, true);

    CHECK_FALSE(stream->CanRead());
    CHECK(stream->CanWrite());
    CHECK(stream->CanSeek());
    CHECK(stream->GetLength() == 0);
    CHECK(stream->GetCapacity() == 16);
    CHECK(stream->GetPosition() == 0);

    MemoryStreamPtr zeroed = MemoryStream::NewWritableStream(16, true, true);
    for (int i = 0; i < 16; i++)
        CHECK(zeroed->GetBuffer()[i] == 0);

    MemoryStreamPtr negative = MemoryStream::NewWritableStream(-1, true);
    CHECK(negative->GetCapacity() == 0);
    CHECK(negative->GetBuffer() == nullptr);
}

TEST_CASE("MemoryStream::NewWritableStream fills nothing when it has no buffer")
{
    MemoryStreamPtr empty = MemoryStream::NewWritableStream(0, true, true);
    CHECK(empty->GetCapacity() == 0);
    CHECK(empty->GetBuffer() == nullptr);

    MemoryStreamPtr negative = MemoryStream::NewWritableStream(-1, true, true);
    CHECK(negative->GetCapacity() == 0);
    CHECK(negative->GetBuffer() == nullptr);
}

TEST_CASE("MemoryStream::Write into a stream without a buffer allocates one")
{
    MemoryStreamPtr stream = MemoryStream::NewWritableStream(0, true);
    REQUIRE(stream->GetBuffer() == nullptr);

    const byte data[] = { 9, 8 };
    CHECK(stream->Write(data, 2) == 2);
    CHECK(stream->GetLength() == 2);
    REQUIRE(stream->GetBuffer() != nullptr);
    CHECK(stream->GetBuffer()[0] == 9);
    CHECK(stream->GetBuffer()[1] == 8);
}

TEST_CASE("MemoryStream::Write appends and grows the buffer")
{
    MemoryStreamPtr stream = MemoryStream::NewWritableStream(4, true);

    const byte first[] = { 1, 2, 3 };
    CHECK(stream->Write(first, 3) == 3);
    CHECK(stream->GetLength() == 3);
    CHECK(stream->GetPosition() == 3);
    CHECK(stream->GetCapacity() == 4);

    const byte second[] = { 4, 5, 6, 7, 8 };
    CHECK(stream->Write(second, 5) == 5);
    CHECK(stream->GetLength() == 8);
    CHECK(stream->GetPosition() == 8);
    CHECK(stream->GetCapacity() >= 8);

    for (int i = 0; i < 8; i++)
    {
        CAPTURE(i);
        CHECK(stream->GetBuffer()[i] == i + 1);
    }
}

TEST_CASE("MemoryStream::Write after Seek overwrites and extends the data")
{
    MemoryStreamPtr stream = MemoryStream::NewWritableStream(8, true);
    const byte data[] = { 1, 2, 3, 4 };
    stream->Write(data, 4);

    const byte middle[] = { 7 };
    CHECK(stream->Seek(1, Stream::FromBegin) == 1);
    CHECK(stream->Write(middle, 1) == 1);
    CHECK(stream->GetLength() == 4);
    CHECK(stream->GetPosition() == 2);

    const byte tail[] = { 8, 9, 10 };
    CHECK(stream->Seek(-1, Stream::FromEnd) == 3);
    CHECK(stream->Write(tail, 3) == 3);
    CHECK(stream->GetLength() == 6);

    const byte expected[] = { 1, 7, 3, 8, 9, 10 };
    for (int i = 0; i < 6; i++)
    {
        CAPTURE(i);
        CHECK(stream->GetBuffer()[i] == expected[i]);
    }
}

TEST_CASE("MemoryStream::Seek moves from the beginning, the end and the current position")
{
    const byte data[] = { 1, 2, 3, 4, 5 };
    MemoryStreamPtr stream = MemoryStream::NewReadableStream(data, 5);

    CHECK(stream->Seek(2, Stream::FromBegin) == 2);
    CHECK(stream->GetPosition() == 2);
    CHECK(stream->Seek(1, Stream::FromCurrent) == 3);
    CHECK(stream->Seek(-1, Stream::FromCurrent) == 2);
    CHECK(stream->Seek(0, Stream::FromEnd) == 5);
    CHECK(stream->Seek(-5, Stream::FromEnd) == 0);

    byte value = 0;
    stream->Seek(-2, Stream::FromEnd);
    CHECK(stream->Read(&value, 1) == 1);
    CHECK(value == 4);
}

TEST_CASE("MemoryStream::Seek past the end stops at the end")
{
    const byte data[] = { 1, 2, 3 };
    MemoryStreamPtr stream = MemoryStream::NewReadableStream(data, 3);

    CHECK(stream->Seek(10, Stream::FromBegin) == 3);
    CHECK(stream->Seek(1, Stream::FromEnd) == 3);
    CHECK(stream->Seek(1, Stream::FromCurrent) == 3);
}

TEST_CASE("MemoryStream::Seek before the beginning stops at the beginning"
          * doctest::should_fail(true)
          * doctest::description("Seek clamps only at the end, so the position becomes negative"))
{
    const byte data[] = { 1, 2, 3 };
    MemoryStreamPtr stream = MemoryStream::NewReadableStream(data, 3);

    CHECK(stream->Seek(-1, Stream::FromBegin) == 0);
    CHECK(stream->Seek(-4, Stream::FromEnd) == 0);
    stream->Seek(1, Stream::FromBegin);
    CHECK(stream->Seek(-2, Stream::FromCurrent) == 0);
    CHECK(stream->GetPosition() == 0);
}

TEST_CASE("MemoryStream::Write into a read-only stream writes nothing"
          * doctest::should_fail(true)
          * doctest::description("Write ignores CanWrite and changes the caller's buffer"))
{
    byte data[] = { 1, 2, 3, 4 };
    MemoryStreamPtr stream = MemoryStream::NewReadableStream(data, 4);

    const byte value = 9;
    CHECK(stream->Write(&value, 1) == 0);
    CHECK(data[0] == 1);
    CHECK(stream->GetPosition() == 0);
}
