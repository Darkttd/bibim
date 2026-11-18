// The SDL3 platform layer is tested without a display through the offscreen
// video driver.
#include <Bibim/Config.h>
#include <SDL3/SDL.h>
#include <doctest.h>
#include <string>

TEST_CASE("SDL3 initializes video with the offscreen driver")
{
    REQUIRE(SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen"));
    REQUIRE_MESSAGE(SDL_Init(SDL_INIT_VIDEO), SDL_GetError());
    CHECK(SDL_GetCurrentVideoDriver() == std::string("offscreen"));
    SDL_Quit();
}
