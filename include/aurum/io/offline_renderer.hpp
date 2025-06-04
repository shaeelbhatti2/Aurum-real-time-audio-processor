#pragma once

#include <aurum/dsp/effect_chain.hpp>
#include <aurum/io/wav_file.hpp>

#include <functional>
#include <string>

namespace aurum::io {

class OfflineRenderer {
public:
    using ProgressCallback = std::function<void(float)>;

    bool render(const io::WavData& input, dsp::EffectChain& chain, io::WavData& output,
                ProgressCallback progress, std::string& error);

private:
    int block_size_ = 512;
};

}  // namespace aurum::io
