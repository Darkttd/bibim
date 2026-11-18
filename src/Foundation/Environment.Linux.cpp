#include <Bibim/Config.h>
#if (defined(BIBIM_PLATFORM_LINUX))

#include <Bibim/Environment.Linux.h>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace Bibim
{
    Environment Environment::PrivateInstance;

    static const char* GetNonEmptyVariable(const char* name)
    {
        const char* value = std::getenv(name);
        return (value && value[0] != '\0') ? value : nullptr;
    }

    static bool IsDirectory(const String& path)
    {
        struct stat status;
        return ::stat(path.CStr(), &status) == 0 && S_ISDIR(status.st_mode);
    }

    static bool CreateDirectories(const String& path)
    {
        const char* s = path.CStr();
        for (int i = 1; i < path.GetLength(); i++)
        {
            if (s[i] == '/')
                ::mkdir(path.Substring(0, i).CStr(), 0755);
        }

        ::mkdir(s, 0755);
        return IsDirectory(path);
    }

    Environment::Environment()
    {
        {
            // Same precedence as the LC_MESSAGES category.
            const char* locale = GetNonEmptyVariable("LC_ALL");
            if (locale == nullptr)
                locale = GetNonEmptyVariable("LC_MESSAGES");
            if (locale == nullptr)
                locale = GetNonEmptyVariable("LANG");

            if (locale && std::strncmp(locale, "ko", 2) == 0)
                localeName = "ko-KR";
            else if (locale && std::strncmp(locale, "ja", 2) == 0)
                localeName = "ja-JP";
            else
                localeName = "en-US";
        }

        {
            char* currentDirectory = ::getcwd(nullptr, 0);
            if (currentDirectory)
            {
                workingDirectory = currentDirectory;
                std::free(currentDirectory);
            }
        }

        {
            // The XDG Base Directory spec ignores relative paths.
            const char* dataHome = GetNonEmptyVariable("XDG_DATA_HOME");
            const char* home = GetNonEmptyVariable("HOME");
            if (dataHome && dataHome[0] == '/')
                appDataDirectoryBase = dataHome;
            else if (home)
                appDataDirectoryBase = String(home) + "/.local/share";
        }
    }

    String Environment::GetAppDataPath(const String& appName, const String& filename)
    {
        if (PrivateInstance.appDataDirectoryBase.IsEmpty() == false)
        {
            const String directory = PrivateInstance.appDataDirectoryBase + "/" + appName;
            if (IsDirectory(directory) || CreateDirectories(directory))
                return directory + "/" + filename;
        }

        return PrivateInstance.workingDirectory + "/" + filename;
    }
}

#endif
