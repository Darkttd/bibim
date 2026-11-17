# Unit tests for the engine core. Run them with `ctest` from the build directory.
add_executable(BibimTests
    main.cpp
    Animation/EaseTest.cpp
    Foundation/Geom2DTest.cpp
    Foundation/MathTest.cpp
    Foundation/NumericsTest.cpp
    Foundation/RectTest.cpp
    Foundation/StringTest.cpp
    Foundation/Vector2Test.cpp
    IO/BinaryReaderWriterTest.cpp
    Module/GameModuleFactoryTest.cpp
    Module/GameModuleTreeTest.cpp
)

target_link_libraries(BibimTests PRIVATE Bibim)
target_include_directories(BibimTests PRIVATE ${BIBIM_EXTLIBS}/doctest-2.5.3)

add_test(NAME BibimTests COMMAND BibimTests)
