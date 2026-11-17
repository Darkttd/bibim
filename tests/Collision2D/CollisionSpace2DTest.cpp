// Tests for CollisionSpace2D: shape groups, which group pairs are tested,
// collision events and raycasts.
//
// The space holds its shapes in SharedPointers, so shapes created with new are
// released with the space.
//
// Known bug, not tested because it crashes: CollisionSpace2D::SetGroup erases
// the shape from its old group before adding it to the new one. When the space
// holds the only reference, the erase deletes the shape and the add then uses
// freed memory (found by ASan). The test below keeps its own reference.
#include <Bibim/Config.h>
#include <Bibim/CollisionSpace2D.h>
#include <Bibim/RaycastReport2D.h>
#include <Bibim/SphereShape2D.h>
#include <doctest.h>
#include <cmath>
#include <utility>
#include <vector>

using namespace Bibim;

namespace
{
    SphereShape2D* MakeSphere(float x, float y, float radius)
    {
        SphereShape2D* shape = new SphereShape2D();
        shape->SetPosition(Vector2(x, y));
        shape->SetRadius(radius);
        return shape;
    }

    class RecordingListener : public CollisionSpace2D::IEventListener
    {
        public:
            struct Event
            {
                Shape2D* A;
                Shape2D* B;
                int GroupA;
                int GroupB;
            };

            std::vector<Event> Events;

            virtual void OnIntersected(Shape2D* shapeA, Shape2D* shapeB, int groupA, int groupB)
            {
                Event e = { shapeA, shapeB, groupA, groupB };
                Events.push_back(e);
            }

            bool Has(Shape2D* x, Shape2D* y) const
            {
                for (std::vector<Event>::const_iterator it = Events.begin(); it != Events.end(); ++it)
                {
                    if ((it->A == x && it->B == y) || (it->A == y && it->B == x))
                        return true;
                }
                return false;
            }
    };
}

TEST_CASE("CollisionSpace2D keeps track of which group each shape is in")
{
    CollisionSpace2D space;
    space.SetNumberOfGroups(2);
    CHECK(space.GetNumberOfGroups() == 2);

    SphereShape2D* a = MakeSphere(0.0f, 0.0f, 1.0f);
    SphereShape2D* b = MakeSphere(0.0f, 0.0f, 1.0f);
    space.Add(a, 0);
    space.Add(b, 1);

    int group = -1;
    int index = -1;
    CHECK(space.Find(b, &group, &index));
    CHECK(group == 1);
    CHECK(index == 0);
    CHECK(space.GetGroup(a) == 0);

    const Shape2DPtr keepA = a; // see the SetGroup bug in the header
    space.SetGroup(a, 1);
    CHECK(space.GetGroup(a) == 1);
    CHECK(space.GetShapes(0).empty());
    CHECK(space.GetShapes(1).size() == 2);

    space.Remove(b);
    CHECK(space.GetGroup(b) == -1);
    CHECK_FALSE(space.Find(b, nullptr, nullptr));
    CHECK(space.GetShapes(1).size() == 1);
}

TEST_CASE("CollisionSpace2D::SetNumberOfGroups keeps the shapes and relationships of the groups that remain")
{
    CollisionSpace2D space;
    space.SetNumberOfGroups(2);
    space.SetCollisionRelationship(0, 1, true);
    CHECK(space.GetCollisionRelationship(1, 0)); // symmetric
    CHECK_FALSE(space.GetCollisionRelationship(0, 0));

    SphereShape2D* a = MakeSphere(0.0f, 0.0f, 1.0f);
    space.Add(a, 1);

    space.SetNumberOfGroups(3);
    CHECK(space.GetCollisionRelationship(0, 1));
    CHECK_FALSE(space.GetCollisionRelationship(2, 1));
    CHECK(space.GetGroup(a) == 1);

    space.SetNumberOfGroups(1);
    CHECK(space.GetNumberOfGroups() == 1);
    CHECK(space.GetGroup(a) == -1);
}

TEST_CASE("CollisionSpace2D::Detect reports intersecting pairs only between related groups")
{
    CollisionSpace2D space;
    space.SetNumberOfGroups(2);
    space.SetCollisionRelationship(0, 1, true);

    RecordingListener listener;
    space.AddListener(&listener);

    SphereShape2D* player = MakeSphere(0.0f, 0.0f, 1.0f);
    SphereShape2D* friendNearby = MakeSphere(0.5f, 0.0f, 1.0f);
    SphereShape2D* enemyTouching = MakeSphere(1.5f, 0.0f, 1.0f);
    SphereShape2D* enemyFar = MakeSphere(10.0f, 0.0f, 1.0f);
    space.Add(player, 0);
    space.Add(friendNearby, 0);
    space.Add(enemyTouching, 1);
    space.Add(enemyFar, 1);

    space.Detect();
    CHECK(listener.Events.size() == 2);
    CHECK(listener.Has(player, enemyTouching));
    CHECK(listener.Has(friendNearby, enemyTouching));
    CHECK_FALSE(listener.Has(player, friendNearby)); // group 0 is not related to itself
    for (std::vector<RecordingListener::Event>::const_iterator it = listener.Events.begin(); it != listener.Events.end(); ++it)
    {
        CHECK(it->GroupA == 0);
        CHECK(it->GroupB == 1);
    }

    listener.Events.clear();
    space.SetCollisionRelationship(0, 0, true);
    space.Detect();
    CHECK(listener.Events.size() == 3);
    CHECK(listener.Has(player, friendNearby));

    CHECK(space.RemoveListener(&listener));
    listener.Events.clear();
    space.Detect();
    CHECK(listener.Events.empty());
}

TEST_CASE("CollisionSpace2D::Raycast returns the closest hit among the related groups")
{
    CollisionSpace2D space;
    space.SetNumberOfGroups(3);
    space.SetCollisionRelationship(0, 1, true);

    SphereShape2D* near = MakeSphere(5.0f, 0.0f, 1.0f);
    SphereShape2D* far = MakeSphere(10.0f, 0.0f, 1.0f);
    SphereShape2D* unrelated = MakeSphere(2.0f, 0.0f, 1.0f);
    space.Add(far, 1);
    space.Add(near, 1);
    space.Add(unrelated, 2);

    RaycastReport2D report;
    CHECK(space.Raycast(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), 100.0f, 0, report));
    CHECK(report.ImpactShape == near);
    CHECK(std::fabs(report.ImpactDistance - 4.0f) < 0.0001f);

    CHECK_FALSE(space.Raycast(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), 3.0f, 0, report));
    CHECK_FALSE(space.Raycast(Vector2(0.0f, 0.0f), Vector2(0.0f, 1.0f), 100.0f, 0, report));
}
