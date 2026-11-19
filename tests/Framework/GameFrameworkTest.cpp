// GameFramework::Construct runs Startup::All(), which fills the process-wide
// factories, so these cases run in their own CTest entry.
#include <Bibim/Config.h>
#include <Bibim/GameFramework.h>
#include <Bibim/GameWindow.h>
#include <SDL3/SDL.h>
#include <doctest.h>
#include <functional>
#include <string>
#include "../Platform/OffscreenVideo.h"

using namespace Bibim;

namespace
{
    class TestGame : public GameFramework
    {
        public:
            // A runaway loop fails the case instead of hanging the test run.
            static const int MaxFrames = 100;

            std::function<void (TestGame&, int)> onDraw;
            int frames;

            TestGame()
                : GameFramework(64, 48),
                  frames(0)
            {
                SetFixedTimeStep(false);
            }

        protected:
            virtual void Draw()
            {
                frames++;
                if (onDraw)
                    onDraw(*this, frames);
                if (frames == MaxFrames)
                    Exit();
            }
    };

    class DropRecorder : public GameWindow::DropFileEventListener
    {
        public:
            std::string filename;

            virtual void OnWindowFileDropped(Window* /*window*/, const char* filename)
            {
                this->filename = filename;
            }
    };

    void PushWindowEvent(TestGame& game, SDL_EventType type, char* data = nullptr)
    {
        SDL_Window* window = static_cast<SDL_Window*>(game.GetWindow()->GetHandle());
        SDL_Event event = {};
        event.type = type;
        if (type == SDL_EVENT_DROP_FILE)
        {
            event.drop.windowID = SDL_GetWindowID(window);
            event.drop.data = data;
        }
        else
            event.window.windowID = SDL_GetWindowID(window);
        REQUIRE(SDL_PushEvent(&event));
    }
}

TEST_CASE("GameFramework::Run returns after the game calls Exit")
{
    OffscreenVideo video;
    TestGame game;
    game.onDraw = [](TestGame& g, int frame)
    {
        if (frame == 3)
            g.Exit();
    };

    game.Run();
    CHECK(game.frames == 3);
    CHECK(game.GetWindow()->GetHandle() == nullptr);
}

TEST_CASE("GameFramework::Run returns when the window is asked to close")
{
    OffscreenVideo video;
    TestGame game;
    game.onDraw = [](TestGame& g, int frame)
    {
        if (frame == 1)
            PushWindowEvent(g, SDL_EVENT_WINDOW_CLOSE_REQUESTED);
    };

    game.Run();
    CHECK(game.frames == 1);
    CHECK(game.GetWindow()->GetHandle() == nullptr);
}

TEST_CASE("GameFramework::Run passes dropped files to the window")
{
    OffscreenVideo video;
    TestGame game;
    DropRecorder recorder;
    game.GetWindow()->AddDropFileEventListener(&recorder);
    static char filename[] = "/path/to/dropped file.txt";
    game.onDraw = [](TestGame& g, int frame)
    {
        if (frame == 1)
            PushWindowEvent(g, SDL_EVENT_DROP_FILE, filename);
        else if (frame == 2)
            g.Exit();
    };

    game.Run();
    game.GetWindow()->RemoveDropFileEventListener(&recorder);
    CHECK(recorder.filename == filename);
}

TEST_CASE("GameFramework gives the command line to the game as startup args")
{
    OffscreenVideo video;
    const char* argv[] = { "./game", "--sandbox", "two words" };
    GameFramework::SetCommandLine(3, argv);
    TestGame game;
    GameFramework::SetCommandLine(0, nullptr);

    CHECK(game.GetStartupArgs() == "./game --sandbox two words");
}
