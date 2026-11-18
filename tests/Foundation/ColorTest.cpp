// Tests for Color.
//
// Color keeps 8-bit channels. Arithmetic saturates at 0 and 255, and
// converting from float truncates (it does not round).
//
// Not covered here:
// - Dividing by a Color with a zero channel: a zero channel divided by it is
//   0 / 0, and converting that NaN to a byte is undefined behavior.
#include <Bibim/Config.h>
#include <Bibim/Color.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("Color constructors")
{
    const Color c(10, 20, 30, 40);
    CHECK(c.R == 10);
    CHECK(c.G == 20);
    CHECK(c.B == 30);
    CHECK(c.A == 40);

    CHECK(Color(10, 20, 30) == Color(10, 20, 30, 255));
    CHECK(Color() == Color(0, 0, 0, 0));
    CHECK(Color() == Color::TransparentBlack);

    // Integer channels outside 0 to 255 are clamped.
    CHECK(Color(-5, 300, 128, 1000) == Color(0, 255, 128, 255));
}

TEST_CASE("Color packs and unpacks ARGB and RGBA")
{
    const Color c(0x80112233UL);
    CHECK(c == Color(0x11, 0x22, 0x33, 0x80));
    CHECK(c.ToARGB() == 0x80112233UL);
    CHECK(c.ToRGBA() == 0x11223380UL);

    CHECK(Color::Red == Color(255, 0, 0));
    CHECK(Color::Lime == Color(0, 255, 0));
    CHECK(Color::Blue == Color(0, 0, 255));
    CHECK(Color::White.ToARGB() == 0xFFFFFFFFUL);
    CHECK(Color::TransparentWhite == Color(255, 255, 255, 0));
    CHECK(Color::CornflowerBlue.ToARGB() == 0xFF6495EDUL);
}

TEST_CASE("Color converts to and from 0 to 1 floats")
{
    const Color c(0, 51, 255, 102);
    CHECK(c.GetR() == 0.0f);
    CHECK(c.GetG() == doctest::Approx(0.2f));
    CHECK(c.GetB() == 1.0f);
    CHECK(c.GetA() == doctest::Approx(0.4f));
    CHECK(c.ToVector4().W == c.GetA());
    CHECK(c.ToVector3().Y == c.GetG());

    CHECK(Color(Vector3(1.0f, 0.0f, 0.5f)) == Color(255, 0, 127, 255));
    CHECK(Color(Vector4(0.0f, 1.0f, 0.0f, 0.5f)) == Color(0, 255, 0, 127));

    // Values outside 0 to 1 are clamped.
    CHECK(Color(Vector4(-1.0f, 2.0f, 0.0f, 1.0f)) == Color(0, 255, 0, 255));

    // Every channel value survives a round trip through float.
    for (int i = 0; i < 256; i++)
    {
        const Color original(i, i, i, i);
        CHECK(Color(original.ToVector4()) == original);
        CHECK(Color(original.ToVector3()) == Color(i, i, i, 255));
    }
}

TEST_CASE("Color addition and subtraction saturate")
{
    const Color a(200, 100, 0, 255);
    const Color b(100, 100, 10, 1);
    CHECK(a + b == Color(255, 200, 10, 255));
    CHECK(a - b == Color(100, 0, 0, 254));
    CHECK(b - a == Color(0, 0, 10, 0));

    Color c = a;
    c += b;
    CHECK(c == a + b);
    c = a;
    c -= b;
    CHECK(c == a - b);
}

TEST_CASE("Color multiplication by a Color modulates each channel")
{
    const Color c(200, 100, 50, 255);
    CHECK(c * Color::White == c);
    CHECK(c * Color::TransparentBlack == Color::TransparentBlack);
    CHECK(c * Color(128, 128, 128, 128) == Color(100, 50, 25, 128));

    Color m = c;
    m *= Color(128, 128, 128, 128);
    CHECK(m == c * Color(128, 128, 128, 128));
}

TEST_CASE("Color multiplication and division by numbers clamp")
{
    const Color c(200, 100, 50, 100);
    CHECK(c * 0.5f == Color(100, 50, 25, 50));
    CHECK(c * 2.0f == Color(255, 200, 100, 200));
    CHECK(c * -1.0f == Color(0, 0, 0, 0));
    CHECK(c / 2.0f == Color(100, 50, 25, 50));
    CHECK(c * Vector4(0.5f, 1.0f, 2.0f, 0.0f) == Color(100, 100, 100, 0));

    Color m = c;
    m *= 2.0f;
    CHECK(m == c * 2.0f);
    m = c;
    m /= 2.0f;
    CHECK(m == c / 2.0f);
    m = c;
    m *= Vector4(0.5f, 1.0f, 2.0f, 0.0f);
    CHECK(m == c * Vector4(0.5f, 1.0f, 2.0f, 0.0f));
}

TEST_CASE("Color division by a Color scales each channel up")
{
    const Color c(50, 100, 200, 51);
    const Color half(127, 127, 127, 255);
    CHECK(c / Color::White == c);
    CHECK(c / half == Color(100, 200, 255, 51));

    Color d = c;
    d /= half;
    CHECK(d == c / half);
}
