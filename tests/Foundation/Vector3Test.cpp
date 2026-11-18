// Tests for Vector3.
//
// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
#include <Bibim/Config.h>
#include <Bibim/Math.h>
#include <Bibim/Vector3.h>
#include <doctest.h>
#include <cfloat>

using namespace Bibim;

TEST_CASE("Vector3 constructors")
{
    CHECK(Vector3() == Vector3(0.0f, 0.0f, 0.0f));
    CHECK(Vector3(1, 2, 3) == Vector3(1.0f, 2.0f, 3.0f));
    CHECK(Vector3(Vector2(1.0f, 2.0f), 3.0f) == Vector3(1.0f, 2.0f, 3.0f));
    CHECK(Vector3::Zero == Vector3(0.0f, 0.0f, 0.0f));
    CHECK(Vector3::One == Vector3(1.0f, 1.0f, 1.0f));
    CHECK(Vector3::UnitX == Vector3(1.0f, 0.0f, 0.0f));
    CHECK(Vector3::UnitY == Vector3(0.0f, 1.0f, 0.0f));
    CHECK(Vector3::UnitZ == Vector3(0.0f, 0.0f, 1.0f));
}

TEST_CASE("Vector3 arithmetic works per component")
{
    const Vector3 a(1.0f, 2.0f, 3.0f);
    const Vector3 b(4.0f, -5.0f, 6.0f);
    CHECK(a + b == Vector3(5.0f, -3.0f, 9.0f));
    CHECK(a - b == Vector3(-3.0f, 7.0f, -3.0f));
    CHECK(a * 2.0f == Vector3(2.0f, 4.0f, 6.0f));
    CHECK(2.0f * a == Vector3(2.0f, 4.0f, 6.0f));
    CHECK(b / 2.0f == Vector3(2.0f, -2.5f, 3.0f));
    CHECK(-a == Vector3(-1.0f, -2.0f, -3.0f));
    CHECK(+a == a);
    CHECK(a.Multiply(b) == Vector3(4.0f, -10.0f, 18.0f));
    CHECK(a != b);

    Vector3 v = a;
    v += b;
    CHECK(v == a + b);
    v -= b;
    CHECK(v == a);
    v *= 4.0f;
    CHECK(v == Vector3(4.0f, 8.0f, 12.0f));
    v /= 2.0f;
    CHECK(v == Vector3(2.0f, 4.0f, 6.0f));
}

TEST_CASE("Vector3 Dot and Cross")
{
    const Vector3 a(1.0f, 2.0f, 3.0f);
    const Vector3 b(4.0f, -5.0f, 6.0f);
    CHECK(a.Dot(b) == 12.0f);

    CHECK(Vector3::UnitX.Cross(Vector3::UnitY) == Vector3::UnitZ);
    CHECK(Vector3::UnitY.Cross(Vector3::UnitZ) == Vector3::UnitX);
    CHECK(Vector3::UnitZ.Cross(Vector3::UnitX) == Vector3::UnitY);
    CHECK(Vector3::UnitY.Cross(Vector3::UnitX) == -Vector3::UnitZ);

    const Vector3 c = a.Cross(b);
    CHECK(c == Vector3(27.0f, 6.0f, -13.0f));
    CHECK(c.Dot(a) == 0.0f);
    CHECK(c.Dot(b) == 0.0f);
}

TEST_CASE("Vector3 length and Normalize")
{
    Vector3 v(2.0f, 3.0f, 6.0f);
    CHECK(v.GetLength() == 7.0f);
    CHECK(v.GetLengthSquared() == 49.0f);

    CHECK(v.Normalize() == 7.0f);
    CHECK(Math::Equals(v, Vector3(2.0f / 7.0f, 3.0f / 7.0f, 6.0f / 7.0f)));

    // A unit vector is left as it is.
    Vector3 unit = Vector3::UnitY;
    CHECK(unit.Normalize() == 1.0f);
    CHECK(unit == Vector3::UnitY);
}

TEST_CASE("Vector3 GetDistance and GetDistanceSquared"
          * doctest::should_fail(true)
          * doctest::description("Known bug: GetDistanceSquared multiplies the Z difference by "
                                 "(b.Y - a.Z) instead of (b.Z - a.Z)"))
{
    const Vector3 a(1.0f, 2.0f, 3.0f);
    const Vector3 b(3.0f, 5.0f, 9.0f);
    CHECK(Vector3::GetDistanceSquared(a, b) == 49.0f);
    CHECK(Vector3::GetDistance(a, b) == 7.0f);
    CHECK(Vector3::GetDistance(b, a) == 7.0f);
}

TEST_CASE("Vector3 Rotate turns counterclockwise about a unit axis")
{
    // Looking from the tip of the axis toward the origin, a positive angle
    // turns counterclockwise (right-hand rule).
    Vector3 v = Vector3::UnitX;
    v.Rotate(Vector3::UnitZ, Math::PiOver2);
    CHECK(Math::Equals(v, Vector3::UnitY));

    v = Vector3::UnitY;
    v.Rotate(Vector3::UnitX, Math::PiOver2);
    CHECK(Math::Equals(v, Vector3::UnitZ));

    v = Vector3::UnitZ;
    v.Rotate(Vector3::UnitY, Math::PiOver2);
    CHECK(Math::Equals(v, Vector3::UnitX));

    // The component along the axis stays; the rest turns.
    v = Vector3(1.0f, 0.0f, 5.0f);
    v.Rotate(Vector3::UnitZ, Math::Pi);
    CHECK(Math::Equals(v, Vector3(-1.0f, 0.0f, 5.0f)));

    // The sin/cos overload takes the precomputed values.
    v = Vector3::UnitX;
    v.Rotate(Vector3::UnitZ, 1.0f, 0.0f);
    CHECK(v == Vector3::UnitY);
}

TEST_CASE("Vector3 Minimum and Maximum hold the lowest and highest floats")
{
    CHECK(Vector3::Minimum == Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
    CHECK(Vector3::Maximum == Vector3(FLT_MAX, FLT_MAX, FLT_MAX));
}
