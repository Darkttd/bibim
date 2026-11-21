#pragma once
#include <Bibim/String.h>
#include <cstddef>
#include <vector>

// 0.1 s of a 440 Hz sine, mono, 8000 Hz, Ogg Vorbis (800 frames).
extern const unsigned char SineOgg[];
extern const size_t SineOggSize;

// 16-bit mono 8000 Hz PCM WAV of the given number of silent frames.
std::vector<unsigned char> MakeSilentWav(int frames);

void WriteFile(const Bibim::String& path, const void* data, size_t size);
