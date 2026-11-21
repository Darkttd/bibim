#include <miniaudio.h>
#include <doctest.h>
#include "Audio/AudioSamples.h"

TEST_CASE("miniaudio decodes Ogg Vorbis with the bundled stb_vorbis")
{
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    ma_decoder decoder;
    REQUIRE(ma_decoder_init_memory(SineOgg, SineOggSize, &config, &decoder) == MA_SUCCESS);
    CHECK(decoder.outputSampleRate == 8000);
    CHECK(decoder.outputChannels == 1);

    float frames[256];
    ma_uint64 total = 0;
    for (;;)
    {
        ma_uint64 read = 0;
        const ma_result result = ma_decoder_read_pcm_frames(&decoder, frames, 256, &read);
        total += read;
        if (result != MA_SUCCESS || read == 0)
            break;
    }
    CHECK(total == 800);

    ma_decoder_uninit(&decoder);
}

TEST_CASE("miniaudio engine starts on the null backend")
{
    ma_backend backends[] = { ma_backend_null };
    ma_context context;
    REQUIRE(ma_context_init(backends, 1, nullptr, &context) == MA_SUCCESS);

    ma_engine_config config = ma_engine_config_init();
    config.pContext = &context;
    ma_engine engine;
    REQUIRE(ma_engine_init(&config, &engine) == MA_SUCCESS);
    CHECK(ma_engine_get_sample_rate(&engine) > 0);

    ma_engine_uninit(&engine);
    ma_context_uninit(&context);
}
