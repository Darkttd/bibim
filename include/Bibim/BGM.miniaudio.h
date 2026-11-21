#pragma once
#ifndef __BIBIM_BGM_MINIAUDIO_H__
#define __BIBIM_BGM_MINIAUDIO_H__

#include <Bibim/FWD.h>
#if (defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/TimelineGameModule.h>
#include <Bibim/String.h>
#include <utility>
#include <vector>

struct ma_sound;

namespace Bibim
{
    class BGM : public TimelineGameModule
    {
        BBModuleClass(BGM, TimelineGameModule, 'B', 'G', 'M', 'M');
        public:
            BGM();
            BGM(AudioDevice* audioDevice);
            virtual ~BGM();

            virtual void Update(float dt, int timestamp);

            inline void Change(const String& name);
            void Change(const String& name, float rewindTime);

            inline AudioDevice* GetAudioDevice() const;
            void SetAudioDevice(AudioDevice* value);

            inline float GetVolume() const;
            void SetVolume(float value);

            inline bool GetMute() const;
            void SetMute(bool value);

            inline float GetCrossfadeTime() const;
            void SetCrossfadeTime(float value);

        protected:
            inline void Rewind();
            void Rewind(float time);
            float GetPlayTime() const;

        private:
            typedef std::pair<ma_sound*, float> DeadBGM;

            void UpdateVolumes();
            void DropAliveBGM();
            void DropAllDeadBGMs();

        private:
            AudioDevice* audioDevice;
            ma_sound* aliveBGM;
            float aliveBGMVolume;
            String aliveBGMName;
            float aliveBGMRewindTime;
            std::vector<DeadBGM> deadBGMs;
            float volume;
            float crossfadeTime;
            bool mute;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void BGM::Change(const String& name)
    {
        Change(name, 0.0f);
    }

    AudioDevice* BGM::GetAudioDevice() const
    {
        return audioDevice;
    }

    float BGM::GetVolume() const
    {
        return volume;
    }

    bool BGM::GetMute() const
    {
        return mute;
    }

    float BGM::GetCrossfadeTime() const
    {
        return crossfadeTime;
    }

    void BGM::Rewind()
    {
        Rewind(0.0f);
    }
}

#endif
#endif
