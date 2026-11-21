#include <Bibim/Config.h>
#if (defined(BIBIM_USE_MINIAUDIO))

#include <Bibim/AudioDevice.miniaudio.h>
#include <Bibim/FileStream.h>
#include <Bibim/Log.h>
#include <Bibim/MPQ.h>
#include <Bibim/MPQStream.h>
#include <miniaudio.h>
#include <cstdlib>
#include <vector>

namespace Bibim
{
    class AudioDevice::FileSystem
    {
        public:
            FileSystem()
            {
                callbacks.onOpen = &Open;
                callbacks.onOpenW = nullptr;
                callbacks.onClose = &Close;
                callbacks.onRead = &Read;
                callbacks.onWrite = nullptr;
                callbacks.onSeek = &Seek;
                callbacks.onTell = &Tell;
                callbacks.onInfo = &Info;
            }

            ma_vfs* GetVFS()
            {
                return &callbacks;
            }

            void AddArchive(MPQ* mpq)
            {
                archives.push_back(mpq);
            }

        private:
            static FileSystem* From(ma_vfs* vfs)
            {
                return static_cast<FileSystem*>(vfs);
            }

            static Stream* ToStream(ma_vfs_file file)
            {
                return static_cast<StreamPtr*>(file)->GetPointee();
            }

            static ma_result Open(ma_vfs* vfs, const char* path, ma_uint32 openMode, ma_vfs_file* file)
            {
                if (path == nullptr || file == nullptr || (openMode & MA_OPEN_MODE_WRITE) != 0)
                    return MA_INVALID_ARGS;

                *file = nullptr;

                const std::vector<MPQPtr>& archives = From(vfs)->archives;
                for (std::vector<MPQPtr>::const_reverse_iterator it = archives.rbegin(); it != archives.rend(); ++it)
                {
                    if ((*it)->Has(path))
                    {
                        *file = new StreamPtr(new MPQStream(*it, path));
                        return MA_SUCCESS;
                    }
                }

                FileStreamPtr stream = new FileStream(path, FileStream::ReadOnly);
                if (stream->CanRead() == false)
                    return MA_DOES_NOT_EXIST;

                *file = new StreamPtr(stream);
                return MA_SUCCESS;
            }

            static ma_result Close(ma_vfs* /*vfs*/, ma_vfs_file file)
            {
                delete static_cast<StreamPtr*>(file);
                return MA_SUCCESS;
            }

            static ma_result Read(ma_vfs* /*vfs*/, ma_vfs_file file, void* buffer, size_t size, size_t* bytesRead)
            {
                const int requested = size < 0x7FFFFFFF ? static_cast<int>(size) : 0x7FFFFFFF;
                const int read = ToStream(file)->Read(buffer, requested);
                if (bytesRead)
                    *bytesRead = read > 0 ? static_cast<size_t>(read) : 0;

                if (read <= 0 && size > 0)
                    return MA_AT_END;
                else
                    return MA_SUCCESS;
            }

            static ma_result Seek(ma_vfs* /*vfs*/, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin)
            {
                Stream* stream = ToStream(file);
                const int length = stream->GetLength();

                ma_int64 target = offset;
                if (origin == ma_seek_origin_current)
                    target += stream->GetPosition();
                else if (origin == ma_seek_origin_end)
                    target += length;

                if (target < 0 || target > length)
                    return MA_BAD_SEEK;

                if (stream->Seek(static_cast<int>(target), Stream::FromBegin) != target)
                    return MA_BAD_SEEK;
                else
                    return MA_SUCCESS;
            }

            static ma_result Tell(ma_vfs* /*vfs*/, ma_vfs_file file, ma_int64* cursor)
            {
                *cursor = ToStream(file)->GetPosition();
                return MA_SUCCESS;
            }

            static ma_result Info(ma_vfs* /*vfs*/, ma_vfs_file file, ma_file_info* info)
            {
                info->sizeInBytes = static_cast<ma_uint64>(ToStream(file)->GetLength());
                return MA_SUCCESS;
            }

        private:
            ma_vfs_callbacks callbacks;
            std::vector<MPQPtr> archives;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    AudioDevice::AudioDevice()
        : fileSystem(new FileSystem()),
          context(nullptr),
          engine(nullptr)
    {
        // A miniaudio backend name such as "null", like SDL_AUDIO_DRIVER.
        if (const char* driver = std::getenv("BIBIM_AUDIO_DRIVER"))
        {
            for (int i = 0; i < MA_BACKEND_COUNT; i++)
            {
                const ma_backend backend = static_cast<ma_backend>(i);
                if (String(driver).EqualsIgnoreCase(ma_get_backend_name(backend)) == false)
                    continue;

                context = new ma_context;
                if (ma_context_init(&backend, 1, nullptr, context) != MA_SUCCESS)
                {
                    delete context;
                    context = nullptr;
                }
                break;
            }

            if (context == nullptr)
                Log::Warning("AudioDevice", String::CFormat("Unknown or unavailable audio driver '%s'", driver).CStr());
        }

        ma_engine_config config = ma_engine_config_init();
        config.pContext = context;
        config.pResourceManagerVFS = fileSystem->GetVFS();

        engine = new ma_engine;
        if (ma_engine_init(&config, engine) != MA_SUCCESS)
        {
            Log::Warning("AudioDevice", "No audio device could be opened; sounds are not played.");
            delete engine;
            engine = nullptr;
        }
    }

    AudioDevice::~AudioDevice()
    {
        DropDecodedSounds();

        if (engine)
        {
            ma_engine_uninit(engine);
            delete engine;
        }

        if (context)
        {
            ma_context_uninit(context);
            delete context;
        }

        delete fileSystem;
    }

    void AudioDevice::AddArchive(MPQ* mpq)
    {
        if (mpq)
        {
            fileSystem->AddArchive(mpq);
            DropDecodedSounds();
            durations.clear();
        }
    }

    float AudioDevice::GetDurationByChars(const char* uri) const
    {
        if (uri == nullptr || engine == nullptr)
            return 0.0f;

        const String name = uri;
        for (std::vector<std::pair<String, float> >::const_iterator it = durations.begin(); it != durations.end(); ++it)
        {
            if ((*it).first == name)
                return (*it).second;
        }

        ma_sound sound;
        if (ma_sound_init_from_file(engine, uri, MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT, nullptr, nullptr, &sound) != MA_SUCCESS)
            return 0.0f;

        float length = 0.0f;
        if (ma_sound_get_length_in_seconds(&sound, &length) != MA_SUCCESS)
            length = 0.0f;

        ma_sound_uninit(&sound);
        durations.push_back(std::pair<String, float>(name, length));
        return length;
    }

    ma_sound* AudioDevice::GetDecodedSound(const String& name)
    {
        for (std::vector<std::pair<String, ma_sound*> >::const_iterator it = decodedSounds.begin(); it != decodedSounds.end(); ++it)
        {
            if ((*it).first == name)
                return (*it).second;
        }

        if (engine == nullptr)
            return nullptr;

        const ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT | MA_SOUND_FLAG_NO_SPATIALIZATION;
        ma_sound* sound = new ma_sound;
        if (ma_sound_init_from_file(engine, name.CStr(), flags, nullptr, nullptr, sound) != MA_SUCCESS)
        {
            delete sound;
            return nullptr;
        }

        decodedSounds.push_back(std::pair<String, ma_sound*>(name, sound));
        return sound;
    }

    void AudioDevice::DropDecodedSounds()
    {
        for (std::vector<std::pair<String, ma_sound*> >::iterator it = decodedSounds.begin(); it != decodedSounds.end(); ++it)
        {
            ma_sound_uninit((*it).second);
            delete (*it).second;
        }

        decodedSounds.clear();
    }
}

#endif
