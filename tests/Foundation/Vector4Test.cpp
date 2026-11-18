// Tests for Vector4.
#include <Bibim/Config.h>
#include <Bibim/Vector4.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("Vector4 constructors")
{
    CHECK(Vector4() == Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    CHECK(Vector4(1.0f, 2.0f, 3.0f) == Vector4(1.0f, 2.0f, 3.0f, 0.0f));
    CHECK(Vector4(Vector3(1.0f, 2.0f, 3.0f), 4.0f) == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    CHECK(Vector4::Zero == Vector4(0.0f, 0.0f, 0.0f, 0.0f));
    CHECK(Vector4::One == Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    CHECK(Vector4::UnitX == Vector4(1.0f, 0.0f, 0.0f, 0.0f));
    CHECK(Vector4::UnitY == Vector4(0.0f, 1.0f, 0.0f, 0.0f));
    CHECK(Vector4::UnitZ == Vector4(0.0f, 0.0f, 1.0f, 0.0f));
    CHECK(Vector4::UnitW == Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

TEST_CASE("Vector4 arithmetic works per component")
{
    const Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    const Vector4 b(5.0f, -6.0f, 7.0f, -8.0f);
    CHECK(a + b == Vector4(6.0f, -4.0f, 10.0f, -4.0f));
    CHECK(a - b == Vector4(-4.0f, 8.0f, -4.0f, 12.0f));
    CHECK(a * 2.0f == Vector4(2.0f, 4.0f, 6.0f, 8.0f));
    CHECK(2.0f * a == Vector4(2.0f, 4.0f, 6.0f, 8.0f));
    CHECK(b / 2.0f == Vector4(2.5f, -3.0f, 3.5f, -4.0f));
    CHECK(-a == Vector4(-1.0f, -2.0f, -3.0f, -4.0f));
    CHECK(+a == a);
    CHECK(a.Multiply(b) == Vector4(5.0f, -12.0f, 21.0f, -32.0f));
    CHECK(a != b);

    // Each component counts in equality.
    CHECK(a != Vector4(1.0f, 2.0f, 3.0f, 5.0f));

    Vector4 v = a;
    v += b;
    CHECK(v == a + b);
    v -= b;
    CHECK(v == a);
    v *= 4.0f;
    CHECK(v == Vector4(4.0f, 8.0f, 12.0f, 16.0f));
    v /= 2.0f;
    CHECK(v == Vector4(2.0f, 4.0f, 6.0f, 8.0f));
}
