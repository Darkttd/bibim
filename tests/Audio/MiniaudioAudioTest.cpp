#include <Bibim/Config.h>
#if (defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/AudioDevice.h>
#include <Bibim/BGM.h>
#include <Bibim/GameModuleNode.h>
#include <Bibim/GameModuleTree.h>
#include <Bibim/MPQ.h>
#include <Bibim/SoundFX.h>
#include <StormLib.h>
#include <miniaudio.h>
#include <doctest.h>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>
#include "Audio/AudioSamples.h"
#include "Common/TemporaryDirectory.h"

using namespace Bibim;

namespace
{
    const float SampleLength = 0.1f;

    // Keeps the tests off the real sound device when BibimTests runs outside CTest.
    const bool UsesNullDriver = []()
    {
#if (defined(BIBIM_PLATFORM_WINDOWS))
        if (std::getenv("BIBIM_AUDIO_DRIVER") == nullptr)
            _putenv_s("BIBIM_AUDIO_DRIVER", "null");
#else
        setenv("BIBIM_AUDIO_DRIVER", "null", 0);
#endif
        return true;
    }();

    class AudioFiles
    {
        public:
            AudioFiles()
                : ogg(directory.Get("Sine.ogg")),
                  wav(directory.Get("Silence.wav")),
                  longWav(directory.Get("LongSilence.wav"))
            {
                WriteFile(ogg, SineOgg, SineOggSize);
                const std::vector<unsigned char> silence = MakeSilentWav(800);
                WriteFile(wav, silence.data(), silence.size());
                const std::vector<unsigned char> longSilence = MakeSilentWav(8000);
                WriteFile(longWav, longSilence.data(), longSilence.size());
            }

            TemporaryDirectory directory;
            String ogg;
            String wav;
            String longWav;
    };

    class TestBGM : public BGM
    {
        public:
            TestBGM(AudioDevice* audioDevice)
                : BGM(audioDevice)
            {
            }

            using BGM::GetPlayTime;
            using BGM::Rewind;
    };

    // The null driver plays in real time on its own thread, so tests poll
    // instead of waiting a fixed time.
    template <typename Condition> bool WaitUntil(Condition condition)
    {
        for (int i = 0; i < 200; i++)
        {
            if (condition())
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return false;
    }

    float LowestPlayTimeAfterLoop(TestBGM* bgm)
    {
        float previous = bgm->GetPlayTime();
        int wraps = 0;
        int timestamp = 0;
        float lowest = 1000.0f;
        WaitUntil([&]()
        {
            bgm->Update(0.0f, ++timestamp);
            const float current = bgm->GetPlayTime();
            if (current < previous)
                wraps++;
            if (wraps >= 1 && current < lowest)
                lowest = current;
            previous = current;
            return wraps >= 2;
        });
        return lowest;
    }
}

TEST_CASE("miniaudio AudioDevice plays on the null driver in the tests")
{
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    modules.GetRoot()->AttachChild(device);

    REQUIRE(device->GetEngine() != nullptr);
    CHECK(ma_engine_get_device(device->GetEngine())->pContext->backend == ma_backend_null);
}

TEST_CASE("miniaudio AudioDevice reads the duration of files")
{
    AudioFiles files;
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    modules.GetRoot()->AttachChild(device);

    CHECK(device->GetDuration(files.ogg) == doctest::Approx(SampleLength));
    CHECK(device->GetDuration(files.wav) == doctest::Approx(SampleLength));
    CHECK(device->GetDuration(files.directory.Get("Missing.ogg")) == 0.0f);
    CHECK(device->GetDurationByChars(nullptr) == 0.0f);
}

TEST_CASE("miniaudio AudioDevice reads files from added archives first")
{
    TemporaryDirectory directory;
    const String archivePath = directory.Get("Sound.mpq");
    {
        HANDLE archive = nullptr;
        REQUIRE(SFileCreateArchive(archivePath.CStr(), MPQ_CREATE_ARCHIVE_V1, 16, &archive));
        HANDLE file = nullptr;
        REQUIRE(SFileCreateFile(archive, "BGM\\Sine.ogg", 0, static_cast<DWORD>(SineOggSize), 0, MPQ_FILE_COMPRESS, &file));
        REQUIRE(SFileWriteFile(file, SineOgg, static_cast<DWORD>(SineOggSize), MPQ_COMPRESSION_ZLIB));
        REQUIRE(SFileFinishFile(file));
        REQUIRE(SFileCloseArchive(archive));
    }

    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    modules.GetRoot()->AttachChild(device);

    CHECK(device->GetDuration("BGM/Sine.ogg") == 0.0f);
    device->AddArchive(new MPQ(archivePath));
    CHECK(device->GetDuration("BGM/Sine.ogg") == doctest::Approx(SampleLength));
}

TEST_CASE("miniaudio BGM keeps its settings")
{
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    BGM* bgm = new BGM(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(bgm);

    CHECK(bgm->GetAudioDevice() == device);
    CHECK(bgm->GetVolume() == 1.0f);
    CHECK(bgm->GetCrossfadeTime() == 0.5f);
    CHECK_FALSE(bgm->GetMute());

    bgm->SetVolume(0.25f);
    CHECK(bgm->GetVolume() == 0.25f);
    bgm->SetVolume(2.0f);
    CHECK(bgm->GetVolume() == 1.0f);
    bgm->SetVolume(-1.0f);
    CHECK(bgm->GetVolume() == 0.0f);

    bgm->SetCrossfadeTime(20.0f);
    CHECK(bgm->GetCrossfadeTime() == 10.0f);
    bgm->SetMute(true);
    CHECK(bgm->GetMute());
}

TEST_CASE("miniaudio BGM loops the piece")
{
    AudioFiles files;
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    TestBGM* bgm = new TestBGM(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(bgm);

    bgm->Change(files.ogg);
    CHECK(LowestPlayTimeAfterLoop(bgm) < SampleLength * 0.5f);
    CHECK(bgm->GetPlayTime() < SampleLength);
}

TEST_CASE("miniaudio BGM plays the part before the rewind time only once")
{
    AudioFiles files;
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    TestBGM* bgm = new TestBGM(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(bgm);

    const float rewindTime = SampleLength * 0.5f;
    bgm->Change(files.ogg, rewindTime);
    const float lowest = LowestPlayTimeAfterLoop(bgm);
    CHECK(lowest >= rewindTime - 0.001f);
    CHECK(lowest < SampleLength);
}

TEST_CASE("miniaudio BGM rewinds the current piece")
{
    AudioFiles files;
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    TestBGM* bgm = new TestBGM(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(bgm);

    CHECK(bgm->GetPlayTime() == 0.0f);
    bgm->Change(files.longWav);
    bgm->Rewind(0.5f);

    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    REQUIRE(WaitUntil([bgm]() { return bgm->GetPlayTime() >= 0.55f; }));
    CHECK(std::chrono::steady_clock::now() - start < std::chrono::milliseconds(400));
}

TEST_CASE("miniaudio BGM crossfades and ignores files it cannot open")
{
    AudioFiles files;
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    TestBGM* bgm = new TestBGM(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(bgm);

    bgm->SetCrossfadeTime(0.2f);
    bgm->Change(files.ogg);
    bgm->Update(0.1f, 1);
    bgm->Change(files.wav);
    bgm->Update(0.1f, 2);
    bgm->Update(0.1f, 3);
    bgm->Update(0.1f, 4);

    bgm->Change(files.directory.Get("Missing.ogg"));
    CHECK(bgm->GetPlayTime() == 0.0f);
    bgm->Update(0.1f, 5);
    bgm->Change(String::Empty);
    bgm->Update(1.0f, 6);
}

TEST_CASE("miniaudio SoundFX keeps its settings")
{
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    SoundFX* sfx = new SoundFX(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(sfx);

    CHECK(sfx->GetAudioDevice() == device);
    CHECK(sfx->GetVolume() == 1.0f);
    CHECK_FALSE(sfx->GetMute());

    sfx->SetVolume(0.5f);
    CHECK(sfx->GetVolume() == 0.5f);
    sfx->SetVolume(3.0f);
    CHECK(sfx->GetVolume() == 1.0f);
    sfx->SetMute(true);
    CHECK(sfx->GetMute());
}

TEST_CASE("miniaudio SoundFX plays, stops and finishes sounds by group")
{
    AudioFiles files;
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    SoundFX* sfx = new SoundFX(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(sfx);

    sfx->Play(files.directory.Get("Missing.wav"), 1);
    CHECK_FALSE(sfx->IsPlaying());

    sfx->Play(files.wav, 1);
    sfx->Play(files.ogg, 2);
    sfx->Play(files.ogg, 2);
    CHECK(sfx->IsPlaying());
    CHECK(sfx->IsPlaying(1));
    CHECK(sfx->IsPlaying(2));
    CHECK_FALSE(sfx->IsPlaying(3));

    sfx->Stop(1);
    CHECK_FALSE(sfx->IsPlaying(1));
    CHECK(sfx->IsPlaying(2));

    int timestamp = 0;
    CHECK(WaitUntil([&]() { sfx->Update(0.0f, ++timestamp); return sfx->IsPlaying() == false; }));

    sfx->Play(files.wav, 1);
    sfx->Stop();
    CHECK_FALSE(sfx->IsPlaying());
}

TEST_CASE("miniaudio SoundFX keeps paused sounds until they are resumed and finish")
{
    AudioFiles files;
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    SoundFX* sfx = new SoundFX(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(sfx);

    sfx->Play(files.wav, 1);
    sfx->Play(files.wav, 2);
    sfx->Pause(2);
    int timestamp = 0;
    CHECK(WaitUntil([&]() { sfx->Update(0.0f, ++timestamp); return sfx->IsPlaying(1) == false; }));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sfx->Update(0.0f, ++timestamp);
    CHECK(sfx->IsPlaying(2));

    sfx->Pause();
    sfx->Resume(2);
    CHECK(WaitUntil([&]() { sfx->Update(0.0f, ++timestamp); return sfx->IsPlaying() == false; }));
}

TEST_CASE("miniaudio SoundFX does not replay a finished sound on Resume")
{
    AudioFiles files;
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    SoundFX* sfx = new SoundFX(device);
    modules.GetRoot()->AttachChild(device);
    modules.GetRoot()->AttachChild(sfx);

    sfx->Play(files.wav, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    sfx->Resume();
    sfx->Update(0.0f, 1);
    CHECK_FALSE(sfx->IsPlaying());
}

#endif
