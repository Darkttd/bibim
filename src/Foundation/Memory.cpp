#include <Bibim/Config.h>
#include <Bibim/Memory.h>
#include <cstring>
#include <cstdlib>

namespace Bibim
{
    void* Memory::Alloc(int size)
    {
        return std::malloc(size);
    }

    void Memory::Free(void* pointer)
    {
        std::free(pointer);
    }

    void Memory::Fill(void* destination, int destinationLength)
    {
        if (destinationLength <= 0)
            return;

        std::memset(destination, 0, destinationLength);
    }

    void Memory::Copy(void* destination, int destinationLength, const void* source, int sourceLength)
    {
        if (sourceLength <= 0)
            return;

#       if (defined(BIBIM_PLATFORM_WINDOWS))
            ::memcpy_s(destination, destinationLength, source, sourceLength);
#       else
            BBUnusedParameter(destinationLength);
            std::memcpy(destination, source, sourceLength);
#       endif
    }
}
