// Tests for Preferences, which stores values as JSON in
// Environment::GetAppDataPath(name, "preferences"). The PreferencesTests CTest
// entry in tests/CMakeLists.txt points XDG_DATA_HOME at the build directory, so
// these cases run only from there.
#include <Bibim/Config.h>
#include <Bibim/Any.h>
#include <Bibim/Environment.h>
#include <Bibim/FileStream.h>
#include <Bibim/Preferences.h>
#include <doctest.h>
#include <cstdio>
#include <vector>

using namespace Bibim;

namespace
{
    void WriteFile(const String& path, const char* text, int length)
    {
        FileStreamPtr stream = new FileStream(path, FileStream::WriteOnly);
        REQUIRE(stream->CanWrite());
        REQUIRE(stream->Write(text, length) == length);
    }

    void WriteFile(const String& path, const char* text)
    {
        WriteFile(path, text, String(text).GetLength());
    }

    String ReadFile(const String& path)
    {
        FileStreamPtr stream = new FileStream(path, FileStream::ReadOnly);
        REQUIRE(stream->CanRead());
        const int length = stream->GetLength();
        std::vector<char> buffer(length + 1, '\0');
        REQUIRE(stream->Read(buffer.data(), length) == length);
        return String(buffer.data(), 0, length);
    }
}

TEST_CASE("Preferences reads back committed values")
{
    {
        Preferences preferences("BibimTestsRoundTrip");
        preferences.SetValue("volume", Any(3));
        preferences.SetValue("fullscreen", Any::True);
        preferences.SetValue("name", Any(String("bibim")));
        preferences.Commit();
    }

    Preferences preferences("BibimTestsRoundTrip");
    CHECK(preferences.GetValue("volume") == Any(3));
    CHECK(preferences.GetValue("fullscreen") == Any::True);
    CHECK(preferences.GetValue("name") == Any(String("bibim")));
    CHECK(preferences.GetValue("missing") == Any::Void);
}

TEST_CASE("Preferences treats an empty file as no values")
{
    WriteFile(Environment::GetAppDataPath("BibimTestsEmpty", "preferences"), "", 0);

    Preferences preferences("BibimTestsEmpty");
    CHECK(preferences.GetValue("volume") == Any::Void);
}

TEST_CASE("Preferences writes a compact JSON object with keys in order")
{
    {
        Preferences preferences("BibimTestsFormat");
        preferences.SetValue("volume", Any(-3));
        preferences.SetValue("fullscreen", Any::False);
        preferences.SetValue("name", Any(String("bibim")));
        preferences.Commit();
    }

    const String text = ReadFile(Environment::GetAppDataPath("BibimTestsFormat", "preferences"));
    CHECK(text == "{\"fullscreen\":false,\"name\":\"bibim\",\"volume\":-3}");
    CHECK(text.GetLength() == 47);
}

TEST_CASE("Preferences reads back strings that need escaping")
{
    const String special("quote\" backslash\\ slash/ newline\n tab\t \xed\x95\x9c\xea\xb8\x80");
    {
        Preferences preferences("BibimTestsEscape");
        preferences.SetValue("key \"with\" quotes", Any(special));
        preferences.Commit();
    }

    Preferences preferences("BibimTestsEscape");
    CHECK(preferences.GetValue("key \"with\" quotes") == Any(special));
}

TEST_CASE("Preferences reads a hand-written file")
{
    WriteFile(Environment::GetAppDataPath("BibimTestsHandWritten", "preferences"),
              "{\n"
              "    \"on\": true,\n"
              "    \"off\": false,\n"
              "    \"count\": 2147483647,\n"
              "    \"low\": -2147483648,\n"
              "    \"text\": \"a\\u00e9\\ud55c\\/\\\"\",\n"
              "    \"nothing\": null,\n"
              "    \"list\": [1, 2],\n"
              "    \"child\": {\"x\": 1}\n"
              "}\n");

    Preferences preferences("BibimTestsHandWritten");
    CHECK(preferences.GetValue("on") == Any::True);
    CHECK(preferences.GetValue("off") == Any::False);
    CHECK(preferences.GetValue("count") == Any(2147483647));
    CHECK(preferences.GetValue("low") == Any(-2147483647 - 1));
    CHECK(preferences.GetValue("text") == Any(String("a\xc3\xa9\xed\x95\x9c/\"")));
    CHECK(preferences.GetValue("nothing") == Any::Void);
    CHECK(preferences.GetValue("list") == Any::Void);
    CHECK(preferences.GetValue("child") == Any::Void);
}

TEST_CASE("Preferences treats a malformed file as no values")
{
    WriteFile(Environment::GetAppDataPath("BibimTestsMalformed", "preferences"), "{\"volume\": 3,");

    Preferences preferences("BibimTestsMalformed");
    CHECK(preferences.GetValue("volume") == Any::Void);
}

TEST_CASE("Preferences treats a file whose root is not an object as no values")
{
    const char* const roots[] = { "3", "\"text\"", "[1, 2]", "null" };
    for (const char* root : roots)
    {
        CAPTURE(root);
        WriteFile(Environment::GetAppDataPath("BibimTestsNotObject", "preferences"), root);

        Preferences preferences("BibimTestsNotObject");
        CHECK(preferences.GetValue("volume") == Any::Void);
    }
}

TEST_CASE("Preferences skips numbers that are not ints")
{
    WriteFile(Environment::GetAppDataPath("BibimTestsNumbers", "preferences"),
              "{\"real\": 1.5, \"big\": 2147483648, \"small\": -2147483649, \"huge\": 18446744073709551615, \"ok\": 7}");

    Preferences preferences("BibimTestsNumbers");
    CHECK(preferences.GetValue("real") == Any::Void);
    CHECK(preferences.GetValue("big") == Any::Void);
    CHECK(preferences.GetValue("small") == Any::Void);
    CHECK(preferences.GetValue("huge") == Any::Void);
    CHECK(preferences.GetValue("ok") == Any(7));
}

TEST_CASE("Preferences reads back ints at the limits")
{
    {
        Preferences preferences("BibimTestsIntLimits");
        preferences.SetValue("max", Any(2147483647));
        preferences.SetValue("min", Any(-2147483647 - 1));
        preferences.Commit();
    }

    Preferences preferences("BibimTestsIntLimits");
    CHECK(preferences.GetValue("max") == Any(2147483647));
    CHECK(preferences.GetValue("min") == Any(-2147483647 - 1));
}
