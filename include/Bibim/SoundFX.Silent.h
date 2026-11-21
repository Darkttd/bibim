#pragma once
#ifndef __BIBIM_SOUNDFX_SILENT_H__
#define __BIBIM_SOUNDFX_SILENT_H__

#include <Bibim/FWD.h>
#if (defined(BIBIM_PLATFORM_PC) && !defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/TimelineGameModule.h>
#include <Bibim/String.h>

namespace Bibim
{
    // Keeps the settings but plays nothing until the audio backend lands
    // (phase 5 of #8).
    class SoundFX : public TimelineGameModule
    {
        BBModuleClass(SoundFX, TimelineGameModule, 'S', 'D', 'F', 'X');
        public:
            SoundFX();
            SoundFX(AudioDevice* audioDevice);
            virtual ~SoundFX();

            virtual void Update(float dt, int timestamp);

            void Play(const String& name, int group);
            void Stop();
            void Stop(int group);
            void Pause();
            void Pause(int group);
            void Resume();
            void Resume(int group);

            bool IsPlaying() const;
            bool IsPlaying(int group) const;

            inline AudioDevice* GetAudioDevice() const;
            void SetAudioDevice(AudioDevice* value);

            inline float GetVolume() const;
            void SetVolume(float value);

            inline bool GetMute() const;
            void SetMute(bool value);

        private:
            AudioDevice* audioDevice;
            float volume;
            bool mute;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    AudioDevice* SoundFX::GetAudioDevice() const
    {
        return audioDevice;
    }

    float SoundFX::GetVolume() const
    {
        return volume;
    }

    bool SoundFX::GetMute() const
    {
        return mute;
    }
}

#endif
#endif
