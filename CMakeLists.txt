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

# Bundled zlib 1.2.8 and libpng 1.6.9, built from the same sources as
# build/vs20xx/zlib.vcxproj and libpng.vcxproj. PNG.cpp reads libpng internals
# (pngstruct.h), so the system libpng cannot be used.
set(BIBIM_ZLIB_DIR ${BIBIM_EXTLIBS}/zlib-1.2.8)
add_library(zlib STATIC
    ${BIBIM_ZLIB_DIR}/adler32.c
    ${BIBIM_ZLIB_DIR}/compress.c
    ${BIBIM_ZLIB_DIR}/crc32.c
    ${BIBIM_ZLIB_DIR}/deflate.c
    ${BIBIM_ZLIB_DIR}/gzclose.c
    ${BIBIM_ZLIB_DIR}/gzlib.c
    ${BIBIM_ZLIB_DIR}/gzread.c
    ${BIBIM_ZLIB_DIR}/gzwrite.c
    ${BIBIM_ZLIB_DIR}/infback.c
    ${BIBIM_ZLIB_DIR}/inffast.c
    ${BIBIM_ZLIB_DIR}/inflate.c
    ${BIBIM_ZLIB_DIR}/inftrees.c
    ${BIBIM_ZLIB_DIR}/trees.c
    ${BIBIM_ZLIB_DIR}/uncompr.c
    ${BIBIM_ZLIB_DIR}/zutil.c
)
target_include_directories(zlib PUBLIC ${BIBIM_ZLIB_DIR})
# zconf.h only includes <unistd.h> (lseek, read, write in gz*.c) when told to.
target_compile_definitions(zlib PRIVATE Z_HAVE_UNISTD_H)
# zlib 1.2.8 uses K&R function definitions, which C23 (GCC 15's default)
# removed; build it as the C89 it was written in.
set_target_properties(zlib PROPERTIES C_STANDARD 90 C_EXTENSIONS ON)

set(BIBIM_LIBPNG_DIR ${BIBIM_EXTLIBS}/libpng-1.6.9)
add_library(png STATIC
    ${BIBIM_LIBPNG_DIR}/png.c
    ${BIBIM_LIBPNG_DIR}/pngerror.c
    ${BIBIM_LIBPNG_DIR}/pngget.c
    ${BIBIM_LIBPNG_DIR}/pngmem.c
    ${BIBIM_LIBPNG_DIR}/pngpread.c
    ${BIBIM_LIBPNG_DIR}/pngread.c
    ${BIBIM_LIBPNG_DIR}/pngrio.c
    ${BIBIM_LIBPNG_DIR}/pngrtran.c
    ${BIBIM_LIBPNG_DIR}/pngrutil.c
    ${BIBIM_LIBPNG_DIR}/pngset.c
    ${BIBIM_LIBPNG_DIR}/pngtrans.c
    ${BIBIM_LIBPNG_DIR}/pngwio.c
    ${BIBIM_LIBPNG_DIR}/pngwrite.c
    ${BIBIM_LIBPNG_DIR}/pngwtran.c
    ${BIBIM_LIBPNG_DIR}/pngwutil.c
)
target_include_directories(png PUBLIC ${BIBIM_LIBPNG_DIR})
target_link_libraries(png PUBLIC zlib m)

# Bundled libjpeg 9a, built from the same sources as build/vs20xx/libjpeg.vcxproj
# (jmemansi.c is the portable memory manager). jconfig.h includes the generic
# jconfig.txt.
set(BIBIM_LIBJPEG_DIR ${BIBIM_EXTLIBS}/jpeg-9a)
add_library(jpeg STATIC
    ${BIBIM_LIBJPEG_DIR}/jaricom.c
    ${BIBIM_LIBJPEG_DIR}/jcapimin.c
    ${BIBIM_LIBJPEG_DIR}/jcapistd.c
    ${BIBIM_LIBJPEG_DIR}/jcarith.c
    ${BIBIM_LIBJPEG_DIR}/jccoefct.c
    ${BIBIM_LIBJPEG_DIR}/jccolor.c
    ${BIBIM_LIBJPEG_DIR}/jcdctmgr.c
    ${BIBIM_LIBJPEG_DIR}/jchuff.c
    ${BIBIM_LIBJPEG_DIR}/jcinit.c
    ${BIBIM_LIBJPEG_DIR}/jcmainct.c
    ${BIBIM_LIBJPEG_DIR}/jcmarker.c
    ${BIBIM_LIBJPEG_DIR}/jcmaster.c
    ${BIBIM_LIBJPEG_DIR}/jcomapi.c
    ${BIBIM_LIBJPEG_DIR}/jcparam.c
    ${BIBIM_LIBJPEG_DIR}/jcprepct.c
    ${BIBIM_LIBJPEG_DIR}/jcsample.c
    ${BIBIM_LIBJPEG_DIR}/jctrans.c
    ${BIBIM_LIBJPEG_DIR}/jdapimin.c
    ${BIBIM_LIBJPEG_DIR}/jdapistd.c
    ${BIBIM_LIBJPEG_DIR}/jdarith.c
    ${BIBIM_LIBJPEG_DIR}/jdatadst.c
    ${BIBIM_LIBJPEG_DIR}/jdatasrc.c
    ${BIBIM_LIBJPEG_DIR}/jdcoefct.c
    ${BIBIM_LIBJPEG_DIR}/jdcolor.c
    ${BIBIM_LIBJPEG_DIR}/jddctmgr.c
    ${BIBIM_LIBJPEG_DIR}/jdhuff.c
    ${BIBIM_LIBJPEG_DIR}/jdinput.c
    ${BIBIM_LIBJPEG_DIR}/jdmainct.c
    ${BIBIM_LIBJPEG_DIR}/jdmarker.c
    ${BIBIM_LIBJPEG_DIR}/jdmaster.c
    ${BIBIM_LIBJPEG_DIR}/jdmerge.c
    ${BIBIM_LIBJPEG_DIR}/jdpostct.c
    ${BIBIM_LIBJPEG_DIR}/jdsample.c
    ${BIBIM_LIBJPEG_DIR}/jdtrans.c
    ${BIBIM_LIBJPEG_DIR}/jerror.c
    ${BIBIM_LIBJPEG_DIR}/jfdctflt.c
    ${BIBIM_LIBJPEG_DIR}/jfdctfst.c
    ${BIBIM_LIBJPEG_DIR}/jfdctint.c
    ${BIBIM_LIBJPEG_DIR}/jidctflt.c
    ${BIBIM_LIBJPEG_DIR}/jidctfst.c
    ${BIBIM_LIBJPEG_DIR}/jidctint.c
    ${BIBIM_LIBJPEG_DIR}/jmemansi.c
    ${BIBIM_LIBJPEG_DIR}/jmemmgr.c
    ${BIBIM_LIBJPEG_DIR}/jquant1.c
    ${BIBIM_LIBJPEG_DIR}/jquant2.c
    ${BIBIM_LIBJPEG_DIR}/jutils.c
)
target_include_directories(jpeg PUBLIC ${BIBIM_LIBJPEG_DIR})

# Bundled FreeType 2.5.2, built like build/vs20xx/freetype.vcxproj: one
# translation unit (custom/freetype-all.c) with the modules the engine uses.
# custom/ comes first so its config/ftoption.h and config/ftmodule.h replace
# the stock ones in include/.
set(BIBIM_FREETYPE_DIR ${BIBIM_EXTLIBS}/freetype-2.5.2)
add_library(freetype STATIC ${BIBIM_FREETYPE_DIR}/custom/freetype-all.c)
target_include_directories(freetype PUBLIC
    ${BIBIM_FREETYPE_DIR}/custom
    ${BIBIM_FREETYPE_DIR}/include
)
target_compile_definitions(freetype PRIVATE FT2_BUILD_LIBRARY)

add_library(Bibim STATIC
    ${BIBIM_SOURCES}
    ${BIBIM_EXTLIBS}/lua_tinker/lua_tinker.cpp
)
target_link_libraries(Bibim
    PUBLIC lua
    PRIVATE png jpeg freetype
)

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
        ${BIBIM_EXTLIBS}/StormLib/src
)

# Config.h derives BIBIM_DEBUG from _DEBUG, as on Windows.
target_compile_definitions(Bibim PUBLIC $<$<CONFIG:Debug>:_DEBUG>)

option(BIBIM_BUILD_TESTS "Build the engine unit tests (tests/)" ON)
if(BIBIM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
