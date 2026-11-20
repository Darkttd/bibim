// The PC audio classes play nothing yet but keep their settings, so games
// that set volumes and mute flags behave the same until real audio lands.
#include <Bibim/Config.h>
#include <Bibim/AudioDevice.h>
#include <Bibim/BGM.h>
#include <Bibim/GameModuleNode.h>
#include <Bibim/GameModuleTree.h>
#include <Bibim/SoundFX.h>
#include <doctest.h>

using namespace Bibim;

TEST_CASE("Silent AudioDevice reports no duration and ignores archives")
{
    GameModuleTree modules;
    AudioDevice* device = new AudioDevice();
    modules.GetRoot()->AttachChild(device);

    device->AddArchive(nullptr);
    CHECK(device->GetDuration("Asset/BGM/Title.ogg") == 0.0f);
    CHECK(device->GetDurationByChars(nullptr) == 0.0f);
}

TEST_CASE("Silent BGM keeps its settings")
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

    bgm->Change("Asset/BGM/Title.ogg");
    bgm->Change("Asset/BGM/Stage.ogg", 1.0f);
    bgm->Update(1.0f / 60.0f, 1);
}

TEST_CASE("Silent SoundFX keeps its settings and is never playing")
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

    sfx->Play("Asset/SFX/Click.wav", 1);
    CHECK_FALSE(sfx->IsPlaying());
    CHECK_FALSE(sfx->IsPlaying(1));
    sfx->Pause(1);
    sfx->Resume();
    sfx->Stop(1);
    sfx->Stop();
    sfx->Update(1.0f / 60.0f, 1);
}
