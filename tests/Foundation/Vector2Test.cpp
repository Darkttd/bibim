// Tests for Vector2.
#include <Bibim/Config.h>
#include <Bibim/Math.h>
#include <Bibim/Vector2.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("Vector2 arithmetic works per component")
{
    const Vector2 a(1.0f, 2.0f);
    const Vector2 b(3.0f, -4.0f);
    CHECK(a + b == Vector2(4.0f, -2.0f));
    CHECK(a - b == Vector2(-2.0f, 6.0f));
    CHECK(a * 2.0f == Vector2(2.0f, 4.0f));
    CHECK(2.0f * a == Vector2(2.0f, 4.0f));
    CHECK(b / 2.0f == Vector2(1.5f, -2.0f));
    CHECK(-a == Vector2(-1.0f, -2.0f));
    CHECK(a.Multiply(b) == Vector2(3.0f, -8.0f));
    CHECK(a.Dot(b) == -5.0f);
    CHECK(Vector2(3, 4) == Vector2(3.0f, 4.0f));
}

TEST_CASE("Vector2 length, distance and Normalize")
{
    Vector2 v(3.0f, 4.0f);
    CHECK(v.GetLength() == 5.0f);
    CHECK(v.GetLengthSquared() == 25.0f);
    CHECK(Vector2::GetDistance(Vector2(1.0f, 1.0f), Vector2(4.0f, 5.0f)) == 5.0f);
    CHECK(Vector2::GetDistanceSquared(Vector2(1.0f, 1.0f), Vector2(4.0f, 5.0f)) == 25.0f);

    CHECK(v.Normalize() == 5.0f);
    CHECK(Math::Equals(v, Vector2(0.6f, 0.8f)));

    // A unit vector is left as it is.
    Vector2 unit = Vector2::UnitX;
    CHECK(unit.Normalize() == 1.0f);
    CHECK(unit == Vector2::UnitX);
}

TEST_CASE("Vector2::Rotate turns from +X toward +Y")
{
    Vector2 v = Vector2::UnitX;
    v.Rotate(Math::PiOver2);
    CHECK(Math::Equals(v, Vector2::UnitY));
    v.Rotate(Math::Pi);
    CHECK(Math::Equals(v, Vector2(0.0f, -1.0f)));

    Vector2 w(2.0f, 0.0f);
    w.Rotate(1.0f, 0.0f); // sin, cos of a quarter turn
    CHECK(Math::Equals(w, Vector2(0.0f, 2.0f)));

    CHECK(Vector2(0.0f, 1.0f).ToRadian() == doctest::Approx(Math::PiOver2));
    CHECK(Vector2(-1.0f, 0.0f).ToRadian() == doctest::Approx(Math::Pi));
}

TEST_CASE("Vector2::Reflect bounces off a surface with the given normal")
{
    Vector2 v(1.0f, 1.0f);
    v.Reflect(Vector2(0.0f, -1.0f));
    CHECK(Math::Equals(v, Vector2(1.0f, -1.0f)));

    Vector2 w(1.0f, -2.0f);
    w.Reflect(Vector2(-1.0f, 0.0f));
    CHECK(Math::Equals(w, Vector2(-1.0f, -2.0f)));
}

TEST_CASE("Vector2::IsCollinear")
{
    CHECK(Vector2::IsCollinear(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(3.0f, 3.0f)));
    CHECK(Vector2::IsCollinear(Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(0.0f, 5.0f))); // vertical
    CHECK_FALSE(Vector2::IsCollinear(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(3.0f, 2.0f)));
    CHECK_FALSE(Vector2::IsCollinear(Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector2(1.0f, 5.0f)));
}
