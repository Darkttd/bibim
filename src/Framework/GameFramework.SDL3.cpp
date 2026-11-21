#include <Bibim/Config.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/GameFramework.SDL3.h>
#include <Bibim/GameWindow.h>
#include <Bibim/Log.h>
#include <SDL3/SDL.h>

namespace Bibim
{
    static String& CommandLine()
    {
        static String value;
        return value;
    }

    static void RaiseMouseButtonEvent(Window* window, const SDL_MouseButtonEvent& event)
    {
        const int x = static_cast<int>(event.x);
        const int y = static_cast<int>(event.y);
        switch (event.button)
        {
            case SDL_BUTTON_LEFT:
                if (event.down)
                    window->RaiseMouseLeftButtonDownEvent(x, y);
                else
                    window->RaiseMouseLeftButtonUpEvent(x, y);
                break;
            case SDL_BUTTON_MIDDLE:
                if (event.down)
                    window->RaiseMouseMiddleButtonDownEvent(x, y);
                else
                    window->RaiseMouseMiddleButtonUpEvent(x, y);
                break;
            case SDL_BUTTON_RIGHT:
                if (event.down)
                    window->RaiseMouseRightButtonDownEvent(x, y);
                else
                    window->RaiseMouseRightButtonUpEvent(x, y);
                break;
        }
    }

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
        String value;
        for (int i = 0; i < argc; i++)
        {
            if (i > 0)
                value += ' ';
            value += argv[i];
        }
        CommandLine() = value;
    }

    void GameFramework::Construct(int width, int height)
    {
        // The quit event that GameWindow::Close pushes stays queued only while
        // the video subsystem is held, so it is held until the game is deleted.
        videoInitialized = SDL_InitSubSystem(SDL_INIT_VIDEO);
        if (videoInitialized == false)
            Log::Error("GameFramework", SDL_GetError());

        GameFrameworkBase::Construct(width, height, CommandLine());
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
                    case SDL_EVENT_MOUSE_MOTION:
                        GetWindow()->RaiseMouseMoveEvent(static_cast<int>(event.motion.x),
                                                         static_cast<int>(event.motion.y));
                        break;
                    case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    case SDL_EVENT_MOUSE_BUTTON_UP:
                        RaiseMouseButtonEvent(GetWindow(), event.button);
                        break;
                    case SDL_EVENT_MOUSE_WHEEL:
                        // Whole notches, like WHEEL_DELTA units on Windows.
                        if (event.wheel.integer_y != 0)
                            GetWindow()->RaiseMouseWheelEvent(event.wheel.integer_y);
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
