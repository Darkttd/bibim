#include <Bibim/Config.h>
#include <Bibim/String.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("String compares equal to a C string with the same characters")
{
    const String s = "bibim";
    CHECK(s == "bibim");
    CHECK(s.GetLength() == 5);
}

TEST_CASE("String::Trim removes surrounding white space")
{
    String s = "  Hello ";
    s.Trim();
    CHECK(s == "Hello");
}

TEST_CASE("String::ToUpper and String::ToLower change letter case")
{
    String s = "Bibim";
    s.ToUpper();
    CHECK(s == "BIBIM");
    s.ToLower();
    CHECK(s == "bibim");
}

TEST_CASE("String::EqualsIgnoreCase ignores letter case")
{
    const String s = "Bibim";
    CHECK(s.EqualsIgnoreCase("BIBIM"));
    CHECK_FALSE(s.EqualsIgnoreCase("bibimbap"));
}

TEST_CASE("String::StartsWith and String::Contains find substrings")
{
    const String s = "game engine";
    CHECK(s.StartsWith("game"));
    CHECK_FALSE(s.StartsWith("engine"));
    CHECK(s.Contains("me en"));
    CHECK(s.Contains('g'));
    CHECK_FALSE(s.Contains("bibim"));
}

TEST_CASE("String::CFormat formats its arguments")
{
    CHECK(String::CFormat("%d-%s", 42, "bibim") == "42-bibim");
    CHECK(String::CFormat("%s has %d stages, %.1f%% clear", "Maple", 300, 87.5) == "Maple has 300 stages, 87.5% clear");
    CHECK(String::CFormat("%s", "") == "");
}

TEST_CASE("String::Substring stops at the end of the string")
{
    const String s = "bibim";
    CHECK(s.Substring(1, 2) == "ib");
    CHECK(s.Substring(2, 3) == "bim");
    // Compare lengths too: == against a C string stops at the first NUL.
    CHECK(s.Substring(2, 100) == "bim");
    CHECK(s.Substring(2, 100).GetLength() == 3);
    CHECK(s.Substring(0, 100) == "bibim");
    CHECK(s.Substring(-2, 4) == "bi");
    CHECK(s.Substring(5, 1) == String::Empty);
}

TEST_CASE("Equal strings compare equal however they were built, including past 64 characters")
{
    // String compares hash codes first, so every way of building a string must
    // compute the same hash for the same characters.
    const int lengths[] = { 10, 64, 65, 100 };
    for (int length : lengths)
    {
        CAPTURE(length);
        char characters[101] = { };
        for (int i = 0; i < length; i++)
            characters[i] = 'a';

        const String whole(characters);
        String appended(characters, 0, length - 1);
        appended.Append("a");

        CHECK(appended.GetLength() == length);
        CHECK(whole == appended);
    }
}
