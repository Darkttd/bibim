# Unit tests for the engine core. Run them with `ctest` from the build directory.
add_executable(BibimTests
    main.cpp
    Animation/EaseTest.cpp
    Asset/FileAssetProviderTest.cpp
    Collision2D/CollisionSpace2DTest.cpp
    Collision2D/Shape2DTest.cpp
    Diagnostics/SystemLoggerTest.cpp
    FileSystem/FileStreamTest.cpp
    Foundation/ColorTest.cpp
    Foundation/EnvironmentTest.cpp
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
    Framework/PreferencesTest.cpp
    IO/BinaryReaderWriterTest.cpp
    Module/GameModuleFactoryTest.cpp
    Module/GameModuleTreeTest.cpp
    Platform/GameWindowTest.cpp
    Platform/SDL3Test.cpp
    UI/UITransform3DTest.cpp
)

target_link_libraries(BibimTests PRIVATE Bibim SDL3::SDL3)
target_include_directories(BibimTests PRIVATE ${BIBIM_EXTLIBS}/doctest-2.5.3)

add_test(NAME BibimTests COMMAND BibimTests --test-case-exclude=Environment*,Preferences*)
set_tests_properties(BibimTests PROPERTIES ENVIRONMENT
    "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/lsan.supp:print_suppressions=0")

# Environment reads the process environment once, so each setting runs in its
# own process. The data directories are created under the build directory.
set(BIBIM_TEST_DATA ${CMAKE_CURRENT_BINARY_DIR}/environment)
add_test(NAME EnvironmentKorean COMMAND BibimTests --test-case=Environment*)
set_tests_properties(EnvironmentKorean PROPERTIES ENVIRONMENT
    "LC_ALL=;LC_MESSAGES=;LANG=ko_KR.UTF-8;XDG_DATA_HOME=${BIBIM_TEST_DATA}/xdg;BIBIM_EXPECTED_LOCALE=ko-KR;BIBIM_EXPECTED_APPDATA=${BIBIM_TEST_DATA}/xdg")
add_test(NAME EnvironmentJapaneseOverride COMMAND BibimTests --test-case=Environment*)
set_tests_properties(EnvironmentJapaneseOverride PROPERTIES ENVIRONMENT
    "LC_ALL=ja_JP.UTF-8;LC_MESSAGES=;LANG=ko_KR.UTF-8;XDG_DATA_HOME=relative;HOME=${BIBIM_TEST_DATA}/home;BIBIM_EXPECTED_LOCALE=ja-JP;BIBIM_EXPECTED_APPDATA=${BIBIM_TEST_DATA}/home/.local/share")
add_test(NAME EnvironmentDefault COMMAND BibimTests --test-case=Environment*)
set_tests_properties(EnvironmentDefault PROPERTIES ENVIRONMENT
    "LC_ALL=;LC_MESSAGES=;LANG=C.UTF-8;XDG_DATA_HOME=;HOME=${BIBIM_TEST_DATA}/home;BIBIM_EXPECTED_LOCALE=en-US;BIBIM_EXPECTED_APPDATA=${BIBIM_TEST_DATA}/home/.local/share")
add_test(NAME PreferencesTests COMMAND BibimTests --test-case=Preferences*)
set_tests_properties(PreferencesTests PROPERTIES ENVIRONMENT
    "XDG_DATA_HOME=${BIBIM_TEST_DATA}/preferences")
