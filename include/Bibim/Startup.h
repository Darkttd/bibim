#pragma once
#ifndef __BIBIM_STARTUP_H__
#define __BIBIM_STARTUP_H__

#include <Bibim/Foundation.h>

namespace Bibim
{
    class Startup
    {
        BBThisIsStaticClass(Startup);
        public:
            static void All();
    };
}

#if (defined(BIBIM_PLATFORM_WINDOWS))
#    include <windows.h>
#    include <crtdbg.h>
#    include <tchar.h>
#    define BBMain(gameclass) \
        int APIENTRY _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) \
        { \
            gameclass g; \
            g.Run(); \
            return 0; \
        }
#elif (defined(BIBIM_PLATFORM_ANDROID) || defined(BIBIM_PLATFORM_IOS))
#   define BBMain(gameClass) \
        Bibim::GameFramework* Bibim::GameFramework::Create() \
        { \
            return new gameClass(); \
        }
#elif (defined(BIBIM_USE_SDL3))
#   include <Bibim/GameFramework.h>
#   define BBMain(gameclass) \
        int main(int argc, char** argv) \
        { \
            Bibim::GameFramework::SetCommandLine(argc, argv); \
            gameclass g; \
            g.Run(); \
            return 0; \
        }
#endif

#endif
