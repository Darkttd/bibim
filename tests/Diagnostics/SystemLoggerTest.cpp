// Tests for SystemLogger, which writes to stderr on Linux in the same format
// as the Windows debugger output.
#include <Bibim/Config.h>
#include <Bibim/SystemLogger.h>
#include <doctest.h>
#include <cstdio>
#include <string>
#include <unistd.h>

using namespace Bibim;

namespace
{
    template <typename F> std::string CaptureStderr(F write)
    {
        std::fflush(stderr);
        const int saved = dup(STDERR_FILENO);
        FILE* capture = std::tmpfile();
        REQUIRE(capture != nullptr);
        dup2(fileno(capture), STDERR_FILENO);

        write();

        std::fflush(stderr);
        dup2(saved, STDERR_FILENO);
        close(saved);

        std::string text;
        std::rewind(capture);
        for (int c = std::fgetc(capture); c != EOF; c = std::fgetc(capture))
            text += static_cast<char>(c);
        std::fclose(capture);
        return text;
    }
}

TEST_CASE("SystemLogger writes each level to stderr with the logger name")
{
    SystemLogger logger("Game");
    const std::string text = CaptureStderr([&logger]()
    {
        logger.Error("Asset", "missing");
        logger.Warning("Asset", "slow");
        logger.Information("Asset", "loaded");
    });
    CHECK(text == "(Game) <ERROR> [Asset] missing\n"
                  "(Game) <WARN>  [Asset] slow\n"
                  "(Game) <INFO>  [Asset] loaded\n");
}

TEST_CASE("SystemLogger leaves out the brackets when there is no category")
{
    SystemLogger logger("Game");
    const std::string text = CaptureStderr([&logger]()
    {
        logger.Information(nullptr, "started");
    });
    CHECK(text == "(Game) <INFO>  started\n");
}
