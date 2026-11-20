#include <Bibim/Config.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/AudioDevice.SDL3.h>

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
