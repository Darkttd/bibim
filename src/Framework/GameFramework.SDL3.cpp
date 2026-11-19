#include <Bibim/Config.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/GameFramework.SDL3.h>
#include <Bibim/GameWindow.h>
#include <Bibim/Log.h>
#include <SDL3/SDL.h>
#include <string>

namespace Bibim
{
    // Not a String: a default String copies String::Empty, which may not be
    // initialized yet when this file's statics are.
    static std::string CommandLine;

    GameFramework::GameFramework()
    {
        Construct(800, 600);
    }

    GameFramework::GameFramework(int width, int height)
    {
        Construct(width, height);
    }

    GameFramework::~GameFramework()
    {
        if (videoInitialized)
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    void GameFramework::SetCommandLine(int argc, const char* const* argv)
    {
        CommandLine.clear();
        for (int i = 0; i < argc; i++)
        {
            if (i > 0)
                CommandLine += ' ';
            CommandLine += argv[i];
        }
    }

    void GameFramework::Construct(int width, int height)
    {
        // The quit event that GameWindow::Close pushes stays queued only while
        // the video subsystem is held, so it is held until the game is deleted.
        videoInitialized = SDL_InitSubSystem(SDL_INIT_VIDEO);
        if (videoInitialized == false)
            Log::Error("GameFramework", SDL_GetError());

        GameFrameworkBase::Construct(width, height, CommandLine.c_str());
    }

    void GameFramework::Run()
    {
        if (videoInitialized == false)
            return;

        Initialize();
        PostInitialize();

        for (;;)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_EVENT_QUIT:
                        Finalize();
                        return;
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                        Exit();
                        break;
                    case SDL_EVENT_DROP_FILE:
                        GetWindow()->RaiseDropFileEvent(event.drop.data);
                        break;
                }
            }

            // SDL_PollEvent leaves events pushed while polling (the quit event
            // from Exit) for the next call; quit before drawing another frame,
            // as the Windows loop does.
            if (SDL_HasEvent(SDL_EVENT_QUIT))
                continue;

            StepFrame();
            SDL_Delay(1);
        }
    }
}

#endif
