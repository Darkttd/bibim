#pragma once
#ifndef __BIBIM_ENVIRONMENT_LINUX_H__
#define __BIBIM_ENVIRONMENT_LINUX_H__

#include <Bibim/FWD.h>
#if (defined(BIBIM_PLATFORM_LINUX))

#include <Bibim/String.h>

namespace Bibim
{
    class Environment
    {
        BBThisIsNoncopyableClass(Environment);
        public:
            static inline const String& GetLocaleName();
            static inline const String& GetWorkingDirectory();
            static String GetAppDataPath(const String& appName, const String& filename);

        private:
            Environment();

        private:
            String localeName;
            String workingDirectory;
            String appDataDirectoryBase;

            static Environment PrivateInstance;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    const String& Environment::GetLocaleName()
    {
        return PrivateInstance.localeName;
    }

    const String& Environment::GetWorkingDirectory()
    {
        return PrivateInstance.workingDirectory;
    }
}

#endif
#endif
