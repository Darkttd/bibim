// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
#include <Bibim/Config.h>
#include <Bibim/DumpStream.h>
#include <Bibim/MemoryStream.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    class FlushCountingStream : public Stream
    {
        public:
            FlushCountingStream()
                : flushCount(0)
            {
            }

            virtual int Read(void* /*buffer*/, int /*size*/) { return 0; }
            virtual int Write(const void* /*buffer*/, int size) { return size; }
            virtual void Flush() { flushCount++; }
            virtual int Seek(int /*offset*/, SeekOrigin /*origin*/) { return 0; }
            virtual bool CanRead() const { return true; }
            virtual bool CanWrite() const { return true; }
            virtual bool CanSeek() const { return false; }

            int GetFlushCount() const { return flushCount; }

        private:
            int flushCount;
    };
}

TEST_CASE("DumpStream::Read copies what it reads from the base stream to the output stream")
{
    const byte data[] = { 1, 2, 3, 4, 5 };
    MemoryStreamPtr base = MemoryStream::NewReadableStream(data, 5);
    MemoryStreamPtr output = MemoryStream::NewWritableStream(8, true);
    DumpStream dump(base, output);

    CHECK(dump.GetBaseStream() == base);
    CHECK(dump.GetOutputStream() == output);

    byte buffer[8] = {};
    CHECK(dump.Read(buffer, 3) == 3);
    CHECK(buffer[0] == 1);
    CHECK(buffer[2] == 3);
    CHECK(output->GetLength() == 3);

    CHECK(dump.Read(buffer, 8) == 2);
    CHECK(buffer[0] == 4);
    CHECK(buffer[1] == 5);
    REQUIRE(output->GetLength() == 5);
    for (int i = 0; i < 5; i++)
    {
        CAPTURE(i);
        CHECK(output->GetBuffer()[i] == data[i]);
    }

    CHECK(dump.Read(buffer, 8) == 0);
    CHECK(output->GetLength() == 5);
}

TEST_CASE("DumpStream::Write copies what it writes to the base stream to the output stream")
{
    MemoryStreamPtr base = MemoryStream::NewWritableStream(8, true);
    MemoryStreamPtr output = MemoryStream::NewWritableStream(8, true);
    DumpStream dump(base, output);

    const byte data[] = { 6, 7, 8 };
    CHECK(dump.Write(data, 3) == 3);
    REQUIRE(base->GetLength() == 3);
    REQUIRE(output->GetLength() == 3);
    for (int i = 0; i < 3; i++)
    {
        CAPTURE(i);
        CHECK(base->GetBuffer()[i] == data[i]);
        CHECK(output->GetBuffer()[i] == data[i]);
    }
}

TEST_CASE("DumpStream passes position, length, seeking and capabilities through to the base stream")
{
    const byte data[] = { 1, 2, 3, 4, 5 };
    MemoryStreamPtr base = MemoryStream::NewReadableStream(data, 5);
    MemoryStreamPtr output = MemoryStream::NewWritableStream(8, true);
    DumpStream dump(base, output);

    CHECK(dump.GetLength() == 5);
    CHECK(dump.Seek(2, Stream::FromBegin) == 2);
    CHECK(dump.GetPosition() == 2);
    CHECK(base->GetPosition() == 2);
    CHECK(output->GetLength() == 0);

    CHECK(dump.CanRead());
    CHECK_FALSE(dump.CanWrite());
    CHECK(dump.CanSeek());
}

TEST_CASE("DumpStream::Flush flushes the base and the output stream")
{
    SharedPointer<FlushCountingStream> base = new FlushCountingStream();
    SharedPointer<FlushCountingStream> output = new FlushCountingStream();
    DumpStream dump(base, output);

    dump.Flush();
    CHECK(base->GetFlushCount() == 1);
    CHECK(output->GetFlushCount() == 1);
}
