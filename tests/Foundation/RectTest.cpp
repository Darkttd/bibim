// Tests for the rectangle types Rect (int) and RectF (float).
//
// A rectangle is X, Y (left top) plus Width and Height; Y grows downward.
// Contains includes the right and bottom edges.
//
// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
#include <Bibim/Config.h>
#include <Bibim/Rect.h>
#include <Bibim/RectF.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("Rect edges, center and corners come from X, Y, Width and Height")
{
    const Rect r(10, 20, 30, 40);
    CHECK(r.GetLeft() == 10);
    CHECK(r.GetTop() == 20);
    CHECK(r.GetRight() == 40);
    CHECK(r.GetBottom() == 60);
    CHECK(r.GetCenter() == 25);
    CHECK(r.GetMiddle() == 40);
    CHECK(r.GetCenterPoint() == Point2(25, 40));
    CHECK(r.GetLeftTop() == Point2(10, 20));
    CHECK(r.GetRightBottom() == Point2(40, 60));
    CHECK(Rect(Point2(10, 20), Point2(40, 60)) == r);
}

TEST_CASE("Rect setters move one edge and keep the opposite edge")
{
    Rect r(10, 20, 30, 40);
    r.SetLeft(0);
    CHECK(r == Rect(0, 20, 40, 40));
    r.SetTop(0);
    CHECK(r == Rect(0, 0, 40, 60));
    r.SetRight(50);
    CHECK(r == Rect(0, 0, 50, 60));
    r.SetBottom(70);
    CHECK(r == Rect(0, 0, 50, 70));
}

TEST_CASE("Rect::Inflate grows each side by its own amount")
{
    Rect r(10, 10, 10, 10);
    r.Inflate(1);
    CHECK(r == Rect(9, 9, 12, 12));
    r.Inflate(2, 3);
    CHECK(r == Rect(7, 6, 16, 18));
    r.Inflate(1, 2, 3, 4);
    CHECK(r == Rect(6, 4, 20, 24));
}

TEST_CASE("Rect::Contains includes every edge")
{
    const Rect r(0, 0, 10, 10);
    CHECK(r.Contains(Point2(5, 5)));
    CHECK(r.Contains(Point2(0, 0)));
    CHECK(r.Contains(Point2(10, 10)));
    CHECK_FALSE(r.Contains(Point2(11, 5)));
    CHECK_FALSE(r.Contains(Point2(5, -1)));
}

TEST_CASE("Rect::Intersect and Rect::Union of overlapping rectangles")
{
    const Rect a(0, 0, 10, 10);
    const Rect b(5, 5, 10, 10);
    CHECK(Rect::Intersect(a, b) == Rect(5, 5, 5, 5));
    CHECK(Rect::Union(a, b) == Rect(0, 0, 15, 15));
    CHECK(Rect::Intersect(a, a) == a);
    CHECK(Rect::Union(a, Rect(20, 0, 5, 5)) == Rect(0, 0, 25, 10));

    CHECK(Rect::Empty.IsEmpty());
    CHECK(Rect(3, 3, 0, 5).IsEmpty());
    CHECK_FALSE(a.IsEmpty());
}

TEST_CASE("RectF mirrors Rect with floats")
{
    const RectF r(10.0f, 20.0f, 30.0f, 40.0f);
    CHECK(r.GetRight() == 40.0f);
    CHECK(r.GetBottom() == 60.0f);
    CHECK(r.GetCenterPoint() == Vector2(25.0f, 40.0f));
    CHECK(RectF(Vector2(10.0f, 20.0f), Vector2(40.0f, 60.0f)) == r);
    CHECK(r.Contains(Vector2(40.0f, 60.0f)));
    CHECK_FALSE(r.Contains(Vector2(40.5f, 60.0f)));

    RectF inflated = r;
    inflated.Inflate(1.0f, 2.0f, 3.0f, 4.0f);
    CHECK(inflated == RectF(9.0f, 18.0f, 34.0f, 46.0f));

    const RectF a(0.0f, 0.0f, 10.0f, 10.0f);
    const RectF b(5.0f, 5.0f, 10.0f, 10.0f);
    CHECK(RectF::Intersect(a, b) == RectF(5.0f, 5.0f, 5.0f, 5.0f));
    CHECK(RectF::Union(a, b) == RectF(0.0f, 0.0f, 15.0f, 15.0f));
}

TEST_CASE("RectF::Intersects is true only when the rectangles overlap or touch")
{
    const RectF a(0.0f, 0.0f, 10.0f, 10.0f);
    CHECK(a.Intersects(RectF(5.0f, 5.0f, 10.0f, 10.0f)));
    CHECK(a.Intersects(RectF(10.0f, 0.0f, 5.0f, 5.0f)));   // touching
    CHECK(a.Intersects(RectF(2.0f, 2.0f, 1.0f, 1.0f)));    // inside

    CHECK_FALSE(a.Intersects(RectF(20.0f, 0.0f, 5.0f, 5.0f)));
    CHECK_FALSE(a.Intersects(RectF(0.0f, 20.0f, 5.0f, 5.0f)));
    // The other rectangle lies to the left top of a.
    CHECK_FALSE(RectF(20.0f, 20.0f, 5.0f, 5.0f).Intersects(a));
}
