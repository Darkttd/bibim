// Tests for the SDL3 GameWindow and the GLES2 GraphicsDevice on top of it,
// run without a display through the offscreen video driver (EGL).
#include <Bibim/Config.h>
#include <Bibim/Color.h>
#include <Bibim/GameModuleNode.h>
#include <Bibim/GameModuleTree.h>
#include <Bibim/GameWindow.h>
#include <Bibim/GLES2.h>
#include <Bibim/GraphicsDevice.h>
#include <SDL3/SDL.h>
#include <doctest.h>
#include <dlfcn.h>
#include <link.h>
#include <string>

using namespace Bibim;

namespace
{
    int PinLoadedLibrary(dl_phdr_info* info, size_t, void*)
    {
        if (info->dlpi_name != nullptr && info->dlpi_name[0] != '\0')
            dlopen(info->dlpi_name, RTLD_NOW | RTLD_NOLOAD);
        return 0;
    }

    // SDL unloads the GL driver when its last GL window is destroyed. The
    // driver's one-time allocations then lose their owner and LeakSanitizer
    // reports them, with every driver frame shown as <unknown module>. Taking
    // one more reference to each library loaded for a GL window and context
    // keeps the driver in memory until exit.
    bool PinGLDriver()
    {
        SDL_Window* window = SDL_CreateWindow("", 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (window == nullptr)
            return false;
        SDL_GLContext context = SDL_GL_CreateContext(window);
        dl_iterate_phdr(PinLoadedLibrary, nullptr);
        if (context != nullptr)
            SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        return context != nullptr;
    }

    // Holds the video subsystem for the whole case, as the GameFramework does.
    struct OffscreenVideo
    {
        OffscreenVideo()
        {
            REQUIRE(SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen"));
            REQUIRE_MESSAGE(SDL_InitSubSystem(SDL_INIT_VIDEO), SDL_GetError());

            static const bool pinned = PinGLDriver();
            REQUIRE_MESSAGE(pinned, SDL_GetError());
        }

        ~OffscreenVideo()
        {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
        }
    };

    int CountSDLWindows()
    {
        int count = -1;
        SDL_free(SDL_GetWindows(&count));
        return count;
    }
}

TEST_CASE("GameWindow creates an SDL window with the given title and size")
{
    OffscreenVideo video;
    GameModuleTree modules;
    GameWindow* window = new GameWindow();
    modules.GetRoot()->AttachChild(window);

    window->SetSize(Point2(320, 240));
    window->SetTitle("Bibim");
    REQUIRE(window->GetHandle() != nullptr);

    SDL_Window* sdlWindow = static_cast<SDL_Window*>(window->GetHandle());
    CHECK(window->GetSize() == Point2(320, 240));
    CHECK(window->GetTitle() == "Bibim");
    CHECK(SDL_GetWindowTitle(sdlWindow) == std::string("Bibim"));
    int width = 0;
    int height = 0;
    CHECK(SDL_GetWindowSize(sdlWindow, &width, &height));
    CHECK(width == 320);
    CHECK(height == 240);

    CHECK_FALSE(window->GetVisible());
    window->SetVisible(true);
    CHECK(window->GetVisible());
}

TEST_CASE("GameWindow::Close destroys the window and asks the main loop to quit")
{
    OffscreenVideo video;
    GameModuleTree modules;
    GameWindow* window = new GameWindow();
    modules.GetRoot()->AttachChild(window);
    window->SetSize(Point2(64, 64));
    REQUIRE(window->GetHandle() != nullptr);

    SDL_FlushEvent(SDL_EVENT_QUIT);
    window->Close();
    CHECK(window->GetHandle() == nullptr);
    CHECK(CountSDLWindows() == 0);
    CHECK(SDL_HasEvent(SDL_EVENT_QUIT));
}

TEST_CASE("GameWindow destroys its SDL window when it is deleted")
{
    OffscreenVideo video;
    {
        GameModuleTree modules;
        GameWindow* window = new GameWindow();
        modules.GetRoot()->AttachChild(window);
        window->SetSize(Point2(64, 64));
        REQUIRE(window->GetHandle() != nullptr);
        REQUIRE(CountSDLWindows() == 1);
    }
    CHECK(CountSDLWindows() == 0);
}

TEST_CASE("GameWindow is not created again after Close")
{
    OffscreenVideo video;
    GameModuleTree modules;
    GameWindow* window = new GameWindow();
    modules.GetRoot()->AttachChild(window);
    window->SetVisible(true);
    REQUIRE(window->GetHandle() != nullptr);
    window->Close();

    // GameFrameworkBase::Finalize hides the window after the main loop ends.
    window->SetVisible(false);
    window->SetTitle("Closed");
    window->SetSize(Point2(16, 16));
    CHECK(window->GetHandle() == nullptr);
    CHECK_FALSE(window->GetVisible());
}

TEST_CASE("GraphicsDevice draws into a GLES2 context on the SDL window")
{
    OffscreenVideo video;
    GameModuleTree modules;
    GameWindow* window = new GameWindow();
    GraphicsDevice* graphicsDevice = new GraphicsDevice();
    modules.GetRoot()->AttachChild(window);
    modules.GetRoot()->AttachChild(graphicsDevice);

    window->SetSize(Point2(64, 32));
    graphicsDevice->SetWindow(window);
    REQUIRE_MESSAGE(SDL_GL_GetCurrentContext() != nullptr, SDL_GetError());

    graphicsDevice->Clear(Color(255, 0, 0, 255));
    unsigned char pixel[4] = { 0, 0, 0, 0 };
    glReadPixels(32, 16, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    CHECK(glGetError() == GL_NO_ERROR);
    CHECK(pixel[0] == 255);
    CHECK(pixel[1] == 0);
    CHECK(pixel[2] == 0);
    CHECK(pixel[3] == 255);

    graphicsDevice->Present();
}
