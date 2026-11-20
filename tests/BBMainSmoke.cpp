// A game built with BBMain, as Maple is. It checks its startup
// args against BIBIM_EXPECTED_STARTUP_ARGS and exits after a few frames.
#include <Bibim/Config.h>
#include <Bibim/GameFramework.h>
#include <Bibim/Startup.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Platform/PinLoadedLibraries.h"

class SmokeGame : public Bibim::GameFramework
{
    public:
        SmokeGame()
            : frames(0)
        {
            SetFixedTimeStep(false);

            const char* expected = std::getenv("BIBIM_EXPECTED_STARTUP_ARGS");
            if (expected == nullptr || std::strcmp(GetStartupArgs().CStr(), expected) != 0)
            {
                std::fprintf(stderr, "startup args: \"%s\", expected: \"%s\"\n",
                             GetStartupArgs().CStr(), expected ? expected : "(unset)");
                std::abort();
            }
        }

    protected:
        virtual void Draw()
        {
            if (++frames == 1)
                PinLoadedLibraries();
            if (frames == 3)
                Exit();
        }

    private:
        int frames;
};

BBMain(SmokeGame);
