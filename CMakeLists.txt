# Linux build of the Bibim engine.
# Windows keeps using build/vs20xx, Android build/eclipse, iOS build/xcode6.
#
# Status (Linux port, phase 1): builds the platform-independent engine core
# as a static library. Sources that need the Linux platform layer
# (window, input, file system, audio) are listed in BIBIM_PENDING_PLATFORM_SOURCES
# and will be added as that layer lands.
cmake_minimum_required(VERSION 3.16)
project(Bibim LANGUAGES C CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This CMake build targets Linux only. Use build/vs20xx on Windows.")
endif()

# The GLES2 renderer includes the OpenGL ES 2.0 and EGL headers. Fail at
# configure time with a clear message instead of midway through the build.
find_path(BIBIM_GLES2_INCLUDE_DIR GLES2/gl2.h)
find_path(BIBIM_EGL_INCLUDE_DIR EGL/egl.h)
if(NOT BIBIM_GLES2_INCLUDE_DIR OR NOT BIBIM_EGL_INCLUDE_DIR)
    message(FATAL_ERROR
        "OpenGL ES 2.0 / EGL headers not found (GLES2/gl2.h, EGL/egl.h). "
        "Install them, e.g. 'libgles-dev libegl-dev' on Debian/Ubuntu or 'libglvnd' on Arch.")
endif()
include_directories(${BIBIM_GLES2_INCLUDE_DIR} ${BIBIM_EGL_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(BIBIM_EXTLIBS ${CMAKE_CURRENT_SOURCE_DIR}/extlibs)

file(GLOB BIBIM_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp)

# Other platforms' implementations. *.PC.cpp is the irrKlang audio backend,
# whose bundled Linux binary is 32-bit only; it gets replaced in a later phase.
list(FILTER BIBIM_SOURCES EXCLUDE REGEX "\\.(Windows|DX9|Android|iOS|Mobile|PC)\\.cpp$")

# Test bed references headers that no longer exist in the repository.
list(FILTER BIBIM_SOURCES EXCLUDE REGEX "/src/TestBed/")

# Need Linux implementations of GameFramework, GameWindow, FileStream,
# Environment or IME, which do not exist yet.
set(BIBIM_PENDING_PLATFORM_SOURCES
    Asset/FileAssetProviderBase.cpp
    Asset/NetworkAssetProvider.cpp
    Framework/Dashboard.cpp
    Framework/GameFrameworkBase.cpp
    Framework/Lua.cpp
    Framework/Preferences.cpp
    Framework/StandardGame.cpp
    Framework/Startup.cpp
    UI/UIEditText.cpp
)
foreach(pending ${BIBIM_PENDING_PLATFORM_SOURCES})
    list(REMOVE_ITEM BIBIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/${pending})
endforeach()

# Bundled Lua 5.2.3, built like build/vs20xx/lua.vcxproj: every source except the
# standalone interpreter and compiler. lua_tinker needs the 5.2 API, so the
# system Lua cannot be used. LUA_USE_POSIX picks the POSIX implementations
# (e.g. mkstemp instead of tmpnam), the Linux counterpart of Lua's defaults.
file(GLOB BIBIM_LUA_SOURCES ${BIBIM_EXTLIBS}/lua-5.2.3/src/*.c)
list(REMOVE_ITEM BIBIM_LUA_SOURCES
    ${BIBIM_EXTLIBS}/lua-5.2.3/src/lua.c
    ${BIBIM_EXTLIBS}/lua-5.2.3/src/luac.c
)
add_library(lua STATIC ${BIBIM_LUA_SOURCES})
target_include_directories(lua PUBLIC ${BIBIM_EXTLIBS}/lua-5.2.3/src)
target_compile_definitions(lua PRIVATE LUA_USE_POSIX)
target_link_libraries(lua PUBLIC m)

add_library(Bibim STATIC
    ${BIBIM_SOURCES}
    ${BIBIM_EXTLIBS}/lua_tinker/lua_tinker.cpp
)
target_link_libraries(Bibim PUBLIC lua)

# Same third-party header locations as build/vs20xx/Bibim.vcxproj, so every
# platform compiles against the same library versions. PNG.cpp reads libpng
# internals (pngstruct.h), so the bundled libpng is required.
target_include_directories(Bibim
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${BIBIM_EXTLIBS}/lua-5.2.3/src
        ${BIBIM_EXTLIBS}/lua_tinker
        ${BIBIM_EXTLIBS}/utf8_v2_3_4/source
        ${BIBIM_EXTLIBS}/rapidjson/include
    PRIVATE
        ${BIBIM_EXTLIBS}/zlib-1.2.8
        ${BIBIM_EXTLIBS}/libpng-1.6.9
        ${BIBIM_EXTLIBS}/jpeg-9a
        ${BIBIM_EXTLIBS}/freetype-2.5.2/custom
        ${BIBIM_EXTLIBS}/freetype-2.5.2/include
        ${BIBIM_EXTLIBS}/StormLib/src
)

# Config.h derives BIBIM_DEBUG from _DEBUG, as on Windows.
target_compile_definitions(Bibim PUBLIC $<$<CONFIG:Debug>:_DEBUG>)

option(BIBIM_BUILD_TESTS "Build the engine unit tests (tests/)" ON)
if(BIBIM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
