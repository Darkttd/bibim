#include <Bibim/Config.h>
#if (defined(BIBIM_PLATFORM_PC))

#include <Bibim/SoundFX.Silent.h>
#include <Bibim/Math.h>

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
    }

    void SoundFX::Update(float /*dt*/, int /*timestamp*/)
    {
    }

    void SoundFX::Play(const String& /*name*/, int /*group*/)
    {
    }

    void SoundFX::Stop()
    {
    }

    void SoundFX::Stop(int /*group*/)
    {
    }

    void SoundFX::Pause()
    {
    }

    void SoundFX::Pause(int /*group*/)
    {
    }

    void SoundFX::Resume()
    {
    }

    void SoundFX::Resume(int /*group*/)
    {
    }

    bool SoundFX::IsPlaying() const
    {
        return false;
    }

    bool SoundFX::IsPlaying(int /*group*/) const
    {
        return false;
    }

    void SoundFX::SetAudioDevice(AudioDevice* value)
    {
        audioDevice = value;
    }

    void SoundFX::SetVolume(float value)
    {
        volume = Math::Clamp(value, 0.0f, 1.0f);
    }

    void SoundFX::SetMute(bool value)
    {
        mute = value;
    }
}

#endif
