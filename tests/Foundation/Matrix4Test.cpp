// Tests for Matrix4.
//
// Matrix4 follows the Direct3D layout: vectors are rows multiplied on the left
// (v * M), the translation is in the last row (M30, M31, M32), and the floats
// are stored row by row (M00, M01, M02, M03, M10, ...). So A * B applies A
// first and B second.
//
// Test cases state the intended behavior. A case marked doctest::should_fail
// documents a known bug: it is expected to fail until the bug is fixed, and then
// the marker must be removed (doctest reports a should_fail case that passes).
#include <Bibim/Config.h>
#include <Bibim/Math.h>
#include <Bibim/Matrix4.h>
#include <Bibim/Vector4.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    bool NearlyEquals(const Matrix4& a, const Matrix4& b)
    {
        const float* x = a;
        const float* y = b;
        for (int i = 0; i < 16; i++)
        {
            if (!Math::Equals(x[i], y[i]))
                return false;
        }
        return true;
    }

    // v * M with the full fourth column, which Matrix4::Transform drops.
    Vector4 TransformHomogeneous(const Matrix4& m, Vector4 v)
    {
        return Vector4(v.X * m.M00 + v.Y * m.M10 + v.Z * m.M20 + v.W * m.M30,
                       v.X * m.M01 + v.Y * m.M11 + v.Z * m.M21 + v.W * m.M31,
                       v.X * m.M02 + v.Y * m.M12 + v.Z * m.M22 + v.W * m.M32,
                       v.X * m.M03 + v.Y * m.M13 + v.Z * m.M23 + v.W * m.M33);
    }

    const Matrix4 Sample(1.0f,  2.0f,  3.0f,  4.0f,
                         5.0f,  6.0f,  7.0f,  8.0f,
                         9.0f,  10.0f, 11.0f, 12.0f,
                         13.0f, 14.0f, 15.0f, 16.0f);
}

TEST_CASE("Matrix4 stores its values row by row")
{
    const float* values = Sample;
    for (int i = 0; i < 16; i++)
        CHECK(values[i] == static_cast<float>(i + 1));

    CHECK(Matrix4(values) == Sample);
    CHECK(Matrix4(Sample) == Sample);
    CHECK(Sample.GetTranslation() == Vector3(13.0f, 14.0f, 15.0f));
}

TEST_CASE("Matrix4 arithmetic works per element")
{
    const Matrix4 twice(2.0f,  4.0f,  6.0f,  8.0f,
                        10.0f, 12.0f, 14.0f, 16.0f,
                        18.0f, 20.0f, 22.0f, 24.0f,
                        26.0f, 28.0f, 30.0f, 32.0f);
    CHECK(Sample + Sample == twice);
    CHECK(twice - Sample == Sample);
    CHECK(Sample * 2.0f == twice);
    CHECK(2.0f * Sample == twice);
    CHECK(-Sample + Sample == Matrix4::Zero);
    CHECK(+Sample == Sample);
    CHECK(Sample != twice);

    Matrix4 m = Sample;
    m += Sample;
    CHECK(m == twice);
    m -= Sample;
    CHECK(m == Sample);
    m *= 2.0f;
    CHECK(m == twice);
}

TEST_CASE("Matrix4 product follows the row-by-column rule")
{
    const Matrix4 a(1.0f, 2.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f);
    const Matrix4 b(1.0f, 0.0f, 0.0f, 0.0f,
                    3.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f);
    CHECK(a * b == Matrix4(7.0f, 2.0f, 0.0f, 0.0f,
                           3.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           0.0f, 0.0f, 0.0f, 1.0f));
    CHECK(b * a == Matrix4(1.0f, 2.0f, 0.0f, 0.0f,
                           3.0f, 7.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           0.0f, 0.0f, 0.0f, 1.0f));

    CHECK(Sample * Matrix4::Identity == Sample);
    CHECK(Matrix4::Identity * Sample == Sample);

    Matrix4 m = a;
    m *= b;
    CHECK(m == a * b);
}

TEST_CASE("Matrix4 Transform applies translation and TransformNormal does not")
{
    const Matrix4 translation = Matrix4::Translation(Vector3(10.0f, 20.0f, 30.0f));
    CHECK(translation.Transform(Vector3(1.0f, 2.0f, 3.0f)) == Vector3(11.0f, 22.0f, 33.0f));
    CHECK(translation.TransformNormal(Vector3(1.0f, 2.0f, 3.0f)) == Vector3(1.0f, 2.0f, 3.0f));
    CHECK(translation.GetTranslation() == Vector3(10.0f, 20.0f, 30.0f));

    CHECK(Matrix4::Scaling(2.0f).Transform(Vector3(1.0f, 2.0f, 3.0f)) == Vector3(2.0f, 4.0f, 6.0f));
    CHECK(Matrix4::Scaling(Vector3(2.0f, 3.0f, 4.0f)).Transform(Vector3(1.0f, 1.0f, 1.0f)) == Vector3(2.0f, 3.0f, 4.0f));
}

TEST_CASE("Matrix4 product applies the left matrix first")
{
    const Matrix4 scale = Matrix4::Scaling(2.0f);
    const Matrix4 move = Matrix4::Translation(Vector3(1.0f, 0.0f, 0.0f));
    const Vector3 p(1.0f, 1.0f, 1.0f);

    CHECK((scale * move).Transform(p) == move.Transform(scale.Transform(p)));
    CHECK((scale * move).Transform(p) == Vector3(3.0f, 2.0f, 2.0f));
    CHECK((move * scale).Transform(p) == Vector3(4.0f, 2.0f, 2.0f));
}

TEST_CASE("Matrix4 RotationX, RotationY and RotationZ turn by the negative angle")
{
    // With row vectors these matrices are the transpose of D3DXMatrixRotation*,
    // so a positive angle turns the opposite way. Matrix4::Transformation
    // passes the negated angles to them to get the usual direction.
    const float quarter = Math::PiOver2;
    CHECK(Math::Equals(Matrix4::RotationX(quarter).Transform(Vector3::UnitY), -Vector3::UnitZ));
    CHECK(Math::Equals(Matrix4::RotationY(quarter).Transform(Vector3::UnitZ), -Vector3::UnitX));
    CHECK(Math::Equals(Matrix4::RotationZ(quarter).Transform(Vector3::UnitX), -Vector3::UnitY));

    CHECK(Math::Equals(Matrix4::RotationX(-quarter).Transform(Vector3::UnitY), Vector3::UnitZ));
    CHECK(Math::Equals(Matrix4::RotationY(-quarter).Transform(Vector3::UnitZ), Vector3::UnitX));
    CHECK(Math::Equals(Matrix4::RotationZ(-quarter).Transform(Vector3::UnitX), Vector3::UnitY));

    // The axis itself does not move.
    CHECK(Math::Equals(Matrix4::RotationZ(0.3f).Transform(Vector3::UnitZ), Vector3::UnitZ));
}

TEST_CASE("Matrix4 RotationPitchYawRoll about one axis matches Transformation")
{
    // About several axes they differ on purpose: RotationPitchYawRoll follows
    // D3DXMatrixRotationYawPitchRoll and turns about Z (roll), X (pitch), then
    // Y (yaw), while Transformation turns about X, Y, then Z.
    const float angle = 0.7f;
    const Vector3 rotations[] = {
        Vector3(angle, 0.0f, 0.0f),
        Vector3(0.0f, angle, 0.0f),
        Vector3(0.0f, 0.0f, angle),
    };
    for (const Vector3& rotation : rotations)
    {
        const Matrix4 transformation = Matrix4::Transformation(Vector3::Zero, Vector3::One,
                                                               Vector3::Zero, rotation,
                                                               Vector3::Zero);
        CHECK(NearlyEquals(Matrix4::RotationPitchYawRoll(rotation), transformation));
    }

    CHECK(NearlyEquals(Matrix4::RotationPitchYawRoll(Vector3(0.0f, 0.0f, angle)), Matrix4::RotationZ(-angle)));
}

TEST_CASE("Matrix4 Transformation scales and rotates about their centers, then translates")
{
    // Scaling by 2 about (1, 0, 0) keeps that point and doubles distances from it.
    const Matrix4 scaling = Matrix4::Transformation(Vector3(1.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f),
                                                    Vector3::Zero, Vector3::Zero,
                                                    Vector3::Zero);
    CHECK(scaling.Transform(Vector3(1.0f, 0.0f, 0.0f)) == Vector3(1.0f, 0.0f, 0.0f));
    CHECK(scaling.Transform(Vector3(2.0f, 1.0f, 0.0f)) == Vector3(3.0f, 2.0f, 0.0f));

    // A quarter turn about Z around (1, 1, 0), then a move by (0, 0, 5).
    const Matrix4 rotation = Matrix4::Transformation(Vector3::Zero, Vector3::One,
                                                     Vector3(1.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, Math::PiOver2),
                                                     Vector3(0.0f, 0.0f, 5.0f));
    CHECK(Math::Equals(rotation.Transform(Vector3(1.0f, 1.0f, 0.0f)), Vector3(1.0f, 1.0f, 5.0f)));
    CHECK(Math::Equals(rotation.Transform(Vector3(2.0f, 1.0f, 0.0f)), Vector3(1.0f, 2.0f, 5.0f)));
}

TEST_CASE("Matrix4 Transpose swaps rows and columns")
{
    const Matrix4 transposed = Matrix4::Transpose(Sample);
    CHECK(transposed == Matrix4(1.0f, 5.0f, 9.0f,  13.0f,
                                2.0f, 6.0f, 10.0f, 14.0f,
                                3.0f, 7.0f, 11.0f, 15.0f,
                                4.0f, 8.0f, 12.0f, 16.0f));
    CHECK(Matrix4::Transpose(transposed) == Sample);
}

TEST_CASE("Matrix4 Inversion undoes the matrix")
{
    const Matrix4 m = Matrix4::Scaling(Vector3(2.0f, 4.0f, 0.5f)) *
                      Matrix4::RotationZ(0.4f) *
                      Matrix4::RotationX(1.1f) *
                      Matrix4::Translation(Vector3(3.0f, -2.0f, 7.0f));
    const Matrix4 inverse = Matrix4::Inversion(m);
    CHECK(NearlyEquals(m * inverse, Matrix4::Identity));
    CHECK(NearlyEquals(inverse * m, Matrix4::Identity));

    const Vector3 p(1.0f, 2.0f, 3.0f);
    CHECK(Math::Equals(inverse.Transform(m.Transform(p)), p));

    CHECK(Matrix4::Inversion(Matrix4::Identity) == Matrix4::Identity);
    CHECK(Matrix4::Inversion(Matrix4::Translation(Vector3(1.0f, 2.0f, 3.0f))) ==
          Matrix4::Translation(Vector3(-1.0f, -2.0f, -3.0f)));
}

TEST_CASE("Matrix4 Inversion of a singular matrix is Zero")
{
    CHECK(Matrix4::Inversion(Sample) == Matrix4::Zero);
    CHECK(Matrix4::Inversion(Matrix4::Scaling(Vector3(1.0f, 0.0f, 1.0f))) == Matrix4::Zero);
}

TEST_CASE("Matrix4 LookAt puts the eye at the origin looking down +Z")
{
    const Vector3 eye(0.0f, 0.0f, -5.0f);
    const Matrix4 view = Matrix4::LookAt(eye, Vector3::Zero, Vector3::UnitY);
    CHECK(Math::Equals(view.Transform(eye), Vector3::Zero));
    CHECK(Math::Equals(view.Transform(Vector3::Zero), Vector3(0.0f, 0.0f, 5.0f)));
    CHECK(Math::Equals(view.Transform(Vector3(1.0f, 2.0f, 0.0f)), Vector3(1.0f, 2.0f, 5.0f)));

    // From the side: looking along +X, world +X becomes view +Z and world -Z
    // becomes view +X (left-handed).
    const Matrix4 side = Matrix4::LookAt(Vector3::Zero, Vector3::UnitX, Vector3::UnitY);
    CHECK(Math::Equals(side.Transform(Vector3::UnitX), Vector3::UnitZ));
    CHECK(Math::Equals(side.Transform(Vector3::UnitY), Vector3::UnitY));
    CHECK(Math::Equals(side.Transform(-Vector3::UnitZ), Vector3::UnitX));
}

TEST_CASE("Matrix4 PerspectiveFov maps the near and far planes to depth 0 and 1")
{
    const float nearZ = 1.0f;
    const float farZ = 101.0f;
    const Matrix4 projection = Matrix4::PerspectiveFov(Math::PiOver2, 2.0f, nearZ, farZ);

    const Vector4 nearPoint = TransformHomogeneous(projection, Vector4(0.0f, 0.0f, nearZ, 1.0f));
    const Vector4 farPoint = TransformHomogeneous(projection, Vector4(0.0f, 0.0f, farZ, 1.0f));
    CHECK(nearPoint.W == nearZ);
    CHECK(Math::Equals(nearPoint.Z / nearPoint.W, 0.0f));
    CHECK(farPoint.W == farZ);
    CHECK(Math::Equals(farPoint.Z / farPoint.W, 1.0f));

    // A 90 degree field of view reaches the top edge at y == z, and the
    // aspect ratio 2 reaches the right edge at x == 2z.
    const Vector4 topRight = TransformHomogeneous(projection, Vector4(20.0f, 10.0f, 10.0f, 1.0f));
    CHECK(Math::Equals(topRight.X / topRight.W, 1.0f));
    CHECK(Math::Equals(topRight.Y / topRight.W, 1.0f));
}

TEST_CASE("Matrix4 Ortho maps the view box to clip space with depth 0 to 1")
{
    const Matrix4 projection = Matrix4::Ortho(800.0f, 600.0f, 10.0f, 20.0f);
    CHECK(Math::Equals(projection.Transform(Vector3(400.0f, 300.0f, 10.0f)), Vector3(1.0f, 1.0f, 0.0f)));
    CHECK(Math::Equals(projection.Transform(Vector3(-400.0f, -300.0f, 20.0f)), Vector3(-1.0f, -1.0f, 1.0f)));
    CHECK(Math::Equals(projection.Transform(Vector3(0.0f, 0.0f, 15.0f)), Vector3(0.0f, 0.0f, 0.5f)));
}
