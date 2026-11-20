# Unit tests for the engine core. Run them with `ctest` from the build directory.
add_executable(BibimTests
    main.cpp
    Animation/EaseTest.cpp
    Asset/FileAssetProviderTest.cpp
    Audio/SilentAudioTest.cpp
    Collision2D/CollisionSpace2DTest.cpp
    Collision2D/Shape2DTest.cpp
    Common/TemporaryDirectory.cpp
    FileSystem/FileStreamTest.cpp
    FileSystem/MPQTest.cpp
    Foundation/ColorTest.cpp
    Foundation/ConfigTest.cpp
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
    Graphics/ImageReadTest.cpp
    IO/BinaryReaderWriterTest.cpp
    Module/GameModuleFactoryTest.cpp
    Module/GameModuleTreeTest.cpp
    UI/UITransform3DTest.cpp
)
target_link_libraries(BibimTests PRIVATE Bibim stormlib)
target_compile_options(BibimTests PRIVATE ${BIBIM_WARNING_OPTIONS})
target_include_directories(BibimTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${BIBIM_EXTLIBS}/doctest-2.5.3/doctest)

if(BIBIM_WINDOWS)
    add_test(NAME BibimTests COMMAND BibimTests)
    return()
endif()

# Linux only: the SDL3 platform layer, and tests that use POSIX file system
# and environment functions.
target_sources(BibimTests PRIVATE
    Diagnostics/SystemLoggerTest.cpp
    Foundation/EnvironmentTest.cpp
    Framework/GameFrameworkTest.cpp
    Framework/PreferencesTest.cpp
    Framework/StandardGameTest.cpp
    Input/IMETest.cpp
    Input/MouseTest.cpp
    Platform/GameWindowTest.cpp
    Platform/OffscreenVideo.cpp
    Platform/PinLoadedLibraries.cpp
    Platform/SDL3Test.cpp
)
target_link_libraries(BibimTests PRIVATE SDL3::SDL3)

add_test(NAME BibimTests COMMAND BibimTests --test-case-exclude=Environment*,GameFramework*,Preferences*,StandardGame*)

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
add_test(NAME GameFrameworkTests COMMAND BibimTests --test-case=GameFramework*)
add_test(NAME StandardGameTests COMMAND BibimTests --test-case=StandardGame*)
set_tests_properties(StandardGameTests PROPERTIES ENVIRONMENT
    "XDG_DATA_HOME=${BIBIM_TEST_DATA}/standardgame")

add_executable(BBMainSmoke BBMainSmoke.cpp Platform/PinLoadedLibraries.cpp)
target_link_libraries(BBMainSmoke PRIVATE Bibim)
target_compile_options(BBMainSmoke PRIVATE ${BIBIM_WARNING_OPTIONS})
target_include_directories(BBMainSmoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME BBMainSmoke COMMAND BBMainSmoke --flag "two words")
set_tests_properties(BBMainSmoke PROPERTIES ENVIRONMENT
    "SDL_VIDEO_DRIVER=offscreen;BIBIM_EXPECTED_STARTUP_ARGS=$<TARGET_FILE:BBMainSmoke> --flag two words")
