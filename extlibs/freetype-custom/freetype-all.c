#include "../freetype-2.5.2/src/autofit/autofit.c"
/* #include "../freetype-2.5.2/src/bdf/bdf.c" */
/* #include "../freetype-2.5.2/src/cff/cff.c" */
#include "../freetype-2.5.2/src/base/ftbase.c"
#include "../freetype-2.5.2/src/base/ftbitmap.c"
/* #include "../freetype-2.5.2/src/cache/ftcache.c" */
#include "../freetype-2.5.2/src/base/ftfstype.c"
#include "../freetype-2.5.2/src/base/ftgasp.c"
#include "../freetype-2.5.2/src/base/ftglyph.c"
#if (defined(WIN32))
#include "../freetype-2.5.2/builds/windows/ftdebug.c"
#else
#include "../freetype-2.5.2/src/base/ftdebug.c"
#endif

/* zlib's inflate code in ftgzip.c has a table named border, which clashes with a
   macOS function. */
#define border ft_gzip_border
#include "../freetype-2.5.2/src/gzip/ftgzip.c"
#undef border
#undef bits
#undef exop
#undef local

#include "../freetype-2.5.2/src/base/ftinit.c"
#include "../freetype-2.5.2/src/lzw/ftlzw.c"
#include "../freetype-2.5.2/src/base/ftstroke.c"
#include "../freetype-2.5.2/src/base/ftsystem.c"
#include "../freetype-2.5.2/src/smooth/smooth.c"

// MODULES
#include "../freetype-2.5.2/src/base/ftbbox.c"
#include "../freetype-2.5.2/src/base/ftmm.c"
/* #include "../freetype-2.5.2/src/base/ftpfr.c" */
#include "../freetype-2.5.2/src/base/ftsynth.c"
/* #include "../freetype-2.5.2/src/base/fttype1.c" */
#include "../freetype-2.5.2/src/base/ftwinfnt.c"
#include "../freetype-2.5.2/src/base/ftxf86.c"
#include "../freetype-2.5.2/src/base/ftlcdfil.c"
#include "../freetype-2.5.2/src/base/ftgxval.c"
#include "../freetype-2.5.2/src/base/ftotval.c"
#include "../freetype-2.5.2/src/base/ftpatent.c"
/* #include "../freetype-2.5.2/src/pcf/pcf.c" */
/* #include "../freetype-2.5.2/src/pfr/pfr.c" */
#include "../freetype-2.5.2/src/psaux/psaux.c"
#include "../freetype-2.5.2/src/pshinter/pshinter.c"
#include "../freetype-2.5.2/src/psnames/psmodule.c"
#include "../freetype-2.5.2/src/raster/raster.c"
#include "../freetype-2.5.2/src/sfnt/sfnt.c"
#include "../freetype-2.5.2/src/truetype/truetype.c"
/* #include "../freetype-2.5.2/src/type1/type1.c" */
/* #include "../freetype-2.5.2/src/cid/type1cid.c" */
/* #include "../freetype-2.5.2/src/type42/type42.c" */
#include "../freetype-2.5.2/src/winfonts/winfnt.c"