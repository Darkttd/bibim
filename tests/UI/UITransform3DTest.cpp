#include <Bibim/Config.h>
#include <Bibim/Math.h>
#include <Bibim/Matrix4.h>
#include <Bibim/UITransform3D.h>
#include <doctest.h>

using namespace Bibim;

// Disabled until Bibim links FreeType and GLESv2 (#84, #85): under UBSan,
// -fsanitize=vptr pulls the UI classes, the GLES2 renderer and FreeType into
// the test binary through typeinfo references, and the link fails.
//
// TEST_CASE("UITransform3D TransformOrientation turns like the drawing matrix")
// {
//     const Vector3 rotations[] = {
//         Vector3(0.0f, 0.0f, 0.7f),
//         Vector3(0.3f, 0.5f, 0.7f),
//     };
//     const Vector3 directions[] = { Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3(1.0f, 2.0f, 3.0f) };
//     for (const Vector3& rotation : rotations)
//     {
//         UITransform3D transform;
//         transform.SetRotation(rotation);
//         const Matrix4 drawing = Matrix4::Transformation(Vector3::Zero, Vector3::One,
//                                                         Vector3::Zero, rotation,
//                                                         Vector3::Zero);
//         for (const Vector3& direction : directions)
//             CHECK(Math::Equals(transform.TransformOrientation(direction), drawing.TransformNormal(direction)));
//     }
// }
