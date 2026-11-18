// Tests for the integer points Point2, Point3 and Point4.
#include <Bibim/Config.h>
#include <Bibim/Point2.h>
#include <Bibim/Point3.h>
#include <Bibim/Point4.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("Point2 constructors")
{
    CHECK(Point2() == Point2(0, 0));
    CHECK(Point2::Zero == Point2(0, 0));

    // The float constructor truncates toward zero.
    CHECK(Point2(1.9f, -1.9f) == Point2(1, -1));
    CHECK(Point2(-0.5f, 0.5f) == Point2(0, 0));
}

TEST_CASE("Point2 arithmetic works per component")
{
    const Point2 a(1, 2);
    const Point2 b(3, -4);
    CHECK(a + b == Point2(4, -2));
    CHECK(a - b == Point2(-2, 6));
    CHECK(a * 3 == Point2(3, 6));
    CHECK(-a == Point2(-1, -2));
    CHECK(+a == a);
    CHECK(a.Dot(b) == -5);
    CHECK(a != b);

    // Integer division truncates toward zero.
    CHECK(Point2(7, -7) / 2 == Point2(3, -3));

    Point2 p = a;
    p += b;
    CHECK(p == Point2(4, -2));
    p -= b;
    CHECK(p == a);
    p *= 4;
    CHECK(p == Point2(4, 8));
    p /= 3;
    CHECK(p == Point2(1, 2));
}

TEST_CASE("Point2 GetDistance and GetDistanceSquared")
{
    CHECK(Point2::GetDistanceSquared(Point2(1, 1), Point2(4, 5)) == 25);
    CHECK(Point2::GetDistance(Point2(1, 1), Point2(4, 5)) == 5.0f);
    CHECK(Point2::GetDistance(Point2(4, 5), Point2(1, 1)) == 5.0f);
}

TEST_CASE("Point3 arithmetic works per component")
{
    const Point3 a(1, 2, 3);
    const Point3 b(4, -5, 6);
    CHECK(Point3() == Point3(0, 0, 0));
    CHECK(a + b == Point3(5, -3, 9));
    CHECK(a - b == Point3(-3, 7, -3));
    CHECK(a * 2 == Point3(2, 4, 6));
    CHECK(b / 2 == Point3(2, -2, 3));
    CHECK(-a == Point3(-1, -2, -3));
    CHECK(+a == a);
    CHECK(a != b);
    CHECK(a != Point3(1, 2, 4));

    Point3 p = a;
    p += b;
    CHECK(p == a + b);
    p -= b;
    CHECK(p == a);
    p *= 2;
    CHECK(p == Point3(2, 4, 6));
    p /= 2;
    CHECK(p == a);
}

TEST_CASE("Point4 arithmetic works per component")
{
    const Point4 a(1, 2, 3, 4);
    const Point4 b(5, -6, 7, -8);
    CHECK(Point4() == Point4(0, 0, 0, 0));
    CHECK(Point4::Zero == Point4(0, 0, 0, 0));
    CHECK(a + b == Point4(6, -4, 10, -4));
    CHECK(a - b == Point4(-4, 8, -4, 12));
    CHECK(a * 2 == Point4(2, 4, 6, 8));
    CHECK(b / 2 == Point4(2, -3, 3, -4));
    CHECK(-a == Point4(-1, -2, -3, -4));
    CHECK(+a == a);
    CHECK(a != b);
    CHECK(a != Point4(1, 2, 3, 5));

    Point4 p = a;
    p += b;
    CHECK(p == a + b);
    p -= b;
    CHECK(p == a);
    p *= 2;
    CHECK(p == Point4(2, 4, 6, 8));
    p /= 2;
    CHECK(p == a);
}
