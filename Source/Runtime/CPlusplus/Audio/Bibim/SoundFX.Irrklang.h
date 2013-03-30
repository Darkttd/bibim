#pragma once
#ifndef __BIBIM_SOUNDFX_IRRKLANG_H__
#define __BIBIM_SOUNDFX_IRRKLANG_H__

#   include <Bibim/FWD.h>
#   include <Bibim/TimelineGameModule.h>
#   include <Bibim/Lock.h>
#   include <Bibim/String.h>
#   include <vector>

    namespace irrklang { class ISound; };

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
                inline void Pause();
                inline void Pause(int group);
                inline void Resume();
                inline void Resume(int group);

                inline AudioDevice* GetAudioDevice() const;
                void SetAudioDevice(AudioDevice* value);

                inline float GetVolume() const;
                void SetVolume(float value);

                inline bool GetMute() const;
                void SetMute(bool value);

            private:
                class EventListener;

            private:
                typedef std::vector<irrklang::ISound*> SoundCollection;
                typedef std::vector<int>             SoundDictionaryKeys;
                typedef std::vector<SoundCollection> SoundDictionaryValues;
                typedef std::vector<std::pair<int, irrklang::ISound*> > StoppedSoundCollection;

                void UpdateVolumes();
                SoundCollection* FindSounds(int group);
                const SoundCollection* FindSounds(int group) const;
                void SetInPauseAllSounds(bool value);
                void SetInPauseAllSounds(int group, bool value);
                void DropSound(int group, irrklang::ISound* sound);
                void DropAllSounds();

            private:
                AudioDevice* audioDevice;
                SoundDictionaryKeys   soundGroups;
                SoundDictionaryValues sounds;
                float volume;
                bool mute;
                Lock stoppedSoundsLock;
                StoppedSoundCollection stoppedSounds;
                EventListener* eventListener;
        };
    }

#   include <Bibim/SoundFX.Irrklang.inl>

#endif