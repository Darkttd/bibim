#pragma once
#ifndef __BIBIM_SOUNDFX_MINIAUDIO_H__
#define __BIBIM_SOUNDFX_MINIAUDIO_H__

#include <Bibim/FWD.h>
#if (defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/TimelineGameModule.h>
#include <Bibim/String.h>
#include <vector>

struct ma_sound;

namespace Bibim
{
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
            struct Sound
            {
                int group;
                ma_sound* sound;
            };
            typedef std::vector<Sound> SoundCollection;

            void SetPaused(bool all, int group, bool paused);
            void UpdateVolumes();
            void DropSounds(bool all, int group);

        private:
            AudioDevice* audioDevice;
            SoundCollection sounds;
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
