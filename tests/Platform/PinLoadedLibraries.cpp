#include "Platform/PinLoadedLibraries.h"
#include <dlfcn.h>
#include <link.h>

namespace
{
    int PinLoadedLibrary(dl_phdr_info* info, size_t, void*)
    {
        if (info->dlpi_name != nullptr && info->dlpi_name[0] != '\0')
            dlopen(info->dlpi_name, RTLD_NOW | RTLD_NOLOAD);
        return 0;
    }
}

void PinLoadedLibraries()
{
    dl_iterate_phdr(PinLoadedLibrary, nullptr);
}
