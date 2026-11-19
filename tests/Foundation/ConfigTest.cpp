#include <Bibim/Config.h>
#include <doctest.h>

TEST_CASE("Config defines BIBIM_64BIT exactly when pointers are 64-bit")
{
#if defined(BIBIM_64BIT)
    CHECK(sizeof(void*) == 8);
#else
    CHECK(sizeof(void*) == 4);
#endif
}
