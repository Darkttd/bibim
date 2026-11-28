// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
#include <Bibim/Config.h>
#include <Bibim/ForkStream.h>
#include <Bibim/MemoryStream.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    class LimitedStream : public Stream
    {
        public:
            LimitedStream(int writeLimit, bool canWrite)
                : writeLimit(writeLimit),
                  canWrite(canWrite),
                  flushCount(0)
            {
            }

            virtual int Read(void* /*buffer*/, int /*size*/) { return 0; }
            virtual int Write(const void* /*buffer*/, int size) { return size < writeLimit ? size : writeLimit; }
            virtual void Flush() { flushCount++; }
            virtual int Seek(int /*offset*/, SeekOrigin /*origin*/) { return 0; }
            virtual bool CanRead() const { return false; }
            virtual bool CanWrite() const { return canWrite; }
            virtual bool CanSeek() const { return false; }

            int GetFlushCount() const { return flushCount; }

        private:
            int writeLimit;
            bool canWrite;
            int flushCount;
    };
}

TEST_CASE("ForkStream keeps the non-null streams in order")
{
    MemoryStreamPtr a = MemoryStream::NewWritableStream(4, true);
    MemoryStreamPtr b = MemoryStream::NewWritableStream(4, true);

    ForkStream none;
    CHECK(none.GetStreams().empty());

    ForkStream fork(a, nullptr, b);
    REQUIRE(fork.GetStreams().size() == 2);
    CHECK(fork.GetStreams()[0] == a);
    CHECK(fork.GetStreams()[1] == b);
}

TEST_CASE("ForkStream::Write writes the same bytes to every stream")
{
    MemoryStreamPtr a = MemoryStream::NewWritableStream(4, true);
    MemoryStreamPtr b = MemoryStream::NewWritableStream(4, true);
    ForkStream fork(a, b);

    const byte data[] = { 1, 2, 3, 4, 5, 6 };
    fork.Write(data, 6);

    REQUIRE(a->GetLength() == 6);
    REQUIRE(b->GetLength() == 6);
    for (int i = 0; i < 6; i++)
    {
        CAPTURE(i);
        CHECK(a->GetBuffer()[i] == data[i]);
        CHECK(b->GetBuffer()[i] == data[i]);
    }
}

TEST_CASE("ForkStream::Write returns the smallest size written to a stream")
{
    MemoryStreamPtr a = MemoryStream::NewWritableStream(4, true);
    MemoryStreamPtr b = MemoryStream::NewWritableStream(4, true);
    ForkStream full(a, b);

    const byte data[] = { 1, 2, 3, 4, 5, 6 };
    CHECK(full.Write(data, 6) == 6);

    StreamPtr limited = new LimitedStream(2, true);
    ForkStream partial(a, limited, b);
    CHECK(partial.Write(data, 6) == 2);
}

TEST_CASE("ForkStream::Flush flushes every stream")
{
    SharedPointer<LimitedStream> a = new LimitedStream(8, true);
    SharedPointer<LimitedStream> b = new LimitedStream(8, true);
    ForkStream fork(a, b);

    fork.Flush();
    CHECK(a->GetFlushCount() == 1);
    CHECK(b->GetFlushCount() == 1);
}

TEST_CASE("ForkStream can write only when every stream can, and cannot read or seek")
{
    StreamPtr writable = new LimitedStream(8, true);
    StreamPtr readOnly = new LimitedStream(8, false);

    ForkStream allWritable(writable, writable);
    CHECK(allWritable.CanWrite());

    ForkStream oneReadOnly(writable, readOnly);
    CHECK_FALSE(oneReadOnly.CanWrite());

    CHECK_FALSE(allWritable.CanRead());
    CHECK_FALSE(allWritable.CanSeek());

    byte buffer[4] = {};
    CHECK(allWritable.Read(buffer, 4) == 0);
    CHECK(allWritable.Seek(1, Stream::FromBegin) == 0);
    CHECK(allWritable.GetPosition() == 0);
    CHECK(allWritable.GetLength() == 0);
}

TEST_CASE("ForkStream without streams can write and reports every byte written")
{
    ForkStream none;
    CHECK(none.CanWrite());

    const byte data[] = { 1, 2, 3 };
    CHECK(none.Write(data, 3) == 3);
    none.Flush();
}
