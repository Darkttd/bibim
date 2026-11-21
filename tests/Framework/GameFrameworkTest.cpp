// GameFramework::Construct runs Startup::All(), which fills the process-wide
// factories, so these cases run in their own CTest entry.
#include <Bibim/Config.h>
#include <Bibim/GameFramework.h>
#include <Bibim/GameWindow.h>
#include <Bibim/Mouse.h>
#include <SDL3/SDL.h>
#include <doctest.h>
#include <functional>
#include <vector>
#include "Platform/OffscreenVideo.h"

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
            String filename;

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

namespace
{
    void PushMouseMotion(TestGame& game, float x, float y)
    {
        SDL_Event event = {};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.windowID = SDL_GetWindowID(static_cast<SDL_Window*>(game.GetWindow()->GetHandle()));
        event.motion.x = x;
        event.motion.y = y;
        REQUIRE(SDL_PushEvent(&event));
    }

    void PushMouseButton(TestGame& game, Uint8 button, bool down, float x, float y)
    {
        SDL_Event event = {};
        event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
        event.button.windowID = SDL_GetWindowID(static_cast<SDL_Window*>(game.GetWindow()->GetHandle()));
        event.button.button = button;
        event.button.down = down;
        event.button.x = x;
        event.button.y = y;
        REQUIRE(SDL_PushEvent(&event));
    }

    void PushMouseWheel(TestGame& game, float y, Sint32 integerY)
    {
        SDL_Event event = {};
        event.type = SDL_EVENT_MOUSE_WHEEL;
        event.wheel.windowID = SDL_GetWindowID(static_cast<SDL_Window*>(game.GetWindow()->GetHandle()));
        event.wheel.y = y;
        event.wheel.integer_y = integerY;
        REQUIRE(SDL_PushEvent(&event));
    }

    // Pushes the events of one frame and records the mouse state seen in the next.
    void RunMouseFrames(TestGame& game, Mouse& mouse,
                        const std::vector<std::function<void (TestGame&)>>& frames,
                        std::vector<MouseState>& states)
    {
        game.onDraw = [&](TestGame& g, int frame)
        {
            if (frame > 1)
                states.push_back(mouse.GetState());
            if (frame <= static_cast<int>(frames.size()))
                frames[frame - 1](g);
            else
                g.Exit();
        };
        game.Run();
    }
}

TEST_CASE("GameFramework::Run passes mouse motion to the window")
{
    OffscreenVideo video;
    TestGame game;
    Mouse mouse(game.GetWindow());
    std::vector<MouseState> states;
    RunMouseFrames(game, mouse, {
        [](TestGame& g) { PushMouseMotion(g, 12.75f, 34.25f); },
    }, states);

    REQUIRE(states.size() == 1);
    CHECK(states[0].Position == Point2(12, 34));
    CHECK_FALSE(states[0].IsLeftButtonPressed);
}

TEST_CASE("GameFramework::Run passes the left, middle and right buttons to the window")
{
    OffscreenVideo video;
    TestGame game;
    Mouse mouse(game.GetWindow());
    std::vector<MouseState> states;
    RunMouseFrames(game, mouse, {
        [](TestGame& g) { PushMouseButton(g, SDL_BUTTON_LEFT, true, 1.0f, 2.0f); },
        [](TestGame& g) { PushMouseButton(g, SDL_BUTTON_MIDDLE, true, 3.0f, 4.0f); },
        [](TestGame& g) { PushMouseButton(g, SDL_BUTTON_RIGHT, true, 5.0f, 6.0f); },
        [](TestGame& g) { PushMouseButton(g, SDL_BUTTON_LEFT, false, 7.0f, 8.0f); },
        [](TestGame& g) { PushMouseButton(g, SDL_BUTTON_MIDDLE, false, 9.0f, 10.0f); },
        [](TestGame& g) { PushMouseButton(g, SDL_BUTTON_RIGHT, false, 11.0f, 12.0f); },
    }, states);

    REQUIRE(states.size() == 6);
    CHECK(states[0] == MouseState(Point2(1, 2), 0, true, false, false, true));
    CHECK(states[1] == MouseState(Point2(3, 4), 0, true, false, true, true));
    CHECK(states[2] == MouseState(Point2(5, 6), 0, true, true, true, true));
    CHECK(states[3] == MouseState(Point2(7, 8), 0, false, true, true, true));
    CHECK(states[4] == MouseState(Point2(9, 10), 0, false, true, false, true));
    CHECK(states[5] == MouseState(Point2(11, 12), 0, false, false, false, true));
}

TEST_CASE("GameFramework::Run ignores the side buttons")
{
    OffscreenVideo video;
    TestGame game;
    Mouse mouse(game.GetWindow());
    std::vector<MouseState> states;
    RunMouseFrames(game, mouse, {
        [](TestGame& g) { PushMouseButton(g, SDL_BUTTON_X1, true, 1.0f, 2.0f); },
    }, states);

    REQUIRE(states.size() == 1);
    CHECK(states[0] == MouseState());
}

TEST_CASE("GameFramework::Run adds whole wheel notches to the mouse")
{
    OffscreenVideo video;
    TestGame game;
    Mouse mouse(game.GetWindow());
    std::vector<MouseState> states;
    RunMouseFrames(game, mouse, {
        [](TestGame& g) { PushMouseWheel(g, 1.0f, 1); },
        [](TestGame& g) { PushMouseWheel(g, -2.0f, -2); },
        // A fraction of a notch from a smooth-scrolling device.
        [](TestGame& g) { PushMouseWheel(g, 0.25f, 0); },
    }, states);

    REQUIRE(states.size() == 3);
    CHECK(states[0].Wheel == 1);
    CHECK(states[1].Wheel == -1);
    CHECK(states[2].Wheel == -1);
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
