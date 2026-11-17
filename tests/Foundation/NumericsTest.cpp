// Tests for the number parsing and formatting helpers in Numerics.h.
#include <Bibim/Config.h>
#include <Bibim/Numerics.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("Int::ToString writes decimal digits with a leading minus for negatives")
{
    CHECK(Int::ToString(0) == "0");
    CHECK(Int::ToString(42) == "42");
    CHECK(Int::ToString(-42) == "-42");
    CHECK(Int::ToString(Int::Max) == "2147483647");
    CHECK(Int::ToString(Int::Min) == "-2147483648");
}

TEST_CASE("LongInt::ToString writes 64-bit values")
{
    CHECK(LongInt::ToString(-1) == "-1");
    CHECK(LongInt::ToString(static_cast<longint>(1) << 40) == "1099511627776");
    CHECK(LongInt::ToString(LongInt::Max) == "9223372036854775807");
    CHECK(LongInt::ToString(LongInt::Min) == "-9223372036854775808");
}

TEST_CASE("Int and LongInt parse what ToString writes")
{
    const int ints[] = { 0, 7, -7, 123456, Int::Max, Int::Min };
    for (int value : ints)
    {
        CAPTURE(value);
        CHECK(Int::Parse(Int::ToString(value)) == value);
    }

    const longint longs[] = { 0, -1, static_cast<longint>(1) << 40, LongInt::Max, LongInt::Min };
    for (longint value : longs)
    {
        CAPTURE(value);
        CHECK(LongInt::Parse(LongInt::ToString(value)) == value);
    }
}

TEST_CASE("Int::Parse reads a leading number and returns 0 for text")
{
    CHECK(Int::Parse("  12") == 12);
    CHECK(Int::Parse("12px") == 12);
    CHECK(Int::Parse("px") == 0);
    CHECK(Int::Parse("") == 0);
}

TEST_CASE("Byte::Parse and ShortInt::Parse clamp to their range")
{
    CHECK(Byte::Parse("200") == 200);
    CHECK(Byte::Parse("300") == Byte::Max);
    CHECK(Byte::Parse("-5") == Byte::Min);
    CHECK(ShortInt::Parse("-1234") == -1234);
    CHECK(ShortInt::Parse("40000") == ShortInt::Max);
    CHECK(ShortInt::Parse("-40000") == ShortInt::Min);
}

TEST_CASE("Bool::Parse accepts true, yes and their first letters in any case")
{
    const char* trueTexts[] = { "true", "TRUE", "True", "yes", "YES", "t", "T", "y", "Y" };
    for (const char* text : trueTexts)
    {
        CAPTURE(text);
        CHECK(Bool::Parse(text));
    }

    const char* falseTexts[] = { "false", "no", "f", "n", "1", "", "truth", "yess" };
    for (const char* text : falseTexts)
    {
        CAPTURE(text);
        CHECK_FALSE(Bool::Parse(text));
    }
}

TEST_CASE("Bool::ToString writes true or false")
{
    CHECK(Bool::ToString(true) == "true");
    CHECK(Bool::ToString(false) == "false");
}

TEST_CASE("Float::ToString writes six decimal places")
{
    CHECK(Float::ToString(3.5f) == "3.500000");
    CHECK(Float::ToString(-0.25f) == "-0.250000");
    CHECK(Float::Parse("2.5") == doctest::Approx(2.5f));
    CHECK(Double::Parse("-1.125") == doctest::Approx(-1.125));
}

TEST_CASE("Double::ToString writes six decimal places like Float::ToString")
{
    CHECK(Double::ToString(3.5) == "3.500000");
    CHECK(Double::ToString(-0.125) == "-0.125000");
    CHECK(Double::ToString(123456789.125) == "123456789.125000");
}
