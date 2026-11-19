// Config.h decides BIBIM_64BIT / BIBIM_32BIT from compiler macros; check that
// the decision matches the actual pointer size of the target.
#include <Bibim/Config.h>
#include <doctest.h>

#if (defined(BIBIM_64BIT) == defined(BIBIM_32BIT))
#    error "Exactly one of BIBIM_64BIT and BIBIM_32BIT must be defined"
#endif

#if (defined(BIBIM_64BIT))
static_assert(sizeof(void*) == 8, "BIBIM_64BIT is defined but pointers are not 8 bytes");
#else
static_assert(sizeof(void*) == 4, "BIBIM_32BIT is defined but pointers are not 4 bytes");
#endif

TEST_CASE("Config bitness matches the pointer size")
{
#if (defined(BIBIM_64BIT))
    CHECK(sizeof(void*) == 8);
#else
    CHECK(sizeof(void*) == 4);
#endif
}
