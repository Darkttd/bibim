# CMake build of the Bibim engine for Linux and for 64-bit Windows with
# MinGW-w64 (natively or cross-compiled with cmake/mingw-w64-x86_64.cmake).
# The Visual Studio build stays in build/vs20xx, Android in build/eclipse and
# iOS in build/xcode6.
cmake_minimum_required(VERSION 3.16)
project(Bibim LANGUAGES C CXX)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(BIBIM_LINUX ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MINGW)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The MinGW-w64 build targets 64-bit Windows (x86_64) only.")
    endif()
    set(BIBIM_WINDOWS ON)
else()
    message(FATAL_ERROR
        "This CMake build targets Linux and MinGW-w64 on Windows. "
        "Use build/vs20xx for Visual Studio.")
endif()

if(BIBIM_LINUX)
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
# The GLES2 renderer calls gl* functions. No EGL function is called on Linux
# (only the Android code path does), so libEGL is not linked.
find_library(BIBIM_GLES2_LIBRARY GLESv2)
if(NOT BIBIM_GLES2_LIBRARY)
    message(FATAL_ERROR
        "OpenGL ES 2.0 library not found (libGLESv2). "
        "Install it, e.g. 'libgles-dev' on Debian/Ubuntu or 'libglvnd' on Arch.")
endif()

# SDL3 is the window, GLES2 context and input layer (*.SDL3.cpp, enabled by
# BIBIM_USE_SDL3 in Config.h). The system package is used.
find_package(SDL3 3.4 CONFIG)
if(NOT SDL3_FOUND)
    message(FATAL_ERROR
        "SDL3 3.4 or later not found. "
        "Install it, e.g. 'libsdl3-dev' on Debian/Ubuntu or 'sdl3' on Arch.")
endif()
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(BIBIM_EXTLIBS ${CMAKE_CURRENT_SOURCE_DIR}/extlibs)

file(GLOB BIBIM_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp)

# Other platforms' implementations.
if(BIBIM_LINUX)
    list(FILTER BIBIM_SOURCES EXCLUDE REGEX "\\.(Windows|DX9|Android|iOS|Mobile)\\.cpp$")
else()
    list(FILTER BIBIM_SOURCES EXCLUDE REGEX "\\.(Linux|Unix|SDL3|GLES2|Android|iOS|Mobile)\\.cpp$")
endif()

# Test bed references headers that no longer exist in the repository.
list(FILTER BIBIM_SOURCES EXCLUDE REGEX "/src/TestBed/")

# Bundled Lua 5.2.4, built like build/vs20xx/lua.vcxproj: every source except the
# standalone interpreter and compiler. lua_tinker needs the 5.2 API, so the
# system Lua cannot be used. LUA_USE_POSIX picks the POSIX implementations
# (e.g. mkstemp instead of tmpnam), the Linux counterpart of Lua's defaults.
file(GLOB BIBIM_LUA_SOURCES ${BIBIM_EXTLIBS}/lua-5.2.4/src/*.c)
list(REMOVE_ITEM BIBIM_LUA_SOURCES
    ${BIBIM_EXTLIBS}/lua-5.2.4/src/lua.c
    ${BIBIM_EXTLIBS}/lua-5.2.4/src/luac.c
)
add_library(lua STATIC ${BIBIM_LUA_SOURCES})
target_include_directories(lua PUBLIC ${BIBIM_EXTLIBS}/lua-5.2.4/src)
if(BIBIM_LINUX)
    target_compile_definitions(lua PRIVATE LUA_USE_POSIX)
    target_link_libraries(lua PUBLIC m)
endif()

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
if(BIBIM_LINUX)
    target_compile_definitions(zlib PRIVATE Z_HAVE_UNISTD_H)
endif()
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
target_include_directories(png PUBLIC ${BIBIM_LIBPNG_DIR} ${BIBIM_EXTLIBS}/libpng-config)
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
# The integer IDCT left-shifts negative coefficients (jidctint.c), which UBSan
# reports; GCC and Clang define it as an arithmetic shift.
target_compile_options(jpeg PRIVATE $<$<C_COMPILER_ID:GNU,Clang>:-fno-sanitize=shift-base>)

# Bundled FreeType 2.5.2, built like build/vs20xx/freetype.vcxproj: one
# translation unit (freetype-custom/freetype-all.c) with the modules the engine
# uses. freetype-custom/ comes first so its config/ftoption.h and
# config/ftmodule.h replace the stock ones in include/.
set(BIBIM_FREETYPE_DIR ${BIBIM_EXTLIBS}/freetype-2.5.2)
add_library(freetype STATIC ${BIBIM_EXTLIBS}/freetype-custom/freetype-all.c)
target_include_directories(freetype PUBLIC
    ${BIBIM_EXTLIBS}/freetype-custom
    ${BIBIM_FREETYPE_DIR}/include
)
target_compile_definitions(freetype PRIVATE FT2_BUILD_LIBRARY)

# Bundled StormLib 9.40 (MPQ archives), built from the source lists in its own
# CMakeLists.txt (SRC_FILES, TOMCRYPT_FILES, TOMMATH_FILES, BZIP2_FILES) with
# the same definitions: _7ZIP_ST (single-threaded LZMA) and BZ_STRICT_ANSI.
# __SYS_ZLIB uses the bundled zlib target instead of StormLib's copy.
set(BIBIM_STORMLIB_DIR ${BIBIM_EXTLIBS}/StormLib)
add_library(stormlib STATIC
    ${BIBIM_STORMLIB_DIR}/src/FileStream.cpp
    ${BIBIM_STORMLIB_DIR}/src/SBaseCommon.cpp
    ${BIBIM_STORMLIB_DIR}/src/SBaseDumpData.cpp
    ${BIBIM_STORMLIB_DIR}/src/SBaseFileTable.cpp
    ${BIBIM_STORMLIB_DIR}/src/SBaseSubTypes.cpp
    ${BIBIM_STORMLIB_DIR}/src/SCompression.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileAddFile.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileAttributes.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileCompactArchive.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileCreateArchive.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileExtractFile.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileFindFile.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileGetFileInfo.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileListFile.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileOpenArchive.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileOpenFileEx.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFilePatchArchives.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileReadFile.cpp
    ${BIBIM_STORMLIB_DIR}/src/SFileVerify.cpp
    ${BIBIM_STORMLIB_DIR}/src/SMemUtf8.cpp
    ${BIBIM_STORMLIB_DIR}/src/adpcm/adpcm.cpp
    ${BIBIM_STORMLIB_DIR}/src/bzip2/blocksort.c
    ${BIBIM_STORMLIB_DIR}/src/bzip2/bzlib.c
    ${BIBIM_STORMLIB_DIR}/src/bzip2/compress.c
    ${BIBIM_STORMLIB_DIR}/src/bzip2/crctable.c
    ${BIBIM_STORMLIB_DIR}/src/bzip2/decompress.c
    ${BIBIM_STORMLIB_DIR}/src/bzip2/huffman.c
    ${BIBIM_STORMLIB_DIR}/src/bzip2/randtable.c
    ${BIBIM_STORMLIB_DIR}/src/huffman/huff.cpp
    ${BIBIM_STORMLIB_DIR}/src/jenkins/lookup3.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/hashes/hash_memory.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/hashes/md5.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/hashes/sha1.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/hashes/sha256.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/math/ltm_desc.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/math/multi.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/math/rand_prime.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/base64_decode.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_argchk.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_find_hash.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_find_prng.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_hash_descriptor.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_hash_is_valid.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_libc.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_ltc_mp_descriptor.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_prng_descriptor.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_prng_is_valid.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_register_hash.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/crypt_register_prng.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/misc/zeromem.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_bit_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_boolean.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_choice.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_ia5_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_integer.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_object_identifier.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_octet_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_printable_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_sequence_ex.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_sequence_flexi.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_sequence_multi.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_short_integer.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_utctime.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_decode_utf8_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_bit_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_boolean.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_ia5_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_integer.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_object_identifier.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_octet_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_printable_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_sequence_ex.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_sequence_multi.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_set.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_setof.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_short_integer.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_utctime.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_encode_utf8_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_bit_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_boolean.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_ia5_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_integer.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_object_identifier.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_octet_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_printable_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_sequence.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_short_integer.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_utctime.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_length_utf8_string.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/asn1/der_sequence_free.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/ecc/ltc_ecc_map.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/ecc/ltc_ecc_mul2add.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/ecc/ltc_ecc_mulmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/ecc/ltc_ecc_points.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/ecc/ltc_ecc_projective_add_point.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/ecc/ltc_ecc_projective_dbl_point.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/pkcs1/pkcs_1_mgf1.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/pkcs1/pkcs_1_oaep_decode.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/pkcs1/pkcs_1_pss_decode.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/pkcs1/pkcs_1_pss_encode.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/pkcs1/pkcs_1_v1_5_decode.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/pkcs1/pkcs_1_v1_5_encode.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/rsa/rsa_exptmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/rsa/rsa_free.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/rsa/rsa_import.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/rsa/rsa_make_key.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/rsa/rsa_sign_hash.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/rsa/rsa_verify_hash.c
    ${BIBIM_STORMLIB_DIR}/src/libtomcrypt/src/pk/rsa/rsa_verify_simple.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_fast_mp_invmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_fast_mp_montgomery_reduce.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_fast_s_mp_mul_digs.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_fast_s_mp_mul_high_digs.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_fast_s_mp_sqr.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_2expt.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_abs.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_add.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_add_d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_addmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_and.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_clamp.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_clear.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_clear_multi.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_cmp.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_cmp_d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_cmp_mag.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_cnt_lsb.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_copy.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_count_bits.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_div.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_div_2.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_div_2d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_div_3.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_div_d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_dr_is_modulus.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_dr_reduce.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_dr_setup.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_exch.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_expt_d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_exptmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_exptmod_fast.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_exteuclid.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_fread.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_fwrite.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_gcd.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_get_int.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_grow.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_init.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_init_copy.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_init_multi.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_init_set.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_init_set_int.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_init_size.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_invmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_invmod_slow.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_is_square.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_jacobi.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_karatsuba_mul.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_karatsuba_sqr.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_lcm.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_lshd.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_mod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_mod_2d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_mod_d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_montgomery_calc_normalization.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_montgomery_reduce.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_montgomery_setup.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_mul.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_mul_2.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_mul_2d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_mul_d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_mulmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_n_root.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_neg.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_or.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_prime_fermat.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_prime_is_divisible.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_prime_is_prime.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_prime_miller_rabin.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_prime_next_prime.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_prime_rabin_miller_trials.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_prime_random_ex.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_radix_size.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_radix_smap.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_rand.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_read_radix.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_read_signed_bin.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_read_unsigned_bin.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_reduce.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_reduce_2k.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_reduce_2k_l.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_reduce_2k_setup.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_reduce_2k_setup_l.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_reduce_is_2k.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_reduce_is_2k_l.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_reduce_setup.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_rshd.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_set.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_set_int.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_shrink.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_signed_bin_size.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_sqr.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_sqrmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_sqrt.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_sub.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_sub_d.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_submod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_to_signed_bin.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_to_signed_bin_n.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_to_unsigned_bin.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_to_unsigned_bin_n.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_toom_mul.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_toom_sqr.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_toradix.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_toradix_n.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_unsigned_bin_size.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_xor.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_mp_zero.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_prime_tab.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_reverse.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_s_mp_add.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_s_mp_exptmod.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_s_mp_mul_digs.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_s_mp_mul_high_digs.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_s_mp_sqr.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bn_s_mp_sub.c
    ${BIBIM_STORMLIB_DIR}/src/libtommath/bncore.c
    ${BIBIM_STORMLIB_DIR}/src/lzma/C/LzFind.c
    ${BIBIM_STORMLIB_DIR}/src/lzma/C/LzmaDec.c
    ${BIBIM_STORMLIB_DIR}/src/lzma/C/LzmaEnc.c
    ${BIBIM_STORMLIB_DIR}/src/pklib/explode.c
    ${BIBIM_STORMLIB_DIR}/src/pklib/implode.c
    ${BIBIM_STORMLIB_DIR}/src/sparse/sparse.cpp
)
target_include_directories(stormlib PUBLIC ${BIBIM_STORMLIB_DIR}/src)
target_compile_definitions(stormlib PRIVATE _7ZIP_ST BZ_STRICT_ANSI __SYS_ZLIB)
target_link_libraries(stormlib PRIVATE zlib)
if(BIBIM_WINDOWS)
    # FileStream.cpp opens http:// archives with WinINet.
    target_link_libraries(stormlib PRIVATE wininet)
endif()

add_library(miniaudio STATIC ${BIBIM_EXTLIBS}/miniaudio-custom/miniaudio-all.c)
target_include_directories(miniaudio PUBLIC ${BIBIM_EXTLIBS}/miniaudio)
if(BIBIM_LINUX)
    # The audio backends (PulseAudio, ALSA, JACK) are loaded with dlopen.
    find_package(Threads REQUIRED)
    target_link_libraries(miniaudio PUBLIC Threads::Threads m ${CMAKE_DL_LIBS})
endif()

add_library(Bibim STATIC
    ${BIBIM_SOURCES}
    ${BIBIM_EXTLIBS}/lua_tinker/lua_tinker.cpp
)
target_link_libraries(Bibim
    PUBLIC lua
    PRIVATE png jpeg freetype stormlib miniaudio
)
if(BIBIM_LINUX)
    target_link_libraries(Bibim PRIVATE ${BIBIM_GLES2_LIBRARY} SDL3::SDL3)
else()
    target_link_libraries(Bibim PUBLIC d3d9 d3dx9 winmm ws2_32 imm32)
    # windows.h would otherwise define min and max macros (Config.h defines
    # NOMINMAX only for MSVC).
    target_compile_definitions(Bibim PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS)
    # Executables carry the MinGW runtime (libstdc++, libgcc, winpthread), so
    # they run without its DLLs next to them, on Windows and under Wine.
    add_link_options(-static)
endif()

# Same third-party header locations as build/vs20xx/Bibim.vcxproj, so every
# platform compiles against the same library versions. PNG.cpp reads libpng
# internals (pngstruct.h), so the bundled libpng is required.
target_include_directories(Bibim
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${BIBIM_EXTLIBS}/lua-5.2.4/src
        ${BIBIM_EXTLIBS}/lua_tinker
        ${BIBIM_EXTLIBS}/utf8_v2_3_4/source
)
target_include_directories(Bibim SYSTEM PUBLIC ${BIBIM_EXTLIBS}/rapidjson/include)

# Warnings for the engine's own code and its tests; the bundled libraries
# keep their defaults. The sources carry MSVC's #pragma warning on purpose.
set(BIBIM_WARNING_OPTIONS -Wall -Wextra -Wno-unknown-pragmas)
target_compile_options(Bibim PRIVATE ${BIBIM_WARNING_OPTIONS})

# Config.h derives BIBIM_DEBUG from _DEBUG, as on Windows.
target_compile_definitions(Bibim PUBLIC $<$<CONFIG:Debug>:_DEBUG>)

option(BIBIM_BUILD_TESTS "Build the engine unit tests (tests/)" ON)
if(BIBIM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
