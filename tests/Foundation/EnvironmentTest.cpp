// Tests for Environment on Linux. Environment reads the process environment
// once, at static initialization, so these cases run only from the
// Environment* CTest entries in tests/CMakeLists.txt, which set the variables
// and pass the expected results in BIBIM_EXPECTED_LOCALE and
// BIBIM_EXPECTED_APPDATA.
#include <Bibim/Config.h>
#include <Bibim/Environment.h>
#include <doctest.h>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace Bibim;

namespace
{
    std::string GetExpected(const char* name)
    {
        const char* value = std::getenv(name);
        REQUIRE_MESSAGE(value != nullptr, name << " is not set; run this case through ctest");
        return value;
    }
}

TEST_CASE("Environment maps the locale variables to a locale name")
{
    CHECK(std::string(Environment::GetLocaleName().CStr()) == GetExpected("BIBIM_EXPECTED_LOCALE"));
}

TEST_CASE("Environment uses the current directory as the working directory")
{
    char* current = getcwd(nullptr, 0);
    REQUIRE(current != nullptr);
    CHECK(std::string(Environment::GetWorkingDirectory().CStr()) == current);
    std::free(current);
}

TEST_CASE("Environment::GetAppDataPath creates the app directory under the XDG data directory")
{
    const std::string expected = GetExpected("BIBIM_EXPECTED_APPDATA") + "/BibimTests";
    CHECK(std::string(Environment::GetAppDataPath("BibimTests", "save").CStr()) == expected + "/save");

    struct stat status;
    CHECK(stat(expected.c_str(), &status) == 0);
    CHECK(S_ISDIR(status.st_mode));
}
