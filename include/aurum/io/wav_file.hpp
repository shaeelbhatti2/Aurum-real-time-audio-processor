#pragma once

#include <aurum/engine/audio_buffer.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace aurum::io {

struct WavData {
    int sample_rate = 48000;
    int channels = 2;
    std::vector<float> samples;
};

bool load_wav(const std::string& path, WavData& out, std::string& error);
bool save_wav(const std::string& path, const WavData& data, std::string& error);

}  // namespace aurum::io
