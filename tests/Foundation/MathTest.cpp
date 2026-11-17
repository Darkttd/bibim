#include <Bibim/Config.h>
#include <Bibim/Math.h>
#include <Bibim/Vector2.h>
#include <Bibim/Vector3.h>
#include <doctest.h>
#include <cmath>

using namespace Bibim;

TEST_CASE("Math::Abs returns the absolute value")
{
    CHECK(Math::Abs(-3) == 3);
    CHECK(Math::Abs(3) == 3);
    CHECK(Math::Abs(-2.5f) == doctest::Approx(2.5f));
}

TEST_CASE("Math::Min and Math::Max pick the smaller and larger value")
{
    CHECK(Math::Min(2, 5) == 2);
    CHECK(Math::Max(2, 5) == 5);
    CHECK(Math::Min(4, 1, 3) == 1);
    CHECK(Math::Max(4, 1, 3) == 4);
}

TEST_CASE("Math::Clamp keeps a value within the range")
{
    CHECK(Math::Clamp(5, 0, 10) == 5);
    CHECK(Math::Clamp(-1, 0, 10) == 0);
    CHECK(Math::Clamp(11, 0, 10) == 10);
    CHECK(Math::Clamp(1.5f) == doctest::Approx(1.0f));
    CHECK(Math::Clamp(-0.5f) == doctest::Approx(0.0f));
}

TEST_CASE("Math::ToRadian and Math::ToDegree are inverse")
{
    CHECK(Math::ToRadian(180.0f) == doctest::Approx(Math::Pi));
    CHECK(Math::ToDegree(Math::PiOver2) == doctest::Approx(90.0f));
}

TEST_CASE("Math pi constants are the nearest float values")
{
    CHECK(Math::Pi == 3.14159265358979323846f);

    // Scaling a float by a power of two is exact, so these must match Pi bit for bit.
    CHECK(Math::PiOver2 == Math::Pi / 2.0f);
    CHECK(Math::PiOver4 == Math::Pi / 4.0f);
    CHECK(Math::TwoPi == Math::Pi * 2.0f);

    // With pi rounded to 1.5708f, the cosine of a quarter turn was about -3.7e-6.
    CHECK(Math::Abs(Math::Cos(Math::PiOver2)) < 1e-7f);
    CHECK(Math::Abs(Math::Sin(Math::Pi)) < 1e-6f);
}

TEST_CASE("Math constants for e, logarithms and square roots are the nearest float values")
{
    // Computed in double and rounded once, so each is the float nearest to the exact value.
    CHECK(Math::E == static_cast<float>(std::exp(1.0)));
    CHECK(Math::Log10E == static_cast<float>(1.0 / std::log(10.0)));
    CHECK(Math::Log2E == static_cast<float>(1.0 / std::log(2.0)));
    CHECK(Math::InvSqrt2 == static_cast<float>(1.0 / std::sqrt(2.0)));
}

TEST_CASE("Math::Floor rounds toward negative infinity")
{
    CHECK(Math::Floor(2.7f) == doctest::Approx(2.0f));
    CHECK(Math::Floor(-2.3f) == doctest::Approx(-3.0f));
}

TEST_CASE("Math::Equals allows a difference below 0.00001")
{
    CHECK(Math::Equals(1.0f, 1.0f));
    CHECK(Math::Equals(1.0f, 1.000005f));
    CHECK_FALSE(Math::Equals(1.0f, 1.0001f));
    CHECK_FALSE(Math::Equals(0.0f, -0.0001f));

    CHECK(Math::Equals(Vector2(1.0f, 2.0f), Vector2(1.000005f, 2.0f)));
    CHECK_FALSE(Math::Equals(Vector2(1.0f, 2.0f), Vector2(1.0f, 2.0001f)));

    CHECK(Math::Equals(Vector3(1.0f, 2.0f, 3.0f), Vector3(1.0f, 2.0f, 3.000002f)));
    CHECK_FALSE(Math::Equals(Vector3(1.0f, 2.0f, 3.0f), Vector3(1.0f, 2.0f, 3.0001f)));
}
