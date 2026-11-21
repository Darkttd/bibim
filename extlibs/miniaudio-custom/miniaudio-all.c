/* miniaudio decodes Ogg Vorbis through stb_vorbis when the stb_vorbis
   declarations precede the miniaudio implementation. */
#define STB_VORBIS_HEADER_ONLY
#include "../miniaudio/extras/stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION
#include "../miniaudio/miniaudio.h"

#undef STB_VORBIS_HEADER_ONLY
#include "../miniaudio/extras/stb_vorbis.c"
