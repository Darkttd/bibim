#include <Bibim/Config.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/Mouse.h>
#include <Bibim/Window.h>
#include <SDL3/SDL.h>

namespace Bibim
{
    void Mouse::SetPosition(Point2 value)
    {
        if (window == nullptr || window->GetHandle() == nullptr)
            return;

        SDL_WarpMouseInWindow(static_cast<SDL_Window*>(window->GetHandle()),
                              static_cast<float>(value.X), static_cast<float>(value.Y));
    }

    void Mouse::SetVisible(bool value)
    {
        if (isVisible != value)
        {
            isVisible = value;
            if (value)
                SDL_ShowCursor();
            else
                SDL_HideCursor();

            state.IsVisible = SDL_CursorVisible();
        }
    }
}

#endif
