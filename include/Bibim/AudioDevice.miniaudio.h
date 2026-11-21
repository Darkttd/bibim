#pragma once
#ifndef __BIBIM_AUDIODEVICE_MINIAUDIO_H__
#define __BIBIM_AUDIODEVICE_MINIAUDIO_H__

#include <Bibim/FWD.h>
#if (defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/GameModule.h>
#include <Bibim/String.h>
#include <utility>
#include <vector>

struct ma_context;
struct ma_engine;
struct ma_sound;

namespace Bibim
{
    // Must outlive the BGM and SoundFX modules that use it.
    class AudioDevice : public GameModule
    {
        BBModuleClass(AudioDevice, GameModule, 'A', 'U', 'D', 'D');
        public:
            AudioDevice();
            virtual ~AudioDevice();

            void AddArchive(MPQ* mpq);

            inline float GetDuration(const String& uri) const;
            float GetDurationByChars(const char* uri) const;

            inline ma_engine* GetEngine() const;

            // Kept until an archive is added or the device is destroyed; sounds
            // copied from it share its decoded data.
            ma_sound* GetDecodedSound(const String& name);

        private:
            void DropDecodedSounds();

        private:
            class FileSystem;

        private:
            FileSystem* fileSystem;
            ma_context* context;
            ma_engine* engine;
            std::vector<std::pair<String, ma_sound*> > decodedSounds;
            mutable std::vector<std::pair<String, float> > durations;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    float AudioDevice::GetDuration(const String& uri) const
    {
        return GetDurationByChars(uri.CStr());
    }

    ma_engine* AudioDevice::GetEngine() const
    {
        return engine;
    }
}

#endif
#endif
