// Tests for the Collision2D shapes and the pairwise intersection table behind
// Shape2D::Intersect.
//
// Shapes are reference counted (SharedPointer), so they are created with new
// and held in a *Ptr. Coordinates follow the screen convention (Y grows
// downward). A box's extension is its half size.
//
// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
//
// Not covered here:
// - Segment-Box and AxisAlignedBox-Box intersection are not implemented and
//   always return false.
// - Known bug: AxisAlignedBoxShape2D, SegmentShape2D and BoxShape2D read from a
//   stream (OnRead) set their cached revision equal to the shape's revision, so
//   their scaled/rotated geometry stays at the constructor's zeros until the
//   position, rotation or scale changes. Testing it needs a component stream.
#include <Bibim/Config.h>
#include <Bibim/AxisAlignedBoxShape2D.h>
#include <Bibim/BoxShape2D.h>
#include <Bibim/Math.h>
#include <Bibim/PointShape2D.h>
#include <Bibim/RaycastReport2D.h>
#include <Bibim/SegmentShape2D.h>
#include <Bibim/SphereShape2D.h>
#include <Bibim/UserShape2D.h>
#include <doctest.h>
#include <cmath>

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

    PointShape2DPtr MakePoint(float x, float y)
    {
        PointShape2DPtr shape = new PointShape2D();
        shape->SetPosition(Vector2(x, y));
        return shape;
    }

    SphereShape2DPtr MakeSphere(float x, float y, float radius)
    {
        SphereShape2DPtr shape = new SphereShape2D();
        shape->SetPosition(Vector2(x, y));
        shape->SetRadius(radius);
        return shape;
    }

    AxisAlignedBoxShape2DPtr MakeAxisAlignedBox(float x, float y, Vector2 min, Vector2 max)
    {
        AxisAlignedBoxShape2DPtr shape = new AxisAlignedBoxShape2D();
        shape->SetPosition(Vector2(x, y));
        shape->SetMin(min);
        shape->SetMax(max);
        return shape;
    }

    // A 4 x 2 box turned a quarter, so it covers x in [x-1, x+1] and y in [y-2, y+2].
    BoxShape2DPtr MakeStandingBox(float x, float y)
    {
        BoxShape2DPtr shape = new BoxShape2D();
        shape->SetPosition(Vector2(x, y));
        shape->SetOrientation(Vector2(1.0f, 0.0f));
        shape->SetExtension(Vector2(2.0f, 1.0f));
        shape->SetRotation(Math::PiOver2);
        return shape;
    }

    SegmentShape2DPtr MakeSegment(float x, float y, Vector2 direction, float frontLength)
    {
        SegmentShape2DPtr shape = new SegmentShape2D();
        shape->SetPosition(Vector2(x, y));
        shape->SetDirection(direction);
        shape->SetFrontLength(frontLength);
        return shape;
    }

    // Records what the intersection table passes to a user shape.
    class RecordingShape : public UserShape2D
    {
        BBComponentClass(RecordingShape, UserShape2D, 'T', 'R', 'S', '2');
        public:
            Vector2 LastCenter;
            Vector2 LastExtension;

            RecordingShape() { }
            virtual ~RecordingShape() { }

            virtual bool Raycast(Vector2, Vector2, float, RaycastReport2D&, IRaycastCallback2D*) { return false; }
            virtual void Build(Vertices&) { }

            virtual bool IntersectPoint(Vector2) { return true; }
            virtual bool IntersectSegment(Vector2, Vector2) { return true; }
            virtual bool IntersectSphere(Vector2, float) { return true; }
            virtual bool IntersectAxisAlignedBox(Vector2, Vector2) { return true; }
            virtual bool IntersectBox(Vector2 center, Vector2 /*orientation*/, Vector2 /*up*/, Vector2 extension)
            {
                LastCenter = center;
                LastExtension = extension;
                return true;
            }
            virtual bool IntersectUserShape(UserShape2D*) { return true; }
    };

    BBImplementsComponent(RecordingShape);

    void RecordingShape::OnRead(ComponentStreamReader& reader)
    {
        Base::OnRead(reader);
    }

    void RecordingShape::OnCopy(const GameComponent* original, CloningContext& context)
    {
        Base::OnCopy(original, context);
    }
}

TEST_CASE("Shape2D::Intersect gives the same answer in either argument order")
{
    PointShape2DPtr point = MakePoint(0.5f, 0.5f);
    SphereShape2DPtr sphere = MakeSphere(0.0f, 0.0f, 1.0f);
    CHECK(Shape2D::Intersect(point, sphere));
    CHECK(Shape2D::Intersect(sphere, point));

    point->SetPosition(Vector2(1.0f, 1.0f));
    CHECK_FALSE(Shape2D::Intersect(point, sphere));
    CHECK_FALSE(Shape2D::Intersect(sphere, point));

    CHECK_FALSE(Shape2D::Intersect(sphere, nullptr));
}

TEST_CASE("Point shapes intersect points, spheres, boxes and segments")
{
    CHECK(Shape2D::Intersect(MakePoint(1.0f, 2.0f), MakePoint(1.0f, 2.0f)));
    CHECK_FALSE(Shape2D::Intersect(MakePoint(1.0f, 2.0f), MakePoint(1.0f, 3.0f)));

    AxisAlignedBoxShape2DPtr aabb = MakeAxisAlignedBox(10.0f, 0.0f, Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    CHECK(Shape2D::Intersect(MakePoint(10.5f, 0.5f), aabb));
    CHECK_FALSE(Shape2D::Intersect(MakePoint(0.0f, 0.0f), aabb));

    BoxShape2DPtr box = MakeStandingBox(0.0f, 0.0f);
    CHECK(Shape2D::Intersect(MakePoint(0.0f, 1.5f), box));
    CHECK_FALSE(Shape2D::Intersect(MakePoint(1.5f, 0.0f), box));

    SegmentShape2DPtr segment = MakeSegment(0.0f, 0.0f, Vector2(1.0f, 0.0f), 10.0f);
    CHECK(Shape2D::Intersect(MakePoint(3.0f, 0.0f), segment));
    CHECK_FALSE(Shape2D::Intersect(MakePoint(3.0f, 1.0f), segment));
    CHECK_FALSE(Shape2D::Intersect(MakePoint(11.0f, 0.0f), segment));
}

TEST_CASE("Sphere shapes use the radius times the scale")
{
    SphereShape2DPtr a = MakeSphere(0.0f, 0.0f, 1.0f);
    SphereShape2DPtr b = MakeSphere(1.5f, 0.0f, 1.0f);
    CHECK(Shape2D::Intersect(a, b));

    b->SetPosition(Vector2(2.5f, 0.0f));
    CHECK_FALSE(Shape2D::Intersect(a, b));

    b->SetScale(2.0f);
    CHECK(b->GetScaledRadius() == 2.0f);
    CHECK(b->GetScaledRadiusSquared() == 4.0f);
    CHECK(Shape2D::Intersect(a, b));
}

TEST_CASE("Sphere shapes intersect boxes by their closest point")
{
    AxisAlignedBoxShape2DPtr aabb = MakeAxisAlignedBox(10.0f, 0.0f, Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    SphereShape2DPtr sphere = MakeSphere(12.5f, 0.0f, 1.0f);
    CHECK_FALSE(Shape2D::Intersect(sphere, aabb));
    sphere->SetRadius(2.0f);
    CHECK(Shape2D::Intersect(sphere, aabb));

    BoxShape2DPtr box = MakeStandingBox(0.0f, 0.0f);
    SphereShape2DPtr beside = MakeSphere(2.0f, 0.0f, 1.2f);
    CHECK(Shape2D::Intersect(beside, box));
    beside->SetRadius(0.8f);
    CHECK_FALSE(Shape2D::Intersect(beside, box));
}

TEST_CASE("AxisAlignedBoxShape2D moves and scales its min and max with the shape")
{
    AxisAlignedBoxShape2DPtr aabb = MakeAxisAlignedBox(10.0f, 0.0f, Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    CHECK(aabb->GetScaledMin() == Vector2(9.0f, -1.0f));
    CHECK(aabb->GetScaledMax() == Vector2(11.0f, 1.0f));

    aabb->SetScale(2.0f);
    CHECK(aabb->GetScaledMin() == Vector2(8.0f, -2.0f));
    CHECK(aabb->GetScaledMax() == Vector2(12.0f, 2.0f));
    CHECK(Shape2D::Intersect(MakePoint(11.5f, 0.0f), aabb));
}

TEST_CASE("Two axis-aligned boxes intersect when they overlap"
          * doctest::should_fail()
          * doctest::description("known bug: IntersectShapes2D::AxisAlignedBoxAxisAlignedBox passes the second box's max as its min"))
{
    AxisAlignedBoxShape2DPtr a = MakeAxisAlignedBox(0.0f, 0.0f, Vector2(0.0f, 0.0f), Vector2(2.0f, 2.0f));
    AxisAlignedBoxShape2DPtr b = MakeAxisAlignedBox(1.0f, 1.0f, Vector2(0.0f, 0.0f), Vector2(2.0f, 2.0f));
    AxisAlignedBoxShape2DPtr c = MakeAxisAlignedBox(5.0f, 5.0f, Vector2(0.0f, 0.0f), Vector2(2.0f, 2.0f));
    CHECK(Shape2D::Intersect(a, b));
    CHECK(Shape2D::Intersect(b, a));
    CHECK_FALSE(Shape2D::Intersect(a, c));
}

TEST_CASE("BoxShape2D turns with its rotation")
{
    BoxShape2DPtr box = MakeStandingBox(0.0f, 0.0f);
    CHECK(Near(box->GetRotatedOrientation(), Vector2(0.0f, 1.0f)));
    CHECK(Near(box->GetScaledExtension(), Vector2(2.0f, 1.0f)));

    BoxShape2DPtr other = MakeStandingBox(1.5f, 0.0f);
    CHECK(Shape2D::Intersect(box, other));
    other->SetPosition(Vector2(2.5f, 0.0f));
    CHECK_FALSE(Shape2D::Intersect(box, other));
    other->SetPosition(Vector2(0.0f, 3.5f));
    CHECK(Shape2D::Intersect(box, other));
}

TEST_CASE("SegmentShape2D runs from its position along its rotated direction")
{
    SegmentShape2DPtr segment = MakeSegment(1.0f, 2.0f, Vector2(2.0f, 0.0f), 10.0f);
    CHECK(segment->GetDirection() == Vector2(1.0f, 0.0f)); // normalized
    CHECK(Near(segment->GetStartPoint(), Vector2(1.0f, 2.0f)));
    CHECK(Near(segment->GetEndPoint(), Vector2(11.0f, 2.0f)));

    segment->SetRotation(Math::PiOver2);
    CHECK(Near(segment->GetRotatedDirection(), Vector2(0.0f, 1.0f)));
    CHECK(Near(segment->GetEndPoint(), Vector2(1.0f, 12.0f)));

    segment->SetScale(0.5f);
    CHECK(Near(segment->GetEndPoint(), Vector2(1.0f, 7.0f)));

    segment->SetFrontLength(-3.0f);
    CHECK(segment->GetFrontLength() == 0.0f);
}

TEST_CASE("SegmentShape2D extends its back length behind its position"
          * doctest::should_fail()
          * doctest::description("known bug: UpdateParameters puts the start point back length ahead of the position, not behind it"))
{
    SegmentShape2DPtr segment = MakeSegment(0.0f, 0.0f, Vector2(1.0f, 0.0f), 5.0f);
    segment->SetBackLength(5.0f);
    CHECK(segment->GetLength() == 10.0f);
    CHECK(Near(Vector2::GetDistance(segment->GetStartPoint(), segment->GetEndPoint()), segment->GetLength()));
    CHECK(Shape2D::Intersect(MakePoint(-3.0f, 0.0f), segment));
}

TEST_CASE("Segment shapes intersect spheres, axis-aligned boxes and segments")
{
    SegmentShape2DPtr segment = MakeSegment(0.0f, 0.0f, Vector2(1.0f, 0.0f), 10.0f);
    CHECK(Shape2D::Intersect(segment, MakeSphere(5.0f, 1.0f, 1.5f)));
    CHECK_FALSE(Shape2D::Intersect(segment, MakeSphere(5.0f, 3.0f, 1.5f)));

    CHECK(Shape2D::Intersect(segment, MakeAxisAlignedBox(5.0f, 0.0f, Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f))));
    CHECK_FALSE(Shape2D::Intersect(segment, MakeAxisAlignedBox(5.0f, 5.0f, Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f))));

    CHECK(Shape2D::Intersect(segment, MakeSegment(5.0f, -5.0f, Vector2(0.0f, 1.0f), 10.0f)));
    CHECK_FALSE(Shape2D::Intersect(segment, MakeSegment(5.0f, 1.0f, Vector2(0.0f, 1.0f), 10.0f)));
}

TEST_CASE("Raycast reports the impact point, normal and distance of spheres and boxes")
{
    RaycastReport2D report;
    const Vector2 origin(0.0f, 0.0f);
    const Vector2 direction(1.0f, 0.0f);

    SphereShape2DPtr sphere = MakeSphere(10.0f, 0.0f, 2.0f);
    CHECK(sphere->Raycast(origin, direction, 20.0f, report, nullptr));
    CHECK(report.ImpactShape == sphere.GetPointee());
    CHECK(Near(report.ImpactDistance, 8.0f));
    CHECK(Near(report.ImpactPoint, Vector2(8.0f, 0.0f)));
    CHECK(Near(report.ImpactNormal, Vector2(-1.0f, 0.0f)));
    CHECK_FALSE(sphere->Raycast(origin, direction, 5.0f, report, nullptr));

    AxisAlignedBoxShape2DPtr aabb = MakeAxisAlignedBox(10.0f, 0.0f, Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    CHECK(aabb->Raycast(origin, direction, 20.0f, report, nullptr));
    CHECK(Near(report.ImpactDistance, 9.0f));
    CHECK(Near(report.ImpactNormal, Vector2(-1.0f, 0.0f)));

    BoxShape2DPtr box = MakeStandingBox(10.0f, 0.0f);
    CHECK(box->Raycast(origin, direction, 20.0f, report, nullptr));
    CHECK(Near(report.ImpactDistance, 9.0f));
    CHECK(Near(report.ImpactNormal, Vector2(-1.0f, 0.0f)));

    CHECK_FALSE(MakePoint(5.0f, 0.0f)->Raycast(origin, direction, 20.0f, report, nullptr));
}

TEST_CASE("Raycast on a segment reports a normal perpendicular to it, facing the ray"
          * doctest::should_fail()
          * doctest::description("known bug: SegmentShape2D::Raycast swaps the direction's X and Y instead of turning it a quarter"))
{
    // From (5, 0) to (8, 4).
    SegmentShape2DPtr segment = MakeSegment(5.0f, 0.0f, Vector2(0.6f, 0.8f), 5.0f);
    RaycastReport2D report;
    CHECK(segment->Raycast(Vector2(0.0f, 1.0f), Vector2(1.0f, 0.0f), 20.0f, report, nullptr));
    CHECK(Near(report.ImpactDistance, 5.75f));
    CHECK(Near(report.ImpactPoint, Vector2(5.75f, 1.0f)));
    CHECK(Near(report.ImpactNormal, Vector2(-0.8f, 0.6f)));
}

TEST_CASE("A box hands its scaled extension to a user shape")
{
    BoxShape2DPtr box = MakeStandingBox(3.0f, 4.0f);
    box->SetScale(2.0f);
    SharedPointer<RecordingShape> user = new RecordingShape();
    CHECK(Shape2D::Intersect(box, user));
    CHECK(user->LastCenter == Vector2(3.0f, 4.0f));
    CHECK(user->LastExtension == Vector2(4.0f, 2.0f));
}
