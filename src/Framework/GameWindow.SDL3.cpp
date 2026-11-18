#include <Bibim/Config.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/GameWindow.SDL3.h>
#include <Bibim/Assert.h>
#include <Bibim/Log.h>
#include <SDL3/SDL.h>
#include <algorithm>

namespace Bibim
{
    static const Point2 DefaultSize = Point2(800, 600);

    static SDL_Window* ToSDLWindow(void* handle)
    {
        return static_cast<SDL_Window*>(handle);
    }

    GameWindow::GameWindow()
        : handle(nullptr),
          visible(false)
    {
    }

    GameWindow::~GameWindow()
    {
        Close();
        BBAssert(dropFileEventListeners.empty());
    }

    void GameWindow::MoveToScreenCenter()
    {
        if (handle == nullptr)
            CreateHandle();

        SDL_SetWindowPosition(ToSDLWindow(handle), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    void GameWindow::Close()
    {
        if (handle == nullptr)
            return;

        OnDestroy();
        SDL_DestroyWindow(ToSDLWindow(handle));
        handle = nullptr;
        visible = false;

        // Like PostQuitMessage on Windows: the main loop ends. The event stays
        // queued only while someone else (the GameFramework) holds the video
        // subsystem.
        SDL_Event event = {};
        event.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&event);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    const String& GameWindow::GetTitle() const
    {
        return title;
    }

    void GameWindow::SetTitle(const String& value)
    {
        if (handle == nullptr)
            CreateHandle();

        if (title != value)
        {
            title = value;
            SDL_SetWindowTitle(ToSDLWindow(handle), title.CStr());
        }
    }

    Point2 GameWindow::GetPosition() const
    {
        return position;
    }

    void GameWindow::SetPosition(Point2 value)
    {
        if (handle == nullptr)
            CreateHandle();

        if (position != value)
        {
            position = value;
            SDL_SetWindowPosition(ToSDLWindow(handle), position.X, position.Y);
        }
    }

    Point2 GameWindow::GetSize() const
    {
        return size;
    }

    void GameWindow::SetSize(Point2 value)
    {
        if (handle == nullptr)
        {
            size = value;
            CreateHandle();
            return;
        }

        if (size != value)
        {
            size = value;
            SDL_SetWindowSize(ToSDLWindow(handle), size.X, size.Y);
        }
    }

    bool GameWindow::GetVisible() const
    {
        return visible;
    }

    void GameWindow::SetVisible(bool value)
    {
        if (handle == nullptr)
            CreateHandle();

        if (visible != value)
        {
            visible = value;
            if (visible)
                SDL_ShowWindow(ToSDLWindow(handle));
            else
                SDL_HideWindow(ToSDLWindow(handle));
        }
    }

    bool GameWindow::GetFullscreenStyle() const
    {
        return handle && (SDL_GetWindowFlags(ToSDLWindow(handle)) & SDL_WINDOW_FULLSCREEN) != 0;
    }

    void GameWindow::SetFullscreenStyle(bool value)
    {
        if (handle == nullptr)
            CreateHandle();

        SDL_SetWindowFullscreen(ToSDLWindow(handle), value);
    }

    bool GameWindow::IsForeground() const
    {
        return handle && (SDL_GetWindowFlags(ToSDLWindow(handle)) & SDL_WINDOW_INPUT_FOCUS) != 0;
    }

    void* GameWindow::GetHandle() const
    {
        return handle;
    }

    void* GameWindow::GetDisplayHandle() const
    {
        return nullptr;
    }

    void GameWindow::AddDropFileEventListener(DropFileEventListener* listener)
    {
        if (std::find(dropFileEventListeners.begin(), dropFileEventListeners.end(), listener) == dropFileEventListeners.end())
            dropFileEventListeners.push_back(listener);
    }

    void GameWindow::RemoveDropFileEventListener(DropFileEventListener* listener)
    {
        std::vector<DropFileEventListener*>::iterator it = std::find(dropFileEventListeners.begin(),
                                                                     dropFileEventListeners.end(),
                                                                     listener);
        if (it != dropFileEventListeners.end())
            dropFileEventListeners.erase(it);
    }

    void GameWindow::RaiseDropFileEvent(const char* filename)
    {
        for (std::vector<DropFileEventListener*>::iterator it = dropFileEventListeners.begin(); it != dropFileEventListeners.end(); ++it)
            (*it)->OnWindowFileDropped(this, filename);
    }

    void GameWindow::OnCreated()
    {
    }

    void GameWindow::OnDestroy()
    {
    }

    void GameWindow::OnSnapShot()
    {
    }

    bool GameWindow::OnCommand(int /*commandID*/, int /*controlID*/, void* /*handle*/)
    {
        return false;
    }

    bool GameWindow::OnPaint()
    {
        return false;
    }

    void GameWindow::CreateHandle()
    {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) == false)
        {
            Log::Error("GameWindow", SDL_GetError());
            return;
        }

        // GL attributes apply to windows created after they are set, and the
        // EGL config is chosen with the window. Same as the EGL code path in
        // GraphicsDevice.GLES2: RGBA 8-8-8-8 and a 16-bit depth buffer.
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

        if (size.X <= 0 || size.Y <= 0)
            size = DefaultSize;

        SDL_Window* window = SDL_CreateWindow(title.CStr(), size.X, size.Y,
                                              SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
        if (window == nullptr)
        {
            Log::Error("GameWindow", SDL_GetError());
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
            return;
        }

        handle = window;
        int x = 0;
        int y = 0;
        SDL_GetWindowPosition(window, &x, &y);
        position = Point2(x, y);

        OnCreated();
    }
}

#endif
