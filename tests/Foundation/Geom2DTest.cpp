// Tests for the 2D geometry helpers in Geom2D.h.
//
// Coordinates follow the engine's screen convention: X grows to the right and
// Y grows downward, so "left top" is the minimum corner of a box.
// "Extension" of a box is its half size.
//
// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
//
// Not covered here:
// - Geom2D::SweepSphereSegment is not implemented and always returns false.
// - The Point2 overload of Geom2D::IntersectSegmentSegment multiplies two
//   triangle areas as int, which overflows (undefined behavior) once the
//   coordinates reach a few hundred. It has no callers in the engine.
#include <Bibim/Config.h>
#include <Bibim/Geom2D.h>
#include <Bibim/Math.h>
#include <doctest.h>
#include <cmath>
#include <vector>

using namespace Bibim;

namespace
{
    bool Near(float a, float b)
    {
        return std::fabs(a - b) < 0.0001f;
    }

    bool Near(Vector2 a, Vector2 b)
    {
        return Near(a.X, b.X) && Near(a.Y, b.Y);
    }
}

TEST_CASE("GetPointAxisAlignedBoxDistanceSquared is zero inside and squared distance outside")
{
    const Vector2 min(0.0f, 0.0f);
    const Vector2 max(10.0f, 10.0f);
    CHECK(Geom2D::GetPointAxisAlignedBoxDistanceSquared(Vector2(5.0f, 5.0f), min, max) == 0.0f);
    CHECK(Geom2D::GetPointAxisAlignedBoxDistanceSquared(Vector2(0.0f, 10.0f), min, max) == 0.0f);
    CHECK(Geom2D::GetPointAxisAlignedBoxDistanceSquared(Vector2(-3.0f, 5.0f), min, max) == 9.0f);
    CHECK(Geom2D::GetPointAxisAlignedBoxDistanceSquared(Vector2(5.0f, 12.0f), min, max) == 4.0f);
    CHECK(Geom2D::GetPointAxisAlignedBoxDistanceSquared(Vector2(13.0f, -4.0f), min, max) == 25.0f);

    // The Point2 overload must agree with the Vector2 one.
    CHECK(Geom2D::GetPointAxisAlignedBoxDistanceSquared(Point2(5, 5), Point2(0, 0), Point2(10, 10)) == 0);
    CHECK(Geom2D::GetPointAxisAlignedBoxDistanceSquared(Point2(13, -4), Point2(0, 0), Point2(10, 10)) == 25);
}

TEST_CASE("GetPointSegmentDistanceSquared measures to the nearest point of the segment")
{
    const Vector2 a(0.0f, 0.0f);
    const Vector2 b(10.0f, 0.0f);
    CHECK(Near(Geom2D::GetPointSegmentDistanceSquared(Vector2(5.0f, 3.0f), a, b), 9.0f));   // beside the middle
    CHECK(Near(Geom2D::GetPointSegmentDistanceSquared(Vector2(-3.0f, 4.0f), a, b), 25.0f)); // past the start
    CHECK(Near(Geom2D::GetPointSegmentDistanceSquared(Vector2(13.0f, 4.0f), a, b), 25.0f)); // past the end
    CHECK(Near(Geom2D::GetPointSegmentDistanceSquared(Vector2(7.0f, 0.0f), a, b), 0.0f));   // on it

    CHECK(Geom2D::GetPointSegmentDistanceSquared(Point2(5, 3), Point2(0, 0), Point2(10, 0)) == 9);

    CHECK(Geom2D::IntersectPointSegment(Vector2(7.0f, 0.0f), a, b));
    CHECK_FALSE(Geom2D::IntersectPointSegment(Vector2(7.0f, 1.0f), a, b));
}

TEST_CASE("IntersectPointPolygon tells inside from outside, also for a concave polygon")
{
    // An L shape: the square (0,0)-(10,10) without its right bottom quarter.
    std::vector<Vector2> l;
    l.push_back(Vector2(0.0f, 0.0f));
    l.push_back(Vector2(10.0f, 0.0f));
    l.push_back(Vector2(10.0f, 5.0f));
    l.push_back(Vector2(5.0f, 5.0f));
    l.push_back(Vector2(5.0f, 10.0f));
    l.push_back(Vector2(0.0f, 10.0f));

    CHECK(Geom2D::IntersectPointPolygon(Vector2(2.0f, 2.0f), l));
    CHECK(Geom2D::IntersectPointPolygon(Vector2(8.0f, 2.0f), l));
    CHECK(Geom2D::IntersectPointPolygon(Vector2(2.0f, 8.0f), l));
    CHECK_FALSE(Geom2D::IntersectPointPolygon(Vector2(8.0f, 8.0f), l)); // the cut-out quarter
    CHECK_FALSE(Geom2D::IntersectPointPolygon(Vector2(-1.0f, 2.0f), l));
    CHECK_FALSE(Geom2D::IntersectPointPolygon(Vector2(2.0f, 2.0f), std::vector<Vector2>()));
}

TEST_CASE("IntersectSegmentSegment finds crossings and where they are on the first segment")
{
    float t = -1.0f;
    CHECK(Geom2D::IntersectSegmentSegment(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f),
                                          Vector2(4.0f, -5.0f), Vector2(4.0f, 5.0f), t));
    CHECK(Near(t, 0.4f));

    // Touching at an end point counts.
    CHECK(Geom2D::IntersectSegmentSegment(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f),
                                          Vector2(10.0f, 0.0f), Vector2(10.0f, 5.0f)));

    // Parallel, and crossing only on the extension of one segment.
    CHECK_FALSE(Geom2D::IntersectSegmentSegment(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f),
                                                Vector2(0.0f, 1.0f), Vector2(10.0f, 1.0f)));
    CHECK_FALSE(Geom2D::IntersectSegmentSegment(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f),
                                                Vector2(12.0f, -5.0f), Vector2(12.0f, 5.0f)));

    // The Point2 overload, with small coordinates.
    t = -1.0f;
    CHECK(Geom2D::IntersectSegmentSegment(Point2(0, 0), Point2(10, 10), Point2(0, 10), Point2(10, 0), t));
    CHECK(Near(t, 0.5f));
    CHECK_FALSE(Geom2D::IntersectSegmentSegment(Point2(0, 0), Point2(10, 0), Point2(0, 1), Point2(10, 1)));
}

TEST_CASE("IntersectSegmentSegment handles segments on the same line"
          * doctest::should_fail()
          * doctest::description("known bug: collinear segments always intersect, with result 0/0 (NaN)"))
{
    // Apart on the same line: no intersection.
    CHECK_FALSE(Geom2D::IntersectSegmentSegment(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f),
                                                Vector2(20.0f, 0.0f), Vector2(30.0f, 0.0f)));

    // Overlapping on the same line: an intersection with a usable position.
    float t = -1.0f;
    CHECK(Geom2D::IntersectSegmentSegment(Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f),
                                          Vector2(5.0f, 0.0f), Vector2(15.0f, 0.0f), t));
    CHECK(t >= 0.0f);
    CHECK(t <= 1.0f);
}

TEST_CASE("IntersectSegmentAxisAlignedBox detects segments crossing, inside and missing the box")
{
    const Vector2 leftTop(0.0f, 0.0f);
    const Vector2 rightBottom(10.0f, 10.0f);
    CHECK(Geom2D::IntersectSegmentAxisAlignedBox(Vector2(-5.0f, 5.0f), Vector2(15.0f, 5.0f), leftTop, rightBottom));
    CHECK(Geom2D::IntersectSegmentAxisAlignedBox(Vector2(2.0f, 2.0f), Vector2(3.0f, 3.0f), leftTop, rightBottom));
    CHECK(Geom2D::IntersectSegmentAxisAlignedBox(Vector2(-4.0f, 5.0f), Vector2(6.0f, -5.0f), leftTop, rightBottom)); // cuts the left top corner
    CHECK_FALSE(Geom2D::IntersectSegmentAxisAlignedBox(Vector2(-5.0f, 4.0f), Vector2(4.0f, -5.0f), leftTop, rightBottom)); // passes the corner
    CHECK_FALSE(Geom2D::IntersectSegmentAxisAlignedBox(Vector2(-5.0f, 20.0f), Vector2(15.0f, 20.0f), leftTop, rightBottom));
}

TEST_CASE("IntersectAxisAlignedBoxAxisAlignedBox detects overlap, touching and separation")
{
    CHECK(Geom2D::IntersectAxisAlignedBoxAxisAlignedBox(Vector2(0.0f, 0.0f), Vector2(10.0f, 10.0f),
                                                        Vector2(5.0f, 5.0f), Vector2(15.0f, 15.0f)));
    CHECK(Geom2D::IntersectAxisAlignedBoxAxisAlignedBox(Vector2(0.0f, 0.0f), Vector2(10.0f, 10.0f),
                                                        Vector2(10.0f, 0.0f), Vector2(20.0f, 10.0f)));
    CHECK_FALSE(Geom2D::IntersectAxisAlignedBoxAxisAlignedBox(Vector2(0.0f, 0.0f), Vector2(10.0f, 10.0f),
                                                              Vector2(11.0f, 0.0f), Vector2(20.0f, 10.0f)));
    CHECK_FALSE(Geom2D::IntersectAxisAlignedBoxAxisAlignedBox(Vector2(0.0f, 0.0f), Vector2(10.0f, 10.0f),
                                                              Vector2(0.0f, 11.0f), Vector2(10.0f, 20.0f)));
}

TEST_CASE("IntersectBoxBox separates rotated boxes whose bounding boxes overlap")
{
    const Vector2 axis(1.0f, 0.0f);
    const Vector2 diagonal(Math::InvSqrt2, Math::InvSqrt2);
    const Vector2 unit(1.0f, 1.0f);

    CHECK(Geom2D::IntersectBoxBox(Vector2(0.0f, 0.0f), axis, unit, Vector2(1.5f, 0.0f), axis, unit));
    CHECK_FALSE(Geom2D::IntersectBoxBox(Vector2(0.0f, 0.0f), axis, unit, Vector2(2.5f, 0.0f), axis, unit));

    // A diamond at (2.3, 2.3): its bounding box reaches into the first box,
    // but the diamond itself stays clear of the corner.
    CHECK_FALSE(Geom2D::IntersectBoxBox(Vector2(0.0f, 0.0f), axis, unit, Vector2(2.3f, 2.3f), diagonal, unit));
    CHECK(Geom2D::IntersectBoxBox(Vector2(0.0f, 0.0f), axis, unit, Vector2(1.6f, 1.6f), diagonal, unit));
}

TEST_CASE("RaycastSegment reports the distance along the ray")
{
    float distance = -1.0f;
    CHECK(Geom2D::RaycastSegment(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), 10.0f,
                                 Vector2(4.0f, -1.0f), Vector2(4.0f, 1.0f), distance));
    CHECK(Near(distance, 4.0f));

    // Too short to reach.
    CHECK_FALSE(Geom2D::RaycastSegment(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), 3.0f,
                                       Vector2(4.0f, -1.0f), Vector2(4.0f, 1.0f), distance));
}

TEST_CASE("RaycastSphere reports the entry distance, and 0 from inside")
{
    float distance = -1.0f;
    CHECK(Geom2D::RaycastSphere(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(10.0f, 0.0f), 4.0f, distance));
    CHECK(Near(distance, 8.0f));

    CHECK(Geom2D::RaycastSphere(Vector2(10.0f, 1.0f), Vector2(1.0f, 0.0f), Vector2(10.0f, 0.0f), 4.0f, distance));
    CHECK(distance == 0.0f);

    CHECK_FALSE(Geom2D::RaycastSphere(Vector2(0.0f, 0.0f), Vector2(-1.0f, 0.0f), Vector2(10.0f, 0.0f), 4.0f, distance)); // behind
    CHECK_FALSE(Geom2D::RaycastSphere(Vector2(0.0f, 5.0f), Vector2(1.0f, 0.0f), Vector2(10.0f, 0.0f), 4.0f, distance));  // passes by
}

TEST_CASE("RaycastAxisAlignedBox reports the distance and the normal of the face it enters")
{
    const Vector2 leftTop(0.0f, 0.0f);
    const Vector2 rightBottom(10.0f, 10.0f);
    float distance = -1.0f;
    Vector2 normal;

    CHECK(Geom2D::RaycastAxisAlignedBox(Vector2(-5.0f, 5.0f), Vector2(1.0f, 0.0f), leftTop, rightBottom, distance, normal));
    CHECK(Near(distance, 5.0f));
    CHECK(Near(normal, Vector2(-1.0f, 0.0f)));

    CHECK(Geom2D::RaycastAxisAlignedBox(Vector2(15.0f, 5.0f), Vector2(-1.0f, 0.0f), leftTop, rightBottom, distance, normal));
    CHECK(Near(distance, 5.0f));
    CHECK(Near(normal, Vector2(1.0f, 0.0f)));

    CHECK(Geom2D::RaycastAxisAlignedBox(Vector2(5.0f, -3.0f), Vector2(0.0f, 1.0f), leftTop, rightBottom, distance, normal));
    CHECK(Near(distance, 3.0f));
    CHECK(Near(normal, Vector2(0.0f, -1.0f)));

    CHECK(Geom2D::RaycastAxisAlignedBox(Vector2(5.0f, 12.0f), Vector2(0.0f, -1.0f), leftTop, rightBottom, distance, normal));
    CHECK(Near(distance, 2.0f));
    CHECK(Near(normal, Vector2(0.0f, 1.0f)));

    CHECK_FALSE(Geom2D::RaycastAxisAlignedBox(Vector2(-5.0f, 15.0f), Vector2(1.0f, 0.0f), leftTop, rightBottom, distance, normal));
    CHECK_FALSE(Geom2D::RaycastAxisAlignedBox(Vector2(-5.0f, 5.0f), Vector2(-1.0f, 0.0f), leftTop, rightBottom, distance, normal));
}

TEST_CASE("RaycastBox matches RaycastAxisAlignedBox and rotates the normal back to world space")
{
    float distance = -1.0f;
    Vector2 normal;

    // Not rotated.
    CHECK(Geom2D::RaycastBox(Vector2(-5.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(2.0f, 1.0f), distance, normal));
    CHECK(Near(distance, 3.0f));
    CHECK(Near(normal, Vector2(-1.0f, 0.0f)));

    // Turned a quarter: the long side (extension X = 2) now runs along Y,
    // so the box covers x in [-1, 1] and y in [-2, 2].
    const Vector2 extension(2.0f, 1.0f);
    CHECK(Geom2D::RaycastBox(Vector2(-5.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f), extension, distance, normal));
    CHECK(Near(distance, 4.0f));
    CHECK(Near(normal, Vector2(-1.0f, 0.0f)));

    CHECK(Geom2D::RaycastBox(Vector2(0.0f, 5.0f), Vector2(0.0f, -1.0f), Vector2(0.0f, 0.0f), Vector2(0.0f, -1.0f), extension, distance, normal));
    CHECK(Near(distance, 3.0f));
    CHECK(Near(normal, Vector2(0.0f, 1.0f)));

    CHECK_FALSE(Geom2D::RaycastBox(Vector2(-5.0f, 3.0f), Vector2(1.0f, 0.0f), Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f), extension, distance, normal));
}

TEST_CASE("SweepSphereAxisAlignedBox stops a moving circle at a face or a corner")
{
    const Vector2 leftTop(-1.0f, -1.0f);
    const Vector2 rightBottom(1.0f, 1.0f);
    float distance = -1.0f;
    Vector2 normal;

    // Head-on into the left face: the circle stops one radius before it.
    CHECK(Geom2D::SweepSphereAxisAlignedBox(Vector2(-5.0f, 0.0f), 1.0f, Vector2(1.0f, 0.0f), 10.0f, leftTop, rightBottom, distance, normal));
    CHECK(Near(distance, 3.0f));
    CHECK(Near(normal, Vector2(-1.0f, 0.0f)));

    // Diagonally into the left top corner.
    const Vector2 diagonal(Math::InvSqrt2, Math::InvSqrt2);
    CHECK(Geom2D::SweepSphereAxisAlignedBox(Vector2(-5.0f, -5.0f), 1.0f, diagonal, 10.0f, leftTop, rightBottom, distance, normal));
    CHECK(Near(distance, 4.0f * std::sqrt(2.0f) - 1.0f));
    CHECK(Near(normal, -diagonal));

    // Too short, and moving away.
    CHECK_FALSE(Geom2D::SweepSphereAxisAlignedBox(Vector2(-5.0f, 0.0f), 1.0f, Vector2(1.0f, 0.0f), 2.0f, leftTop, rightBottom, distance, normal));
    CHECK_FALSE(Geom2D::SweepSphereAxisAlignedBox(Vector2(-5.0f, 0.0f), 1.0f, Vector2(-1.0f, 0.0f), 10.0f, leftTop, rightBottom, distance, normal));
}

TEST_CASE("SweepSphereAxisAlignedBoxInside stops a circle moving inside the box at the wall")
{
    float distance = -1.0f;
    Vector2 normal;
    CHECK(Geom2D::SweepSphereAxisAlignedBoxInside(Vector2(0.0f, 0.0f), 1.0f, Vector2(1.0f, 0.0f), 10.0f,
                                                  Vector2(-5.0f, -5.0f), Vector2(5.0f, 5.0f), distance, normal));
    CHECK(Near(distance, 4.0f));
    CHECK(Near(normal, Vector2(-1.0f, 0.0f))); // points back into the box

    CHECK_FALSE(Geom2D::SweepSphereAxisAlignedBoxInside(Vector2(0.0f, 0.0f), 1.0f, Vector2(1.0f, 0.0f), 3.0f,
                                                        Vector2(-5.0f, -5.0f), Vector2(5.0f, 5.0f), distance, normal));
}

TEST_CASE("GetClosestPoint clamps to the segment")
{
    const Vector2 origin(0.0f, 0.0f);
    const Vector2 direction(1.0f, 0.0f);
    CHECK(Near(Geom2D::GetClosestPoint(origin, direction, 10.0f, Vector2(4.0f, 3.0f)), Vector2(4.0f, 0.0f)));
    CHECK(Near(Geom2D::GetClosestPoint(origin, direction, 10.0f, Vector2(-4.0f, 3.0f)), Vector2(0.0f, 0.0f)));
    CHECK(Near(Geom2D::GetClosestPoint(origin, direction, 10.0f, Vector2(14.0f, 3.0f)), Vector2(10.0f, 0.0f)));
}

TEST_CASE("IsClockwise is true for clockwise winding on screen (Y down)")
{
    // Right, then down: clockwise on screen.
    CHECK(Geom2D::IsClockwise(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f)));
    CHECK_FALSE(Geom2D::IsClockwise(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 0.0f)));
    CHECK_FALSE(Geom2D::IsClockwise(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(2.0f, 0.0f)));

    const Vector2 clockwise[] = { Vector2(0.0f, 0.0f), Vector2(10.0f, 0.0f), Vector2(10.0f, 10.0f), Vector2(0.0f, 10.0f) };
    const Vector2 counterClockwise[] = { Vector2(0.0f, 0.0f), Vector2(0.0f, 10.0f), Vector2(10.0f, 10.0f), Vector2(10.0f, 0.0f) };
    CHECK(Geom2D::IsClockwise(clockwise, 4));
    CHECK_FALSE(Geom2D::IsClockwise(counterClockwise, 4));
}
