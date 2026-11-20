#pragma once
#ifndef __BIBIM_GAMEFRAMEWORK_SDL3_H__
#define __BIBIM_GAMEFRAMEWORK_SDL3_H__

#include <Bibim/FWD.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/GameFrameworkBase.h>

namespace Bibim
{
    class GameFramework : public GameFrameworkBase
    {
        BBThisIsNoncopyableClass(GameFramework);
        public:
            virtual ~GameFramework();

            void Run();

            // BBMain passes main's arguments here before the game is created.
            static void SetCommandLine(int argc, const char* const* argv);

        protected:
            GameFramework();
            GameFramework(int width, int height);

            void Construct(int width, int height);

        private:
            bool videoInitialized;
    };
}

#endif
#endif
