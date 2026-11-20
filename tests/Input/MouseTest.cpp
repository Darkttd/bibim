// Tests for the SDL3 Mouse, run through the offscreen video driver.
#include <Bibim/Config.h>
#include <Bibim/GameModuleNode.h>
#include <Bibim/GameModuleTree.h>
#include <Bibim/GameWindow.h>
#include <Bibim/Mouse.h>
#include <SDL3/SDL.h>
#include <doctest.h>
#include "Platform/OffscreenVideo.h"

using namespace Bibim;

TEST_CASE("Mouse::SetVisible hides and shows the SDL cursor")
{
    OffscreenVideo video;
    Mouse mouse;
    CHECK(mouse.GetState().IsVisible);

    mouse.Hide();
    CHECK_FALSE(SDL_CursorVisible());
    CHECK_FALSE(mouse.GetState().IsVisible);

    mouse.Show();
    CHECK(SDL_CursorVisible());
    CHECK(mouse.GetState().IsVisible);
}

TEST_CASE("Mouse::SetPosition without a window does nothing")
{
    OffscreenVideo video;
    Mouse mouse;
    mouse.SetPosition(Point2(10, 20));
    CHECK(mouse.GetState().Position == Point2::Zero);
}

TEST_CASE("Mouse::SetPosition warps the cursor inside its window")
{
    OffscreenVideo video;
    GameModuleTree modules;
    GameWindow* window = new GameWindow();
    modules.GetRoot()->AttachChild(window);
    window->SetSize(Point2(64, 64));
    REQUIRE(window->GetHandle() != nullptr);

    Mouse mouse(window);
    mouse.SetPosition(Point2(10, 20));
    SDL_PumpEvents();
    float x = 0.0f;
    float y = 0.0f;
    SDL_GetMouseState(&x, &y);
    CHECK(x == 10.0f);
    CHECK(y == 20.0f);
}
