#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include <doctest.h>

// FontCache's FT_Get_Glyph copies a space's empty outline; FT_Outline_Copy
// then passes null arrays to memcpy with a size of 0 (UBSan nonnull-attribute).
TEST_CASE("FreeType copies an outline with no points")
{
    FT_Library library = nullptr;
    REQUIRE(FT_Init_FreeType(&library) == 0);

    FT_Outline source;
    FT_Outline target;
    REQUIRE(FT_Outline_New(library, 0, 0, &source) == 0);
    REQUIRE(FT_Outline_New(library, 0, 0, &target) == 0);
    CHECK(FT_Outline_Copy(&source, &target) == 0);

    FT_Outline_Done(library, &target);
    FT_Outline_Done(library, &source);
    FT_Done_FreeType(library);
}
