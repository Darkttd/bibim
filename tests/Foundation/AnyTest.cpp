// Tests for Any.
//
// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
//
// Known bug, not tested because it crashes: CastPoint2 and CastVector2 on a
// Point2/Vector2 Any read through a pointer although the value is stored inline.
#include <Bibim/Config.h>
#include <Bibim/Any.h>
#include <Bibim/SharedObject.h>
#include <Bibim/SharedPointer.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    class TrackedObject : public SharedObject
    {
        public:
            explicit TrackedObject(bool* destroyed)
                : destroyed(destroyed)
            {
            }

            virtual ~TrackedObject()
            {
                *destroyed = true;
            }

        private:
            bool* destroyed;
    };
}

TEST_CASE("Any records the type of the value it was constructed from")
{
    int light = 0;
    CHECK(Any().GetType() == Any::VoidType);
    CHECK(Any(true).GetType() == Any::BoolType);
    CHECK(Any(1).GetType() == Any::IntType);
    CHECK(Any(static_cast<longint>(1)).GetType() == Any::LongIntType);
    CHECK(Any(1.0f).GetType() == Any::FloatType);
    CHECK(Any(Color(1, 2, 3, 4)).GetType() == Any::ColorType);
    CHECK(Any(Point2(1, 2)).GetType() == Any::Point2Type);
    CHECK(Any(Point3(1, 2, 3)).GetType() == Any::Point3Type);
    CHECK(Any(Point4(1, 2, 3, 4)).GetType() == Any::Point4Type);
    CHECK(Any(Vector2(1.0f, 2.0f)).GetType() == Any::Vector2Type);
    CHECK(Any(Vector3(1.0f, 2.0f, 3.0f)).GetType() == Any::Vector3Type);
    CHECK(Any(Vector4(1.0f, 2.0f, 3.0f, 4.0f)).GetType() == Any::Vector4Type);
    CHECK(Any("text").GetType() == Any::StringType);
    CHECK(Any(String("text")).GetType() == Any::StringType);
    CHECK(Any(static_cast<void*>(&light)).GetType() == Any::LightObjectType);

    CHECK(Any::Void.GetType() == Any::VoidType);
    CHECK(Any::True == Any(true));
    CHECK(Any::False == Any(false));
}

TEST_CASE("Any converts between bool, int, longint and float")
{
    CHECK(Any(true).CastInt() == 1);
    CHECK(Any(false).CastLongInt() == 0);
    CHECK(Any(true).CastFloat() == 1.0f);

    CHECK(Any(0).CastBool() == false);
    CHECK(Any(-3).CastBool() == true);
    CHECK(Any(-3).CastLongInt() == -3);
    CHECK(Any(-3).CastFloat() == -3.0f);

    const longint big = static_cast<longint>(1) << 40;
    CHECK(Any(big).CastBool() == true);
    CHECK(Any(big).CastLongInt() == big);
    CHECK(Any(big).CastFloat() == static_cast<float>(big));
    CHECK(Any(static_cast<longint>(7)).CastInt() == 7);

    CHECK(Any(0.0f).CastBool() == false);
    CHECK(Any(0.5f).CastBool() == true);
    CHECK(Any(-2.75f).CastInt() == -2);
    CHECK(Any(-2.75f).CastLongInt() == -2);
}

TEST_CASE("Casting an Any to an unrelated type gives the zero value")
{
    const Any text("12");
    CHECK(text.CastBool() == false);
    CHECK(text.CastInt() == 0);
    CHECK(text.CastLongInt() == 0);
    CHECK(text.CastFloat() == 0.0f);
    CHECK(text.CastColor() == Color(0, 0, 0, 0));
    CHECK(text.CastPoint2() == Point2::Zero);
    CHECK(text.CastPoint3() == Point3::Zero);
    CHECK(text.CastPoint4() == Point4::Zero);
    CHECK(text.CastVector2() == Vector2::Zero);
    CHECK(text.CastVector3() == Vector3::Zero);
    CHECK(text.CastVector4() == Vector4::Zero);
    CHECK(text.CastSharedObject() == nullptr);
    CHECK(text.CastLightObject() == nullptr);

    CHECK(Any().CastBool() == false);
    CHECK(Any().CastInt() == 0);
    CHECK(Any(1).CastString() == String::Empty);
    CHECK(Any(Point3(1, 2, 3)).CastPoint4() == Point4::Zero);
}

TEST_CASE("Any returns the stored Color, point, vector and string values")
{
    CHECK(Any(Color(10, 20, 30, 40)).CastColor() == Color(10, 20, 30, 40));
    CHECK(Any(Color(10, 20, 30, 255)).CastColor() == Color(10, 20, 30, 255));
    CHECK(Any(Point3(1, -2, 3)).CastPoint3() == Point3(1, -2, 3));
    CHECK(Any(Point4(1, -2, 3, -4)).CastPoint4() == Point4(1, -2, 3, -4));
    CHECK(Any(Vector3(1.5f, -2.5f, 3.5f)).CastVector3() == Vector3(1.5f, -2.5f, 3.5f));
    CHECK(Any(Vector4(1.5f, -2.5f, 3.5f, -4.5f)).CastVector4() == Vector4(1.5f, -2.5f, 3.5f, -4.5f));
    CHECK(Any("text").CastString() == "text");
}

TEST_CASE("Any equality needs the same type and the same value")
{
    CHECK(Any() == Any());
    CHECK(Any(1) == Any(1));
    CHECK(Any(1) != Any(2));
    CHECK(Any(1) != Any(1.0f));
    CHECK(Any(1) != Any(static_cast<longint>(1)));
    CHECK(Any(true) != Any(1));
    CHECK(Any(Color(1, 2, 3, 255)) == Any(Color(1, 2, 3, 255)));
    CHECK(Any(Color(1, 2, 3, 4)) != Any(Color(1, 2, 3, 5)));
    CHECK(Any(Point2(1, 2)) == Any(Point2(1, 2)));
    CHECK(Any(Point2(1, 2)) != Any(Point2(1, 3)));
    CHECK(Any(Point3(1, 2, 3)) != Any(Point3(1, 2, 4)));
    CHECK(Any(Point4(1, 2, 3, 4)) != Any(Point4(1, 2, 3, 5)));
    CHECK(Any(Vector2(1.0f, 2.0f)) == Any(Vector2(1.0f, 2.0f)));
    CHECK(Any(Vector3(1.0f, 2.0f, 3.0f)) != Any(Vector3(1.0f, 2.0f, 4.0f)));
    CHECK(Any(Vector4(1.0f, 2.0f, 3.0f, 4.0f)) == Any(Vector4(1.0f, 2.0f, 3.0f, 4.0f)));
    CHECK(Any("a") == Any(String("a")));
    CHECK(Any("a") != Any("b"));

    int a = 0;
    int b = 0;
    CHECK(Any(static_cast<void*>(&a)) == Any(static_cast<void*>(&a)));
    CHECK(Any(static_cast<void*>(&a)) != Any(static_cast<void*>(&b)));
}

TEST_CASE("Any copies are independent of the original")
{
    Any original("first");
    Any copy(original);
    CHECK(copy == original);

    original = Any("second");
    CHECK(copy.CastString() == "first");
    CHECK(original.CastString() == "second");

    Any vector(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
    Any vectorCopy(vector);
    vector = Any(Vector4::Zero);
    CHECK(vectorCopy.CastVector4() == Vector4(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST_CASE("Any assignment replaces the value and type")
{
    Any value("text");
    value = Any(Point3(1, 2, 3));
    CHECK(value.GetType() == Any::Point3Type);
    CHECK(value.CastPoint3() == Point3(1, 2, 3));

    value = Any(Vector3(1.0f, 2.0f, 3.0f));
    CHECK(value.CastVector3() == Vector3(1.0f, 2.0f, 3.0f));

    value = Any(5);
    CHECK(value.GetType() == Any::IntType);
    CHECK(value.CastInt() == 5);

    value = Any("again");
    const Any& same = value;
    value = same;
    CHECK(value.CastString() == "again");
}

TEST_CASE("Any keeps a SharedObject alive until the last copy is destroyed")
{
    bool destroyed = false;
    {
        Any first(static_cast<SharedObject*>(new TrackedObject(&destroyed)));
        CHECK(first.GetType() == Any::SharedObjectType);
        CHECK(first.CastBool() == true);
        CHECK(first.CastLightObject() == first.CastSharedObject());
        {
            const Any second(first);
            CHECK(second == first);
            first = Any();
            CHECK(destroyed == false);
            CHECK(second.CastSharedObject() != nullptr);
        }
        CHECK(destroyed == true);
    }

    CHECK(Any(static_cast<SharedObject*>(nullptr)).CastBool() == false);
}

TEST_CASE("Any holds a LightObject as a plain pointer")
{
    int target = 0;
    const Any light(static_cast<void*>(&target));
    CHECK(light.CastLightObject() == &target);
    CHECK(light.CastBool() == true);
    CHECK(light.CastSharedObject() == nullptr);
    CHECK(Any(static_cast<void*>(nullptr)).CastBool() == false);
}

TEST_CASE("Any::SizeOf gives the byte size of fixed-size types")
{
    CHECK(Any::SizeOf(Any::VoidType) == 0);
    CHECK(Any::SizeOf(Any::BoolType) == 1);
    CHECK(Any::SizeOf(Any::IntType) == 4);
    CHECK(Any::SizeOf(Any::LongIntType) == 8);
    CHECK(Any::SizeOf(Any::FloatType) == 4);
    CHECK(Any::SizeOf(Any::ColorType) == 4);
    CHECK(Any::SizeOf(Any::Point2Type) == 8);
    CHECK(Any::SizeOf(Any::Point3Type) == 12);
    CHECK(Any::SizeOf(Any::Point4Type) == 16);
    CHECK(Any::SizeOf(Any::Vector2Type) == 8);
    CHECK(Any::SizeOf(Any::Vector3Type) == 12);
    CHECK(Any::SizeOf(Any::Vector4Type) == 16);
    CHECK(Any::SizeOf(Any::StringType) == 0);
    CHECK(Any::SizeOf(Any::SharedObjectType) == 0);
    CHECK(Any::SizeOf(Any::LightObjectType) == 0);
}

TEST_CASE("Fixed-size Any values except Color survive WriteToBytes and ReadFromBytes")
{
    const Any values[] =
    {
        Any(true),
        Any(-123456),
        Any(-(static_cast<longint>(1) << 40)),
        Any(-2.5f),
        Any(Point2(1, -2)),
        Any(Point3(1, -2, 3)),
        Any(Point4(1, -2, 3, -4)),
        Any(Vector2(1.5f, -2.5f)),
        Any(Vector3(1.5f, -2.5f, 3.5f)),
        Any(Vector4(1.5f, -2.5f, 3.5f, -4.5f)),
    };

    for (const Any& value : values)
    {
        CAPTURE(value.GetType());
        alignas(8) byte buffer[16] = { 0 };
        CHECK(Any::WriteToBytes(buffer, value) == Any::SizeOf(value.GetType()));
        CHECK(Any::ReadFromBytes(buffer, value.GetType()) == value);
    }

    alignas(8) byte buffer[16] = { 0 };
    CHECK(Any::WriteToBytes(buffer, Any()) == 0);
    CHECK(Any::ReadFromBytes(buffer, Any::VoidType) == Any::Void);
    CHECK(Any::WriteToBytes(buffer, Any("text")) == 0);
    CHECK(Any::ReadFromBytes(buffer, Any::StringType) == Any::Void);
}

// WriteToBytes writes a Color as its ARGB int, but ReadFromBytes reads the bytes
// as a Color struct (A, R, G, B), so they come back in reverse order.
TEST_CASE("A Color survives WriteToBytes and ReadFromBytes" * doctest::should_fail())
{
    alignas(8) byte buffer[4] = { 0 };
    const Any color(Color(10, 20, 30, 40));
    REQUIRE(Any::WriteToBytes(buffer, color) == 4);
    CHECK(Any::ReadFromBytes(buffer, Any::ColorType) == color);
}

TEST_CASE("Any::WriteToBytes converts between bool, int, longint and float")
{
    alignas(8) byte buffer[8] = { 0 };

    CHECK(Any::WriteToBytes(buffer, Any(true), Any::IntType) == 4);
    CHECK(Any::ReadFromBytes(buffer, Any::IntType) == Any(1));
    CHECK(Any::WriteToBytes(buffer, Any(true), Any::LongIntType) == 8);
    CHECK(Any::ReadFromBytes(buffer, Any::LongIntType) == Any(static_cast<longint>(1)));
    CHECK(Any::WriteToBytes(buffer, Any(false), Any::FloatType) == 4);
    CHECK(Any::ReadFromBytes(buffer, Any::FloatType) == Any(0.0f));

    CHECK(Any::WriteToBytes(buffer, Any(-7), Any::BoolType) == 1);
    CHECK(Any::ReadFromBytes(buffer, Any::BoolType) == Any(true));
    CHECK(Any::WriteToBytes(buffer, Any(-7), Any::LongIntType) == 8);
    CHECK(Any::ReadFromBytes(buffer, Any::LongIntType) == Any(static_cast<longint>(-7)));
    CHECK(Any::WriteToBytes(buffer, Any(-7), Any::FloatType) == 4);
    CHECK(Any::ReadFromBytes(buffer, Any::FloatType) == Any(-7.0f));

    CHECK(Any::WriteToBytes(buffer, Any(static_cast<longint>(0)), Any::BoolType) == 1);
    CHECK(Any::ReadFromBytes(buffer, Any::BoolType) == Any(false));
    CHECK(Any::WriteToBytes(buffer, Any(static_cast<longint>(9)), Any::IntType) == 4);
    CHECK(Any::ReadFromBytes(buffer, Any::IntType) == Any(9));
    CHECK(Any::WriteToBytes(buffer, Any(static_cast<longint>(9)), Any::FloatType) == 4);
    CHECK(Any::ReadFromBytes(buffer, Any::FloatType) == Any(9.0f));

    CHECK(Any::WriteToBytes(buffer, Any(2.75f), Any::BoolType) == 1);
    CHECK(Any::ReadFromBytes(buffer, Any::BoolType) == Any(true));
    CHECK(Any::WriteToBytes(buffer, Any(2.75f), Any::IntType) == 4);
    CHECK(Any::ReadFromBytes(buffer, Any::IntType) == Any(2));
    CHECK(Any::WriteToBytes(buffer, Any(-2.75f), Any::LongIntType) == 8);
    CHECK(Any::ReadFromBytes(buffer, Any::LongIntType) == Any(static_cast<longint>(-2)));

    CHECK(Any::WriteToBytes(buffer, Any(3), Any::IntType) == 4);
    CHECK(Any::ReadFromBytes(buffer, Any::IntType) == Any(3));
}

TEST_CASE("Any::WriteToBytes writes nothing for conversions it does not support")
{
    alignas(8) byte buffer[16] = { 0 };
    CHECK(Any::WriteToBytes(buffer, Any(), Any::IntType) == 0);
    CHECK(Any::WriteToBytes(buffer, Any(1), Any::StringType) == 0);
    CHECK(Any::WriteToBytes(buffer, Any(1), Any::Vector2Type) == 0);
    CHECK(Any::WriteToBytes(buffer, Any(Color(1, 2, 3, 4)), Any::IntType) == 0);
    CHECK(Any::WriteToBytes(buffer, Any(Point2(1, 2)), Any::Vector2Type) == 0);
    CHECK(Any::WriteToBytes(buffer, Any(Vector3::Zero), Any::Point3Type) == 0);
    CHECK(Any::WriteToBytes(buffer, Any("1"), Any::IntType) == 0);
}
