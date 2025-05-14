#pragma once

#include <aurum/engine/audio_config.hpp>

#include <cstddef>
#include <vector>

namespace aurum {

class AudioBuffer {
public:
    AudioBuffer() = default;

    AudioBuffer(int frames, int channels) { resize(frames, channels); }

    void resize(int frames, int channels) {
        channels_ = channels;
        frames_ = frames;
        data_.assign(buffer_size(frames, channels), 0.0f);
    }

    float* data() { return data_.data(); }
    const float* data() const { return data_.data(); }

    int frames() const { return frames_; }
    int channels() const { return channels_; }
    std::size_t size() const { return data_.size(); }

    float& at(int frame, int channel) {
        return data_[static_cast<std::size_t>(frame) * channels_ + channel];
    }

    const float& at(int frame, int channel) const {
        return data_[static_cast<std::size_t>(frame) * channels_ + channel];
    }

    void clear() { std::fill(data_.begin(), data_.end(), 0.0f); }

private:
    int frames_ = 0;
    int channels_ = 0;
    std::vector<float> data_;
};

}  // namespace aurum
