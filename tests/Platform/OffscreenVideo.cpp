#include "OffscreenVideo.h"
#include "PinLoadedLibraries.h"
#include <SDL3/SDL.h>
#include <doctest.h>

namespace
{
    bool PinGLDriver()
    {
        SDL_Window* window = SDL_CreateWindow("", 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (window == nullptr)
            return false;
        SDL_GLContext context = SDL_GL_CreateContext(window);
        PinLoadedLibraries();
        if (context != nullptr)
            SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        return context != nullptr;
    }
}

OffscreenVideo::OffscreenVideo()
{
    REQUIRE(SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen"));
    REQUIRE_MESSAGE(SDL_InitSubSystem(SDL_INIT_VIDEO), SDL_GetError());

    static const bool pinned = PinGLDriver();
    REQUIRE_MESSAGE(pinned, SDL_GetError());
}

OffscreenVideo::~OffscreenVideo()
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
