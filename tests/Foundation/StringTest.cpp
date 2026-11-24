// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
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

TEST_CASE("String::EndsWith and String::EndsWithIgnoreCase find suffixes")
{
    const String s = "game engine";
    CHECK(s.EndsWith("engine"));
    CHECK(s.EndsWith(String("engine")));
    CHECK_FALSE(s.EndsWith("game"));
    CHECK(s.EndsWithIgnoreCase("ENGINE"));
    CHECK(s.EndsWithIgnoreCase(String("Engine")));
    CHECK_FALSE(s.EndsWithIgnoreCase("GAME"));
}

// Without the length check these read before the start of the string, which
// usually still returns false; only ASan catches it.
TEST_CASE("String::EndsWith is false for a suffix longer than the string")
{
    CHECK_FALSE(String("Reimu").EndsWith("Shadow"));
    CHECK_FALSE(String("bim").EndsWith(String("bibim")));
    CHECK_FALSE(String("bim").EndsWithIgnoreCase("BIBIM"));
    CHECK_FALSE(String("bim").EndsWithIgnoreCase(String("Bibim")));
    CHECK_FALSE(String().EndsWith("a"));
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

TEST_CASE("String constructors accept zero characters")
{
    const char* chars = "bibim";
    const String original = "bibim";
    const String strings[] = {
        String(chars, 2, 0),
        String(chars, 5),
        String(chars, 5, 0),
        String(original, 2, 0),
    };
    for (const String& s : strings)
    {
        CHECK(s.IsEmpty());
        CHECK(s.GetLength() == 0);
        CHECK(s.CStr()[0] == '\0');
        CHECK(s == String::Empty);
    }
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

TEST_CASE("String prefix and suffix checks treat an empty string as a match")
{
    const String strings[] = { String("bibim"), String() };
    for (const String& s : strings)
    {
        CHECK(s.StartsWith(""));
        CHECK(s.StartsWith(String()));
        CHECK(s.StartsWithIgnoreCase(""));
        CHECK(s.StartsWithIgnoreCase(String()));
        CHECK(s.EndsWith(""));
        CHECK(s.EndsWith(String()));
        CHECK(s.EndsWithIgnoreCase(""));
        CHECK(s.EndsWithIgnoreCase(String()));
    }
}

TEST_CASE("String::Append adds characters at the end")
{
    String s;
    s.Append("bi");
    s.Append(String("bim"));
    s.Append("bap and more", 3);
    CHECK(s == "bibimbap");
    CHECK(s.GetLength() == 8);

    s.Append("");
    s.Append(static_cast<const char*>(nullptr));
    s.Append(String());
    CHECK(s == "bibimbap");
}

// Insert(int, const char*) is not tested: it calls itself until the stack overflows.
TEST_CASE("String::Insert with a String clamps the index to the string")
{
    String s("bim");
    s.Insert(0, String("bi"));
    CHECK(s == "bibim");
    s.Insert(99, String("bap"));
    CHECK(s == "bibimbap");
    s.Insert(-5, String(">"));
    CHECK(s == ">bibimbap");
    s.Insert(3, String());
    CHECK(s == ">bibimbap");
}

TEST_CASE("Changing a copied String leaves the original unchanged")
{
    const String original("bibim");
    String copy(original);
    copy.ToUpper();
    CHECK(original == "bibim");
    CHECK(copy == "BIBIM");

    String replaced(original);
    replaced.Replace('b', 'p');
    CHECK(original == "bibim");
    CHECK(replaced == "pipim");
}

TEST_CASE("String::CompareIgnoreCase orders strings ignoring letter case")
{
    const String s("Bibim");
    CHECK(s.CompareIgnoreCase("bIBIM") == 0);
    CHECK(s.CompareIgnoreCase(String("BIBIM")) == 0);
    CHECK(s.CompareIgnoreCase("bibimbap") < 0);
    CHECK(s.CompareIgnoreCase("ABC") > 0);
    CHECK(s.CompareIgnoreCase(static_cast<const char*>(nullptr)) > 0);
}

TEST_CASE("String::Equals and EqualsIgnoreCase are false when the lengths differ")
{
    const String s("bibim");
    CHECK(s.Equals("bibim", 5));
    CHECK_FALSE(s.Equals("bibim", 4));
    CHECK_FALSE(s.Equals("bibip", 5));
    CHECK(s.EqualsIgnoreCase("BIBIM", 5));
    CHECK_FALSE(s.EqualsIgnoreCase("BIBIM", 4));
    CHECK(s.EqualsIgnoreCase(String("BiBiM")));
    CHECK_FALSE(s.EqualsIgnoreCase(String("BIBIMBAP")));
}

TEST_CASE("String::Contains and ContainsIgnoreCase find characters and substrings")
{
    const String s("Bibimbap");
    CHECK(s.Contains(String("mba")));
    CHECK_FALSE(s.Contains(String("MBA")));
    CHECK(s.Contains(String()));

    CHECK(s.ContainsIgnoreCase('P'));
    CHECK_FALSE(s.ContainsIgnoreCase('x'));
    CHECK(s.ContainsIgnoreCase("MBA"));
    CHECK(s.ContainsIgnoreCase(String("bAp")));
    CHECK_FALSE(s.ContainsIgnoreCase("bapx"));
    CHECK(s.ContainsIgnoreCase(""));
    CHECK_FALSE(s.ContainsIgnoreCase(static_cast<const char*>(nullptr)));
}

TEST_CASE("String::ContainsIgnoreCase finds a match that starts inside a partial match" * doctest::should_fail())
{
    CHECK(String("aab").ContainsIgnoreCase("AB"));
    CHECK(String("aab").ContainsIgnoreCase(String("AB")));
}

TEST_CASE("String::Find and FindIgnoreCase find a character from an index")
{
    const String s("Bibim");
    CHECK(s.Find('i') == 1);
    CHECK(s.Find('i', 2) == 3);
    CHECK(s.Find('x') == -1);
    CHECK(s.FindIgnoreCase('b') == 0);
    CHECK(s.FindIgnoreCase('B', 1) == 2);
    CHECK(s.FindIgnoreCase('X') == -1);
}

TEST_CASE("String::Find finds a substring from an index")
{
    const String s("abcabc");
    CHECK(s.Find("bc") == 1);
    CHECK(s.Find(String("ca")) == 2);
    CHECK(s.Find("ab", 1) == 3);
    CHECK(s.Find("abc") == 0);
    CHECK(s.Find("x") == -1);
    CHECK(s.Find("abcabcd") == -1);
}

TEST_CASE("String::Find of an empty substring returns the start index, like std::string")
{
    const String s("abc");
    CHECK(s.Find("") == 0);
    CHECK(s.Find("", 2) == 2);
    CHECK(s.Find(String(), 3) == 3);
    CHECK(s.Find("", 4) == -1);
    CHECK(s.Find("a", 0x7FFFFFFF) == -1);
    CHECK(s.Find(static_cast<const char*>(nullptr)) == -1);
}

TEST_CASE("String::Find finds a substring that ends at the last character")
{
    CHECK(String("abc").Find("bc") == 1);
    CHECK(String("abc").Find(String("abc")) == 0);
    CHECK(String("abcabc").Find("bc", 2) == 4);
}

TEST_CASE("String::FindIgnoreCase returns the index of a substring")
{
    CHECK(String("xAb").FindIgnoreCase("aB") == 1);
    CHECK(String("abab").FindIgnoreCase(String("AB"), 1) == 2);
    CHECK(String("aab").FindIgnoreCase("AB") == 1);
    CHECK(String("\xC3\xA9" "ab").FindIgnoreCase("AB") == 2);
}

TEST_CASE("String::FindIgnoreCase of an empty substring returns the start index, like std::string")
{
    const String s("bibim");
    CHECK(s.FindIgnoreCase("") == 0);
    CHECK(s.FindIgnoreCase("", 2) == 2);
    CHECK(s.FindIgnoreCase(String(), 5) == 5);
    CHECK(s.FindIgnoreCase("", 6) == -1);
    CHECK(s.FindIgnoreCase("B", 0x7FFFFFFF) == -1);
}

TEST_CASE("String::FindIgnoreCase returns -1 for a null or absent substring")
{
    const String s("bibim");
    CHECK(s.FindIgnoreCase(static_cast<const char*>(nullptr)) == -1);
    CHECK(s.FindIgnoreCase("X") == -1);
}

TEST_CASE("String::ReverseFind and ReverseFindIgnoreCase find a character searching backward")
{
    const String s("Bibim");
    CHECK(s.ReverseFind('i') == 3);
    CHECK(s.ReverseFind('i', 2) == 1);
    CHECK(s.ReverseFind('x') == -1);
    CHECK(s.ReverseFindIgnoreCase('b') == 2);
    CHECK(s.ReverseFindIgnoreCase('b', 1) == 0);
    CHECK(s.ReverseFindIgnoreCase('X') == -1);
    CHECK(String().ReverseFind('a') == -1);
}

TEST_CASE("String::ReverseFind finds a substring searching backward")
{
    const String s("abcabcd");
    CHECK(s.ReverseFind("bc") == 4);
    CHECK(s.ReverseFind(String("ab")) == 3);
    CHECK(s.ReverseFind("ab", 3) == 0);
    CHECK(s.ReverseFind("x") == -1);
}

TEST_CASE("String::ReverseFind finds a substring that ends at the search index" * doctest::should_fail())
{
    CHECK(String("abc").ReverseFind("c") == 2);
    CHECK(String("abcab").ReverseFind(String("ab")) == 3);
    CHECK(String("abcab").ReverseFind("ab", 1) == 0);
}

TEST_CASE("String::ReverseFindIgnoreCase finds a substring searching backward" * doctest::should_fail())
{
    CHECK(String("abcABC").ReverseFindIgnoreCase("bc") == 4);
    CHECK(String("abcABC").ReverseFindIgnoreCase(String("AB"), 3) == 0);
}

TEST_CASE("String::Substring from an index keeps the rest of the string")
{
    const String s("bibimbap");
    CHECK(s.Substring(5) == "bap");
    CHECK(s.Substring(0) == s);
    CHECK(s.Substring(-3) == s);
    CHECK(s.Substring(8).IsEmpty());
    CHECK(s.Substring(20).IsEmpty());
}

TEST_CASE("String::Count counts a character")
{
    CHECK(String("bibimbap").Count('b') == 3);
    CHECK(String("bibimbap").Count('x') == 0);
    CHECK(String().Count('b') == 0);
}
