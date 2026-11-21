#include <Bibim/Config.h>
#if (defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/SoundFX.miniaudio.h>
#include <Bibim/AudioDevice.miniaudio.h>
#include <Bibim/Math.h>
#include <miniaudio.h>

namespace Bibim
{
    SoundFX::SoundFX()
        : audioDevice(nullptr),
          volume(1.0f),
          mute(false)
    {
    }

    SoundFX::SoundFX(AudioDevice* audioDevice)
        : audioDevice(audioDevice),
          volume(1.0f),
          mute(false)
    {
    }

    SoundFX::~SoundFX()
    {
        DropSounds(true, 0);
    }

    void SoundFX::Update(float /*dt*/, int timestamp)
    {
        if (Seal(timestamp))
            return;

        for (SoundCollection::iterator it = sounds.begin(); it != sounds.end();)
        {
            if (ma_sound_at_end((*it).sound))
            {
                ma_sound_uninit((*it).sound);
                delete (*it).sound;
                it = sounds.erase(it);
            }
            else
                ++it;
        }
    }

    void SoundFX::Play(const String& name, int group)
    {
        ma_sound* decoded = audioDevice ? audioDevice->GetDecodedSound(name) : nullptr;
        if (decoded == nullptr)
            return;

        ma_sound* sound = new ma_sound;
        if (ma_sound_init_copy(audioDevice->GetEngine(), decoded, MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, sound) != MA_SUCCESS)
        {
            delete sound;
            return;
        }

        ma_sound_set_volume(sound, mute ? 0.0f : volume);
        ma_sound_start(sound);

        Sound item;
        item.group = group;
        item.sound = sound;
        sounds.push_back(item);
    }

    void SoundFX::Stop()
    {
        DropSounds(true, 0);
    }

    void SoundFX::Stop(int group)
    {
        DropSounds(false, group);
    }

    void SoundFX::Pause()
    {
        SetPaused(true, 0, true);
    }

    void SoundFX::Pause(int group)
    {
        SetPaused(false, group, true);
    }

    void SoundFX::Resume()
    {
        SetPaused(true, 0, false);
    }

    void SoundFX::Resume(int group)
    {
        SetPaused(false, group, false);
    }

    bool SoundFX::IsPlaying() const
    {
        return sounds.empty() == false;
    }

    bool SoundFX::IsPlaying(int group) const
    {
        for (SoundCollection::const_iterator it = sounds.begin(); it != sounds.end(); ++it)
        {
            if ((*it).group == group)
                return true;
        }

        return false;
    }

    void SoundFX::SetAudioDevice(AudioDevice* value)
    {
        if (audioDevice != value)
        {
            DropSounds(true, 0);
            audioDevice = value;
        }
    }

    void SoundFX::SetVolume(float value)
    {
        value = Math::Clamp(value, 0.0f, 1.0f);

        if (volume != value)
        {
            volume = value;
            UpdateVolumes();
        }
    }

    void SoundFX::SetMute(bool value)
    {
        if (mute != value)
        {
            mute = value;
            UpdateVolumes();
        }
    }

    void SoundFX::SetPaused(bool all, int group, bool paused)
    {
        for (SoundCollection::iterator it = sounds.begin(); it != sounds.end(); ++it)
        {
            if (all || (*it).group == group)
            {
                if (paused)
                    ma_sound_stop((*it).sound);
                else if (ma_sound_at_end((*it).sound) == MA_FALSE)
                    ma_sound_start((*it).sound);
            }
        }
    }

    void SoundFX::UpdateVolumes()
    {
        const float value = mute ? 0.0f : volume;
        for (SoundCollection::iterator it = sounds.begin(); it != sounds.end(); ++it)
            ma_sound_set_volume((*it).sound, value);
    }

    void SoundFX::DropSounds(bool all, int group)
    {
        for (SoundCollection::iterator it = sounds.begin(); it != sounds.end();)
        {
            if (all || (*it).group == group)
            {
                ma_sound_uninit((*it).sound);
                delete (*it).sound;
                it = sounds.erase(it);
            }
            else
                ++it;
        }
    }
}

#endif
