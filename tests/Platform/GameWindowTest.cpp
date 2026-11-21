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
#include <string>
#include "Platform/OffscreenVideo.h"

using namespace Bibim;

namespace
{
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

namespace
{
    class ResizeCounter : public Window::ResizeEventListener
    {
        public:
            int count;
            Point2 lastSize;

            ResizeCounter()
                : count(0)
            {
            }

            virtual void OnWindowResized(Window* window)
            {
                count++;
                lastSize = window->GetSize();
            }
    };

    class DeviceEventCounter : public GraphicsDeviceBase::LostEventListener,
                               public GraphicsDeviceBase::RestoreEventListener
    {
        public:
            int lost;
            int restored;

            DeviceEventCounter()
                : lost(0),
                  restored(0)
            {
            }

            virtual void OnGraphicsDeviceLost(GraphicsDeviceBase* /*g*/)
            {
                lost++;
            }

            virtual void OnGraphicsDeviceRestore(GraphicsDeviceBase* /*g*/)
            {
                restored++;
            }
    };

    Point2 GetSDLWindowSize(GameWindow* window)
    {
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(static_cast<SDL_Window*>(window->GetHandle()), &width, &height);
        return Point2(width, height);
    }
}

TEST_CASE("GameWindow::SetSize resizes a created window and raises the resized event")
{
    OffscreenVideo video;
    GameModuleTree modules;
    GameWindow* window = new GameWindow();
    modules.GetRoot()->AttachChild(window);
    window->SetSize(Point2(64, 48));
    REQUIRE(window->GetHandle() != nullptr);

    ResizeCounter counter;
    window->AddResizeEventListener(&counter);
    window->SetSize(Point2(128, 96));
    CHECK(window->GetSize() == Point2(128, 96));
    CHECK(GetSDLWindowSize(window) == Point2(128, 96));
    CHECK(counter.count == 1);
    CHECK(counter.lastSize == Point2(128, 96));

    window->SetSize(Point2(128, 96));
    CHECK(counter.count == 1);
    window->RemoveResizeEventListener(&counter);
}

TEST_CASE("GameWindow::SetFullscreenStyle takes the size of the display and gives it back")
{
    OffscreenVideo video;
    GameModuleTree modules;
    GameWindow* window = new GameWindow();
    modules.GetRoot()->AttachChild(window);
    window->SetSize(Point2(64, 48));
    window->SetVisible(true);
    REQUIRE(window->GetHandle() != nullptr);

    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
    REQUIRE(mode != nullptr);
    REQUIRE(Point2(mode->w, mode->h) != Point2(64, 48));

    ResizeCounter counter;
    window->AddResizeEventListener(&counter);
    window->SetFullscreenStyle(true);
    CHECK(window->GetFullscreenStyle());
    CHECK(window->GetSize() == Point2(mode->w, mode->h));
    CHECK(counter.count == 1);

    window->SetFullscreenStyle(false);
    CHECK_FALSE(window->GetFullscreenStyle());
    CHECK(window->GetSize() == Point2(64, 48));
    CHECK(counter.count == 2);
    window->RemoveResizeEventListener(&counter);
}

TEST_CASE("GraphicsDevice follows a window resize with the viewport and keeps its GL context")
{
    OffscreenVideo video;
    GameModuleTree modules;
    GameWindow* window = new GameWindow();
    GraphicsDevice* graphicsDevice = new GraphicsDevice();
    modules.GetRoot()->AttachChild(window);
    modules.GetRoot()->AttachChild(graphicsDevice);
    window->SetSize(Point2(64, 32));
    graphicsDevice->SetWindow(window);
    const SDL_GLContext context = SDL_GL_GetCurrentContext();
    REQUIRE_MESSAGE(context != nullptr, SDL_GetError());

    DeviceEventCounter events;
    graphicsDevice->AddLostEventListener(&events);
    graphicsDevice->AddRestoreEventListener(&events);

    window->SetSize(Point2(96, 80));
    graphicsDevice->BeginDraw();
    GLint viewport[4] = { 0, 0, 0, 0 };
    glGetIntegerv(GL_VIEWPORT, viewport);
    CHECK(viewport[2] == 96);
    CHECK(viewport[3] == 80);
    CHECK(SDL_GL_GetCurrentContext() == context);
    CHECK(events.lost == 0);
    CHECK(events.restored == 0);
    graphicsDevice->EndDraw();

    graphicsDevice->RemoveRestoreEventListener(&events);
    graphicsDevice->RemoveLostEventListener(&events);
}
