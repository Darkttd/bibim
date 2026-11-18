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

using namespace Bibim;

namespace
{
    void WriteFile(const String& path, const char* text, int length)
    {
        FileStreamPtr stream = new FileStream(path, FileStream::WriteOnly);
        REQUIRE(stream->CanWrite());
        REQUIRE(stream->Write(text, length) == length);
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
