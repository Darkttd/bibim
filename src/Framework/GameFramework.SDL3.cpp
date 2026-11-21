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

    // Key::Code holds Windows virtual-key codes. SDL keycodes follow the
    // keyboard layout as virtual-key codes do; on non-Latin layouts SDL still
    // reports Latin letters, and Windows gives the Latin virtual keys too.
    static Key::Code ToKeyCode(SDL_Keycode key)
    {
        if (key >= SDLK_A && key <= SDLK_Z)
            return static_cast<Key::Code>(Key::A + (key - SDLK_A));
        if (key >= SDLK_0 && key <= SDLK_9)
            return static_cast<Key::Code>(Key::D0 + (key - SDLK_0));
        if (key >= SDLK_F1 && key <= SDLK_F12)
            return static_cast<Key::Code>(Key::F1 + (key - SDLK_F1));
        if (key >= SDLK_F13 && key <= SDLK_F24)
            return static_cast<Key::Code>(0x7C + (key - SDLK_F13));
        if (key >= SDLK_KP_1 && key <= SDLK_KP_9)
            return static_cast<Key::Code>(0x61 + (key - SDLK_KP_1));

        switch (key)
        {
            case SDLK_BACKSPACE:    return Key::Back;
            case SDLK_TAB:          return static_cast<Key::Code>(0x09);
            case SDLK_RETURN:       return Key::Enter;
            case SDLK_KP_ENTER:     return Key::Enter;
            case SDLK_LSHIFT:       return Key::Shift;
            case SDLK_RSHIFT:       return Key::Shift;
            case SDLK_LCTRL:        return Key::Ctrl;
            case SDLK_RCTRL:        return Key::Ctrl;
            case SDLK_LALT:         return Key::Alt;
            case SDLK_RALT:         return Key::Alt;
            case SDLK_PAUSE:        return static_cast<Key::Code>(0x13);
            case SDLK_CAPSLOCK:     return static_cast<Key::Code>(0x14);
            case SDLK_ESCAPE:       return Key::Escape;
            case SDLK_SPACE:        return Key::Space;
            case SDLK_PAGEUP:       return static_cast<Key::Code>(0x21);
            case SDLK_PAGEDOWN:     return static_cast<Key::Code>(0x22);
            case SDLK_END:          return Key::End;
            case SDLK_HOME:         return Key::Home;
            case SDLK_LEFT:         return Key::Left;
            case SDLK_UP:           return Key::Up;
            case SDLK_RIGHT:        return Key::Right;
            case SDLK_DOWN:         return Key::Down;
            case SDLK_PRINTSCREEN:  return static_cast<Key::Code>(0x2C);
            case SDLK_INSERT:       return Key::Insert;
            case SDLK_DELETE:       return Key::Delete;
            case SDLK_LGUI:         return static_cast<Key::Code>(0x5B);
            case SDLK_RGUI:         return static_cast<Key::Code>(0x5C);
            case SDLK_APPLICATION:  return static_cast<Key::Code>(0x5D);
            case SDLK_KP_0:         return static_cast<Key::Code>(0x60);
            case SDLK_KP_MULTIPLY:  return static_cast<Key::Code>(0x6A);
            case SDLK_KP_PLUS:      return static_cast<Key::Code>(0x6B);
            case SDLK_KP_MINUS:     return static_cast<Key::Code>(0x6D);
            case SDLK_KP_PERIOD:    return static_cast<Key::Code>(0x6E);
            case SDLK_KP_DIVIDE:    return static_cast<Key::Code>(0x6F);
            case SDLK_NUMLOCKCLEAR: return static_cast<Key::Code>(0x90);
            case SDLK_SCROLLLOCK:   return static_cast<Key::Code>(0x91);
            case SDLK_SEMICOLON:    return static_cast<Key::Code>(0xBA);
            case SDLK_EQUALS:       return static_cast<Key::Code>(0xBB);
            case SDLK_COMMA:        return static_cast<Key::Code>(0xBC);
            case SDLK_MINUS:        return static_cast<Key::Code>(0xBD);
            case SDLK_PERIOD:       return static_cast<Key::Code>(0xBE);
            case SDLK_SLASH:        return static_cast<Key::Code>(0xBF);
            case SDLK_GRAVE:        return static_cast<Key::Code>(0xC0);
            case SDLK_LEFTBRACKET:  return static_cast<Key::Code>(0xDB);
            case SDLK_BACKSLASH:    return static_cast<Key::Code>(0xDC);
            case SDLK_RIGHTBRACKET: return static_cast<Key::Code>(0xDD);
            case SDLK_APOSTROPHE:   return static_cast<Key::Code>(0xDE);
            default:                return Key::None;
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
                    case SDL_EVENT_KEY_DOWN:
                        // Auto-repeat is skipped, as WM_KEYDOWN with the
                        // previous-state bit set is on Windows.
                        if (event.key.repeat == false)
                        {
                            const Key::Code code = ToKeyCode(event.key.key);
                            if (code != Key::None)
                                GetWindow()->RaiseKeyDownEvent(code);
                        }
                        break;
                    case SDL_EVENT_KEY_UP:
                        {
                            const Key::Code code = ToKeyCode(event.key.key);
                            if (code != Key::None)
                                GetWindow()->RaiseKeyUpEvent(code);
                        }
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
