#pragma once

#include <cstddef>

namespace aurum {

struct AudioConfig {
    int sample_rate = 48000;
    int block_size = 512;
    int input_channels = 0;
    int output_channels = 2;
};

inline std::size_t frame_stride(int channels) {
    return static_cast<std::size_t>(channels);
}

inline std::size_t buffer_size(int frames, int channels) {
    return static_cast<std::size_t>(frames) * frame_stride(channels);
}

}  // namespace aurum
