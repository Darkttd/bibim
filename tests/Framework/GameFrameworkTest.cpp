// GameFramework::Construct runs Startup::All(), which fills the process-wide
// factories, so these cases run in their own CTest entry.
#include <Bibim/Config.h>
#include <Bibim/GameFramework.h>
#include <Bibim/GameWindow.h>
#include <Bibim/Keyboard.h>
#include <Bibim/KeyboardState.h>
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

    // Pushes the events of one frame and records the device state seen in the next.
    template <typename Device, typename State>
    void RunInputFrames(TestGame& game, Device& device,
                        const std::vector<std::function<void (TestGame&)>>& frames,
                        std::vector<State>& states)
    {
        game.onDraw = [&](TestGame& g, int frame)
        {
            if (frame > 1)
                states.push_back(device.GetState());
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
    RunInputFrames(game, mouse, {
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
    RunInputFrames(game, mouse, {
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
    RunInputFrames(game, mouse, {
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
    RunInputFrames(game, mouse, {
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

namespace
{
    void PushKey(TestGame& game, SDL_Keycode key, bool down, bool repeat = false)
    {
        SDL_Event event = {};
        event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        event.key.windowID = SDL_GetWindowID(static_cast<SDL_Window*>(game.GetWindow()->GetHandle()));
        event.key.key = key;
        event.key.down = down;
        event.key.repeat = repeat;
        REQUIRE(SDL_PushEvent(&event));
    }

    int CountPressedKeys(const KeyboardState& state)
    {
        int count = 0;
        for (int i = 0; i < Key::Count; i++)
        {
            if (state.IsPressed(static_cast<Key::Code>(i)))
                count++;
        }
        return count;
    }
}

TEST_CASE("GameFramework::Run passes key presses and releases to the window")
{
    OffscreenVideo video;
    TestGame game;
    Keyboard keyboard(game.GetWindow());
    std::vector<KeyboardState> states;
    RunInputFrames(game, keyboard, {
        [](TestGame& g) { PushKey(g, SDLK_A, true); },
        [](TestGame& g) { PushKey(g, SDLK_LSHIFT, true); },
        [](TestGame& g) { PushKey(g, SDLK_A, false); },
        [](TestGame& g) { PushKey(g, SDLK_LSHIFT, false); },
    }, states);

    REQUIRE(states.size() == 4);
    CHECK(states[0].IsPressed(Key::A));
    CHECK(CountPressedKeys(states[0]) == 1);
    CHECK(states[1].IsPressed(Key::A));
    CHECK(states[1].IsPressed(Key::Shift));
    CHECK(CountPressedKeys(states[1]) == 2);
    CHECK(states[2].IsPressed(Key::Shift));
    CHECK(CountPressedKeys(states[2]) == 1);
    CHECK_FALSE(states[3].IsPressed());
}

TEST_CASE("GameFramework::Run skips auto-repeated key presses")
{
    OffscreenVideo video;
    TestGame game;
    Keyboard keyboard(game.GetWindow());
    std::vector<KeyboardState> states;
    RunInputFrames(game, keyboard, {
        [](TestGame& g) { PushKey(g, SDLK_A, true); },
        // A repeat that SDL delivers after the release.
        [](TestGame& g) { PushKey(g, SDLK_A, false); PushKey(g, SDLK_A, true, true); },
    }, states);

    REQUIRE(states.size() == 2);
    CHECK(states[0].IsPressed(Key::A));
    CHECK_FALSE(states[1].IsPressed());
}

TEST_CASE("GameFramework::Run maps SDL keycodes to Windows virtual-key codes")
{
    struct Mapping
    {
        SDL_Keycode key;
        int code;
    };
    static const Mapping mappings[] =
    {
        { SDLK_A, 'A' }, { SDLK_Z, 'Z' }, { SDLK_0, '0' }, { SDLK_9, '9' },
        { SDLK_F1, 0x70 }, { SDLK_F12, 0x7B }, { SDLK_F13, 0x7C }, { SDLK_F24, 0x87 },
        { SDLK_KP_0, 0x60 }, { SDLK_KP_1, 0x61 }, { SDLK_KP_9, 0x69 },
        { SDLK_KP_MULTIPLY, 0x6A }, { SDLK_KP_PLUS, 0x6B }, { SDLK_KP_MINUS, 0x6D },
        { SDLK_KP_PERIOD, 0x6E }, { SDLK_KP_DIVIDE, 0x6F },
        { SDLK_BACKSPACE, 0x08 }, { SDLK_TAB, 0x09 }, { SDLK_RETURN, 0x0D },
        { SDLK_LSHIFT, 0x10 }, { SDLK_LCTRL, 0x11 }, { SDLK_LALT, 0x12 },
        { SDLK_PAUSE, 0x13 }, { SDLK_CAPSLOCK, 0x14 }, { SDLK_ESCAPE, 0x1B }, { SDLK_SPACE, 0x20 },
        { SDLK_PAGEUP, 0x21 }, { SDLK_PAGEDOWN, 0x22 }, { SDLK_END, 0x23 }, { SDLK_HOME, 0x24 },
        { SDLK_LEFT, 0x25 }, { SDLK_UP, 0x26 }, { SDLK_RIGHT, 0x27 }, { SDLK_DOWN, 0x28 },
        { SDLK_PRINTSCREEN, 0x2C }, { SDLK_INSERT, 0x2D }, { SDLK_DELETE, 0x2E },
        { SDLK_LGUI, 0x5B }, { SDLK_RGUI, 0x5C }, { SDLK_APPLICATION, 0x5D },
        { SDLK_NUMLOCKCLEAR, 0x90 }, { SDLK_SCROLLLOCK, 0x91 },
        { SDLK_SEMICOLON, 0xBA }, { SDLK_EQUALS, 0xBB }, { SDLK_COMMA, 0xBC }, { SDLK_MINUS, 0xBD },
        { SDLK_PERIOD, 0xBE }, { SDLK_SLASH, 0xBF }, { SDLK_GRAVE, 0xC0 },
        { SDLK_LEFTBRACKET, 0xDB }, { SDLK_BACKSLASH, 0xDC }, { SDLK_RIGHTBRACKET, 0xDD },
        { SDLK_APOSTROPHE, 0xDE },
    };
    static const int count = static_cast<int>(sizeof(mappings) / sizeof(mappings[0]));

    OffscreenVideo video;
    TestGame game;
    Keyboard keyboard(game.GetWindow());
    std::vector<KeyboardState> states;
    // Each frame releases the previous key and presses the next one.
    std::vector<std::function<void (TestGame&)>> frames;
    for (int i = 0; i <= count; i++)
    {
        frames.push_back([i](TestGame& g)
        {
            if (i > 0)
                PushKey(g, mappings[i - 1].key, false);
            if (i < count)
                PushKey(g, mappings[i].key, true);
        });
    }
    RunInputFrames(game, keyboard, frames, states);

    REQUIRE(states.size() == static_cast<size_t>(count + 1));
    for (int i = 0; i < count; i++)
    {
        CAPTURE(SDL_GetKeyName(mappings[i].key));
        CHECK(states[i].IsPressed(static_cast<Key::Code>(mappings[i].code)));
        CHECK(CountPressedKeys(states[i]) == 1);
    }
    CHECK_FALSE(states[count].IsPressed());
}

TEST_CASE("GameFramework::Run maps both left and right modifiers and both Enter keys to one code")
{
    OffscreenVideo video;
    TestGame game;
    Keyboard keyboard(game.GetWindow());
    std::vector<KeyboardState> states;
    RunInputFrames(game, keyboard, {
        [](TestGame& g) { PushKey(g, SDLK_RSHIFT, true); PushKey(g, SDLK_RCTRL, true);
                          PushKey(g, SDLK_RALT, true); PushKey(g, SDLK_KP_ENTER, true); },
    }, states);

    REQUIRE(states.size() == 1);
    CHECK(states[0].IsPressed(Key::Shift));
    CHECK(states[0].IsPressed(Key::Ctrl));
    CHECK(states[0].IsPressed(Key::Alt));
    CHECK(states[0].IsPressed(Key::Enter));
    CHECK(CountPressedKeys(states[0]) == 4);
}

TEST_CASE("GameFramework::Run ignores keys without a virtual-key code")
{
    OffscreenVideo video;
    TestGame game;
    Keyboard keyboard(game.GetWindow());
    std::vector<KeyboardState> states;
    RunInputFrames(game, keyboard, {
        [](TestGame& g) { PushKey(g, SDLK_UNKNOWN, true); PushKey(g, SDLK_MODE, true); PushKey(g, SDLK_A, true); },
    }, states);

    REQUIRE(states.size() == 1);
    CHECK(states[0].IsPressed(Key::A));
    CHECK(CountPressedKeys(states[0]) == 1);
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
