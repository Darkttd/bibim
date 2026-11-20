#include <Bibim/Config.h>
#if (defined(BIBIM_PLATFORM_PC))

#include <Bibim/MPQ.h>
#include <StormLib.h>

namespace Bibim
{
    MPQ::MPQ(const String& path)
        : handle(nullptr)
    {
        HANDLE internalHandle = NULL;
        if (SFileOpenArchive(path.CStr(), 0, STREAM_FLAG_READ_ONLY, &internalHandle))
            handle = internalHandle;
    }

    MPQ::~MPQ()
    {
        Close();
    }

    void MPQ::Close()
    {
        if (handle)
        {
            SFileCloseArchive(handle);
            handle = nullptr;
        }
    }

    bool MPQ::Has(const String& path) const
    {
        if (handle)
            return SFileHasFile(handle, ToArchivedName(path).CStr());
        else
            return false;
    }

    String MPQ::ToArchivedName(const String& path)
    {
        // Archives store names with backslashes. StormLib 9 no longer reads
        // a slash as a backslash, so asset names like "Asset/Shader" need it.
        String name = path;
        name.Replace('/', '\\');
        return name;
    }
}

#endif
