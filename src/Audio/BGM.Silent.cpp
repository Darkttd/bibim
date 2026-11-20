#include <Bibim/Config.h>
#if (defined(BIBIM_PLATFORM_PC))

#include <Bibim/BGM.Silent.h>
#include <Bibim/Math.h>

namespace Bibim
{
    BGM::BGM()
        : audioDevice(nullptr),
          volume(1.0f),
          crossfadeTime(0.5f),
          mute(false)
    {
    }

    BGM::BGM(AudioDevice* audioDevice)
        : audioDevice(audioDevice),
          volume(1.0f),
          crossfadeTime(0.5f),
          mute(false)
    {
    }

    BGM::~BGM()
    {
    }

    void BGM::Update(float /*dt*/, int /*timestamp*/)
    {
    }

    void BGM::Change(const String& /*name*/, float /*rewindTime*/)
    {
    }

    void BGM::SetAudioDevice(AudioDevice* value)
    {
        audioDevice = value;
    }

    void BGM::SetVolume(float value)
    {
        volume = Math::Clamp(value, 0.0f, 1.0f);
    }

    void BGM::SetMute(bool value)
    {
        mute = value;
    }

    void BGM::SetCrossfadeTime(float value)
    {
        crossfadeTime = Math::Clamp(value, 0.0f, 10.0f);
    }

    void BGM::Rewind(float /*time*/)
    {
    }

    float BGM::GetPlayTime() const
    {
        return 0.0f;
    }
}

#endif
