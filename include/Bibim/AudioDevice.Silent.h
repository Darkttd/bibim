#pragma once
#ifndef __BIBIM_AUDIODEVICE_SILENT_H__
#define __BIBIM_AUDIODEVICE_SILENT_H__

#include <Bibim/FWD.h>
#if (defined(BIBIM_PLATFORM_PC) && !defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/GameModule.h>
#include <Bibim/String.h>

namespace Bibim
{
    // Plays nothing until the audio backend lands (phase 5 of #8).
    class AudioDevice : public GameModule
    {
        BBModuleClass(AudioDevice, GameModule, 'A', 'U', 'D', 'D');
        public:
            AudioDevice();
            virtual ~AudioDevice();

            void AddArchive(MPQ* mpq);

            inline float GetDuration(const String& uri) const;
            float GetDurationByChars(const char* uri) const;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    float AudioDevice::GetDuration(const String& uri) const
    {
        return GetDurationByChars(uri.CStr());
    }
}

#endif
#endif
