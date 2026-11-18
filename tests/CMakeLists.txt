# Unit tests for the engine core. Run them with `ctest` from the build directory.
add_executable(BibimTests
    main.cpp
    Animation/EaseTest.cpp
    Collision2D/CollisionSpace2DTest.cpp
    Collision2D/Shape2DTest.cpp
    Diagnostics/SystemLoggerTest.cpp
    Foundation/ColorTest.cpp
    Graphics/ImageReadTest.cpp
    Foundation/Geom2DTest.cpp
    Foundation/MathTest.cpp
    Foundation/Matrix4Test.cpp
    Foundation/NumericsTest.cpp
    Foundation/PointTest.cpp
    Foundation/RectTest.cpp
    Foundation/StringTest.cpp
    Foundation/Vector2Test.cpp
    Foundation/Vector3Test.cpp
    Foundation/Vector4Test.cpp
    IO/BinaryReaderWriterTest.cpp
    Module/GameModuleFactoryTest.cpp
    Module/GameModuleTreeTest.cpp
    UI/UITransform3DTest.cpp
)

target_link_libraries(BibimTests PRIVATE Bibim)
target_include_directories(BibimTests PRIVATE ${BIBIM_EXTLIBS}/doctest-2.5.3)

add_test(NAME BibimTests COMMAND BibimTests)
