#include <Bibim/Config.h>
#if (defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/BGM.miniaudio.h>
#include <Bibim/AudioDevice.miniaudio.h>
#include <Bibim/Math.h>
#include <miniaudio.h>

namespace Bibim
{
    namespace
    {
        // A piece with a rewind time plays to the end once, then BGM::Update
        // replaces it with a new sound that starts at that time. miniaudio
        // 0.11.26 loop points do not hold for sounds from its resource
        // manager: the decoder behind the sound loops back to the start.
        ma_sound* CreateBGMSound(ma_engine* engine, const String& name, float rewindTime, float startTime)
        {
            ma_sound* sound = new ma_sound;
            if (ma_sound_init_from_file(engine, name.CStr(), MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, nullptr, sound) != MA_SUCCESS)
            {
                delete sound;
                return nullptr;
            }

            ma_sound_set_looping(sound, rewindTime > 0.0f ? MA_FALSE : MA_TRUE);

            ma_uint32 sampleRate = 0;
            ma_uint64 length = 0;
            if (startTime > 0.0f &&
                ma_sound_get_data_format(sound, nullptr, nullptr, &sampleRate, nullptr, 0) == MA_SUCCESS &&
                ma_sound_get_length_in_pcm_frames(sound, &length) == MA_SUCCESS)
            {
                const ma_uint64 frame = static_cast<ma_uint64>(startTime * static_cast<float>(sampleRate));
                if (frame < length)
                    ma_data_source_seek_to_pcm_frame(ma_sound_get_data_source(sound), frame);
            }

            return sound;
        }

        void DestroyBGMSound(ma_sound* sound)
        {
            ma_sound_uninit(sound);
            delete sound;
        }
    }

    BGM::BGM()
        : audioDevice(nullptr),
          aliveBGM(nullptr),
          aliveBGMVolume(0.0f),
          aliveBGMRewindTime(0.0f),
          volume(1.0f),
          crossfadeTime(0.5f),
          mute(false)
    {
    }

    BGM::BGM(AudioDevice* audioDevice)
        : audioDevice(audioDevice),
          aliveBGM(nullptr),
          aliveBGMVolume(0.0f),
          aliveBGMRewindTime(0.0f),
          volume(1.0f),
          crossfadeTime(0.5f),
          mute(false)
    {
    }

    BGM::~BGM()
    {
        DropAliveBGM();
        DropAllDeadBGMs();
    }

    void BGM::Update(float dt, int timestamp)
    {
        if (Seal(timestamp))
            return;

        if (aliveBGM && aliveBGMRewindTime > 0.0f && ma_sound_at_end(aliveBGM))
        {
            if (ma_sound* sound = CreateBGMSound(audioDevice->GetEngine(), aliveBGMName, aliveBGMRewindTime, aliveBGMRewindTime))
            {
                DestroyBGMSound(aliveBGM);
                aliveBGM = sound;
                UpdateVolumes();
                ma_sound_start(aliveBGM);
            }
        }

        if (crossfadeTime > 0.0f)
        {
            const float vd = (1.0f / crossfadeTime) * dt;

            aliveBGMVolume = Math::Min(aliveBGMVolume + vd, 1.0f);

            for (std::vector<DeadBGM>::iterator it = deadBGMs.begin(); it != deadBGMs.end();)
            {
                float& deadVolume = (*it).second;

                deadVolume -= vd;
                if (deadVolume <= 0.0f)
                {
                    DestroyBGMSound((*it).first);
                    it = deadBGMs.erase(it);
                }
                else
                    ++it;
            }

            UpdateVolumes();
        }
    }

    void BGM::Change(const String& name, float rewindTime)
    {
        if (name == aliveBGMName)
            return;

        if (crossfadeTime > 0.0f)
        {
            if (aliveBGM)
            {
                deadBGMs.push_back(DeadBGM(aliveBGM, aliveBGMVolume));
                aliveBGM = nullptr;
            }
        }
        else
            DropAliveBGM();

        aliveBGMName = name;
        aliveBGMRewindTime = rewindTime;

        if (name.IsEmpty())
            return;

        aliveBGMVolume = (crossfadeTime > 0.0f) ? 0.0f : 1.0f;

        ma_engine* engine = audioDevice ? audioDevice->GetEngine() : nullptr;
        if (engine == nullptr)
            return;

        ma_sound* sound = CreateBGMSound(engine, name, rewindTime, 0.0f);
        if (sound == nullptr)
            return;

        aliveBGM = sound;
        UpdateVolumes();
        ma_sound_start(aliveBGM);
    }

    void BGM::SetAudioDevice(AudioDevice* value)
    {
        if (audioDevice != value)
        {
            const String name = aliveBGMName;
            const float rewindTime = aliveBGMRewindTime;

            DropAliveBGM();
            DropAllDeadBGMs();

            audioDevice = value;

            Change(name, rewindTime);
        }
    }

    void BGM::SetVolume(float value)
    {
        value = Math::Clamp(value, 0.0f, 1.0f);

        if (volume != value)
        {
            volume = value;
            UpdateVolumes();
        }
    }

    void BGM::SetMute(bool value)
    {
        if (mute != value)
        {
            mute = value;
            UpdateVolumes();
        }
    }

    void BGM::SetCrossfadeTime(float value)
    {
        value = Math::Clamp(value, 0.0f, 10.0f);

        if (crossfadeTime != value)
        {
            crossfadeTime = value;

            if (crossfadeTime == 0.0f)
            {
                aliveBGMVolume = 1.0f;
                DropAllDeadBGMs();
                UpdateVolumes();
            }
        }
    }

    void BGM::Rewind(float time)
    {
        if (aliveBGM)
            ma_sound_seek_to_second(aliveBGM, time);
    }

    float BGM::GetPlayTime() const
    {
        float cursor = 0.0f;
        if (aliveBGM && ma_sound_get_cursor_in_seconds(aliveBGM, &cursor) == MA_SUCCESS)
            return cursor;
        else
            return 0.0f;
    }

    void BGM::UpdateVolumes()
    {
        const float scale = mute ? 0.0f : volume;

        if (aliveBGM)
            ma_sound_set_volume(aliveBGM, scale * aliveBGMVolume);

        for (std::vector<DeadBGM>::iterator it = deadBGMs.begin(); it != deadBGMs.end(); ++it)
            ma_sound_set_volume((*it).first, scale * (*it).second);
    }

    void BGM::DropAliveBGM()
    {
        if (aliveBGM)
        {
            DestroyBGMSound(aliveBGM);
            aliveBGM = nullptr;
        }

        aliveBGMName = String::Empty;
        aliveBGMRewindTime = 0.0f;
    }

    void BGM::DropAllDeadBGMs()
    {
        for (std::vector<DeadBGM>::iterator it = deadBGMs.begin(); it != deadBGMs.end(); ++it)
            DestroyBGMSound((*it).first);

        deadBGMs.clear();
    }
}

#endif
