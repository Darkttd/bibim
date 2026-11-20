#include "Common/TemporaryDirectory.h"
#include <Bibim/Config.h>
#include <doctest.h>

#if defined(BIBIM_PLATFORM_WINDOWS)
#   include <windows.h>
#else
#   include <cstdio>
#   include <ftw.h>
#   include <stdlib.h>
#endif

using namespace Bibim;

namespace
{
#if defined(BIBIM_PLATFORM_WINDOWS)
    void RemoveRecursively(const String& directory)
    {
        WIN32_FIND_DATAA data;
        HANDLE find = ::FindFirstFileA((directory + "/*").CStr(), &data);
        if (find != INVALID_HANDLE_VALUE)
        {
            do
            {
                const String name = data.cFileName;
                if (name == "." || name == "..")
                    continue;

                const String child = directory + "/" + name;
                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    RemoveRecursively(child);
                else
                    ::DeleteFileA(child.CStr());
            } while (::FindNextFileA(find, &data));
            ::FindClose(find);
        }
        ::RemoveDirectoryA(directory.CStr());
    }
#else
    int RemoveEntry(const char* entry, const struct stat*, int, struct FTW*)
    {
        return std::remove(entry);
    }
#endif
}

TemporaryDirectory::TemporaryDirectory()
{
#if defined(BIBIM_PLATFORM_WINDOWS)
    char base[MAX_PATH];
    REQUIRE(::GetTempPathA(MAX_PATH, base) != 0);
    String prefix = String(base) + "BibimTests";
    prefix.Replace('\\', '/');
    for (int i = 0; path.IsEmpty() && i < 1000; i++)
    {
        const String candidate = String::CFormat("%s%lu-%d", prefix.CStr(), ::GetCurrentProcessId(), i);
        if (::CreateDirectoryA(candidate.CStr(), nullptr))
            path = candidate;
    }
    REQUIRE_FALSE(path.IsEmpty());
#else
    char pattern[] = "/tmp/BibimTestsXXXXXX";
    REQUIRE(mkdtemp(pattern) != nullptr);
    path = pattern;
#endif
}

TemporaryDirectory::~TemporaryDirectory()
{
#if defined(BIBIM_PLATFORM_WINDOWS)
    RemoveRecursively(path);
#else
    nftw(path.CStr(), &RemoveEntry, 16, FTW_DEPTH | FTW_PHYS);
#endif
}

String TemporaryDirectory::Get(const char* name) const
{
    return path + "/" + name;
}
