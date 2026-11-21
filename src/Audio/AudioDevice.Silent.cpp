#include <Bibim/Config.h>
#if (defined(BIBIM_PLATFORM_PC) && !defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/AudioDevice.Silent.h>

namespace Bibim
{
    AudioDevice::AudioDevice()
    {
    }

    AudioDevice::~AudioDevice()
    {
    }

    void AudioDevice::AddArchive(MPQ* /*mpq*/)
    {
    }

    float AudioDevice::GetDurationByChars(const char* /*uri*/) const
    {
        return 0.0f;
    }
}

#endif
